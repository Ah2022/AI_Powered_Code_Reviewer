#!/usr/bin/env python3
"""
AI-Powered C++ Code Reviewer

This script analyzes C++ code files and provides intelligent feedback
using OpenAI's language models to detect issues and suggest improvements.
"""

import os
import sys
import json
import argparse
import re
from typing import List, Dict, Any, Optional, Tuple

# Import OpenAI client
from openai import OpenAI

# Initialize OpenAI client with API key
OPENAI_API_KEY = os.environ.get("OPENAI_API_KEY")
if not OPENAI_API_KEY:
    print("Error: OPENAI_API_KEY environment variable is not set.")
    print("Please set it to your OpenAI API key to continue.")
    sys.exit(1)

openai = OpenAI(api_key=OPENAI_API_KEY)

# Issue severity levels
class Severity:
    ERROR = "ERROR"
    WARNING = "WARNING"
    INFO = "INFO"
    OPTIMIZATION = "OPTIMIZATION"

# Issue types for C++ code
class IssueType:
    MEMORY_LEAK = "Memory Leak"
    NULL_POINTER = "Null Pointer Dereference"
    UNINITIALIZED_VAR = "Uninitialized Variable"
    RESOURCE_LEAK = "Resource Leak"
    BUFFER_OVERFLOW = "Buffer Overflow"
    DEAD_CODE = "Dead Code"
    STYLE_VIOLATION = "Style Violation"
    PERFORMANCE_ISSUE = "Performance Issue"
    CONCURRENCY_ISSUE = "Concurrency Issue"
    OTHER = "Other Issue"

# ANSI color codes for terminal output
class Colors:
    RED = "\033[1;31m"     # Bold Red (Error)
    YELLOW = "\033[1;33m"  # Bold Yellow (Warning)
    BLUE = "\033[1;34m"    # Bold Blue (Info)
    GREEN = "\033[1;32m"   # Bold Green (Optimization)
    RESET = "\033[0m"      # Reset color

def get_color_for_severity(severity: str) -> str:
    """Return ANSI color code for a severity level."""
    if severity == Severity.ERROR:
        return Colors.RED
    elif severity == Severity.WARNING:
        return Colors.YELLOW
    elif severity == Severity.INFO:
        return Colors.BLUE
    elif severity == Severity.OPTIMIZATION:
        return Colors.GREEN
    return Colors.RESET

def read_file(file_path: str) -> str:
    """Read file contents."""
    try:
        with open(file_path, 'r') as file:
            return file.read()
    except Exception as e:
        print(f"Error reading file '{file_path}': {e}")
        sys.exit(1)

def extract_code_snippet(source_code: str, line_num: int, context_lines: int = 2) -> str:
    """Extract a snippet of code around the specified line number."""
    lines = source_code.split('\n')
    start_line = max(1, line_num - context_lines)
    end_line = min(len(lines), line_num + context_lines)
    
    snippet = []
    for i in range(start_line, end_line + 1):
        line_prefix = f"{i}: " + ("→ " if i == line_num else "  ")
        if i-1 < len(lines):
            snippet.append(f"{line_prefix}{lines[i-1]}")
    
    return '\n'.join(snippet)

def get_built_in_explanation(issue_type: str, message: str) -> Tuple[str, str]:
    """
    Provide built-in explanations and recommendations for common issues
    when OpenAI API integration is not available.
    
    Returns a tuple of (explanation, recommendation)
    """
    # Default fallbacks
    default_explanation = "This is a common issue in C++ programming that could cause problems."
    default_recommendation = "Consider reviewing the code and addressing this issue according to C++ best practices."
    
    # Common issue explanations and recommendations
    explanations = {
        IssueType.MEMORY_LEAK: {
            "explanation": "Memory leaks occur when dynamically allocated memory is not properly deallocated. "
                          "This can lead to increased memory usage over time and eventually cause your program "
                          "to run out of memory if the leak happens in frequently called code.",
            "recommendation": "Ensure every call to 'new' is matched with a corresponding 'delete', and 'new[]' "
                            "with 'delete[]'. Better yet, use smart pointers like std::unique_ptr or std::shared_ptr "
                            "which automatically manage memory for you:\n\n"
                            "```cpp\n"
                            "// Instead of:\n"
                            "int* array = new int[100];\n"
                            "// Use:\n"
                            "std::unique_ptr<int[]> array = std::make_unique<int[]>(100);\n"
                            "```"
        },
        
        IssueType.UNINITIALIZED_VAR: {
            "explanation": "Using uninitialized variables leads to undefined behavior because their values "
                          "are unpredictable. This can cause crashes, security vulnerabilities, or subtle bugs "
                          "that are difficult to track down.",
            "recommendation": "Always initialize variables when you declare them:\n\n"
                            "```cpp\n"
                            "// Instead of:\n"
                            "int x;\n"
                            "// Do this:\n"
                            "int x = 0; // Or another appropriate initial value\n"
                            "```"
        },
        
        IssueType.BUFFER_OVERFLOW: {
            "explanation": "Buffer overflows occur when a program writes data beyond the allocated memory buffer. "
                          "This is a serious security vulnerability that can lead to crashes, data corruption, "
                          "or even allow attackers to execute arbitrary code.",
            "recommendation": "Use safer alternatives to C-style string functions:\n\n"
                            "```cpp\n"
                            "// Instead of:\n"
                            "char buffer[10];\n"
                            "strcpy(buffer, input); // Unsafe\n"
                            "// Use:\n"
                            "std::string buffer = input; // Safe, handles any length\n"
                            "// Or if you must use C-style strings:\n"
                            "char buffer[10];\n"
                            "strncpy(buffer, input, sizeof(buffer) - 1);\n"
                            "buffer[sizeof(buffer) - 1] = '\\0'; // Ensure null termination\n"
                            "```"
        },
        
        IssueType.RESOURCE_LEAK: {
            "explanation": "Resource leaks happen when resources like file handles, network connections, or "
                          "database connections are not properly closed or released. This can exhaust system "
                          "resources and cause your program or even the entire system to malfunction.",
            "recommendation": "Use RAII (Resource Acquisition Is Initialization) principle by wrapping resources "
                            "in classes that manage their lifecycle:\n\n"
                            "```cpp\n"
                            "// Instead of:\n"
                            "FILE* file = fopen(filename, \"r\");\n"
                            "// Use:\n"
                            "std::ifstream file(filename);\n"
                            "// Or for C-style resources:\n"
                            "std::unique_ptr<FILE, decltype(&fclose)> file(fopen(filename, \"r\"), &fclose);\n"
                            "```"
        },
        
        IssueType.NULL_POINTER: {
            "explanation": "Dereferencing a null pointer causes undefined behavior and typically results in a "
                          "program crash. This happens when a pointer that hasn't been initialized or has been "
                          "set to nullptr is accessed as if it points to a valid object.",
            "recommendation": "Always check if a pointer is null before dereferencing it:\n\n"
                            "```cpp\n"
                            "// Instead of:\n"
                            "return *resource; // May crash if resource is nullptr\n"
                            "// Do this:\n"
                            "if (resource != nullptr) {\n"
                            "    return *resource;\n"
                            "} else {\n"
                            "    // Handle null case appropriately\n"
                            "    return defaultValue; // Or throw an exception\n"
                            "}\n"
                            "```"
        },
        
        IssueType.STYLE_VIOLATION: {
            "explanation": "Style violations may not cause functional issues, but they can make code harder to "
                          "read, maintain, and debug. Consistent style improves collaboration and can prevent bugs.",
            "recommendation": "Follow modern C++ coding guidelines and be consistent with your project's style guide. "
                            "For the specific issue, consider the suggested alternative approach."
        },
        
        IssueType.PERFORMANCE_ISSUE: {
            "explanation": "Performance issues can make your code unnecessarily slow or resource-intensive. "
                          "While premature optimization should be avoided, obvious inefficiencies should be fixed.",
            "recommendation": "Look for more efficient alternatives to common operations, especially in loops "
                            "or frequently called functions. Cache values instead of recomputing them repeatedly."
        },
        
        IssueType.DEAD_CODE: {
            "explanation": "Dead code is code that can never be executed because it's unreachable. This clutters "
                          "your codebase, confuses readers, and might indicate logical errors in your program flow.",
            "recommendation": "Remove or fix dead code to maintain a clean codebase and prevent confusion. "
                            "If code seems unreachable but should be, fix the logical flow issue that prevents it "
                            "from being executed."
        }
    }
    
    # Pattern-specific explanations that override the general ones
    pattern_explanations = {
        "Using directive brings all names from namespace 'std' into global namespace": {
            "explanation": "Using 'using namespace std;' imports all names from the standard library into the "
                          "global namespace. This can cause naming conflicts, make it unclear where functions "
                          "and types come from, and lead to subtle bugs when names collide.",
            "recommendation": "Instead of importing the entire namespace, either use qualified names or import "
                            "only the specific names you need:\n\n"
                            "```cpp\n"
                            "// Instead of:\n"
                            "using namespace std;\n"
                            "// Do this:\n"
                            "std::cout << \"Hello\" << std::endl;\n"
                            "// Or if you use a name frequently:\n"
                            "using std::cout;\n"
                            "using std::endl;\n"
                            "cout << \"Hello\" << endl;\n"
                            "```"
        },
        
        "Consider using std::string instead of C-style character arrays": {
            "explanation": "C-style character arrays are prone to buffer overflows and don't provide the safety "
                          "and convenience features of std::string. They require manual memory management and "
                          "don't automatically handle resizing or bounds checking.",
            "recommendation": "Use std::string instead of C-style character arrays for better safety and convenience:\n\n"
                            "```cpp\n"
                            "// Instead of:\n"
                            "char buffer[10];\n"
                            "// Use:\n"
                            "std::string buffer;\n"
                            "```"
        },
        
        "Consider using constexpr or const instead of #define for constants": {
            "explanation": "#define macros are simple text substitutions that happen during preprocessing, "
                          "before the compiler sees the code. They lack type safety, don't respect scoping rules, "
                          "and can cause unexpected behavior because they don't behave like normal C++ constructs.",
            "recommendation": "Use constexpr or const instead of #define for constants:\n\n"
                            "```cpp\n"
                            "// Instead of:\n"
                            "#define MAX_SIZE 100\n"
                            "// Use:\n"
                            "constexpr int MAX_SIZE = 100; // For compile-time constants\n"
                            "// Or:\n"
                            "const int MAX_SIZE = 100; // If constexpr isn't needed\n"
                            "```"
        }
    }
    
    # First check if we have a specific explanation for this exact message
    if message in pattern_explanations:
        return (pattern_explanations[message]["explanation"], 
                pattern_explanations[message]["recommendation"])
    
    # Otherwise use the general explanation for the issue type
    if issue_type in explanations:
        return (explanations[issue_type]["explanation"], 
                explanations[issue_type]["recommendation"])
    
    # If nothing matches, return defaults
    return (default_explanation, default_recommendation)

def detect_common_patterns(source_code: str) -> List[Dict[str, Any]]:
    """
    Simple pattern-based detection of common issues in C++ code.
    This is a basic implementation that will be enhanced by LLM analysis.
    """
    issues = []
    lines = source_code.split('\n')
    
    # Regular expression patterns for common issues
    patterns = [
        (r'\bnew\b(?!.*\bdelete\b)', IssueType.MEMORY_LEAK, Severity.WARNING, 
         "Potential memory leak: 'new' used without matching 'delete'"),
        
        (r'\bmalloc\b(?!.*\bfree\b)', IssueType.MEMORY_LEAK, Severity.WARNING,
         "Potential memory leak: 'malloc' used without matching 'free'"),
        
        (r'using\s+namespace\s+std\s*;', IssueType.STYLE_VIOLATION, Severity.INFO,
         "Using directive brings all names from namespace 'std' into global namespace"),
        
        (r'#define\s+[A-Z_]+\s+\d+', IssueType.STYLE_VIOLATION, Severity.INFO,
         "Consider using constexpr or const instead of #define for constants"),
        
        (r'char\s+\w+\s*\[\s*\d+\s*\]', IssueType.STYLE_VIOLATION, Severity.INFO,
         "Consider using std::string instead of C-style character arrays"),
        
        (r'while\s*\(\s*1\s*\)|while\s*\(\s*true\s*\)', IssueType.STYLE_VIOLATION, Severity.INFO,
         "Infinite loop detected - ensure there's a proper exit condition"),
        
        (r'if\s*\([^)]*=[^)]*\)', IssueType.STYLE_VIOLATION, Severity.WARNING,
         "Assignment in conditional expression - possible mistake (did you mean == instead of =?)"),
        
        (r'goto\s+\w+', IssueType.STYLE_VIOLATION, Severity.INFO,
         "Goto statements make code harder to understand and maintain"),
        
        (r'strcpy|strcat|sprintf|gets', IssueType.BUFFER_OVERFLOW, Severity.WARNING,
         "Unsafe string functions can lead to buffer overflows"),
        
        (r'int\s+\w+\s*;(?!\s*=)', IssueType.UNINITIALIZED_VAR, Severity.WARNING,
         "Uninitialized variable declaration"),
         
        (r'std::vector<\w+>\s+\w+\s*\([^)]*\)(?!\s*\{)', IssueType.STYLE_VIOLATION, Severity.INFO,
         "Consider using uniform initialization with curly braces"),
         
        (r'catch\s*\(\s*\.\.\.\s*\)', IssueType.STYLE_VIOLATION, Severity.WARNING,
         "Catching all exceptions may hide bugs, consider catching specific exception types"),
    ]
    
    # Check each line against the patterns
    for line_num, line in enumerate(lines, 1):
        for pattern, issue_type, severity, message in patterns:
            if re.search(pattern, line):
                # Get built-in explanation and recommendation
                explanation, recommendation = get_built_in_explanation(issue_type, message)
                
                issues.append({
                    "type": issue_type,
                    "severity": severity,
                    "message": message,
                    "line": line_num,
                    "code_snippet": extract_code_snippet(source_code, line_num),
                    "explanation": explanation,
                    "recommendation": recommendation
                })
    
    return issues

def analyze_code_with_openai(source_code: str, issues: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
    """
    Use OpenAI's API to enhance the analysis of C++ code.
    This function will:
    1. Perform general code review
    2. Enhance the explanation of previously detected issues
    """
    # First, get general code review
    try:
        print("Analyzing code with AI...", file=sys.stderr)
        
        # Verify API key is available
        if not OPENAI_API_KEY:
            print("Error: OpenAI API key is not available.", file=sys.stderr)
            print("Please set the OPENAI_API_KEY environment variable and try again.", file=sys.stderr)
            return issues
            
        # Use OpenAI to analyze the entire code
        # the newest OpenAI model is "gpt-4o" which was released May 13, 2024.
        # do not change this unless explicitly requested by the user
        system_prompt = """
        You are an expert C++ code reviewer. Analyze the given C++ code and identify potential issues, bugs,
        performance problems, and style violations. Focus on:
        
        1. Memory management issues (leaks, use-after-free)
        2. Undefined behavior
        3. Performance optimizations
        4. Modern C++ best practices
        5. Security vulnerabilities
        6. Style and readability
        
        For each issue found, provide:
        1. The issue type
        2. Severity level (ERROR, WARNING, INFO, OPTIMIZATION)
        3. Line number or range
        4. Description of the problem
        5. A specific recommendation to fix it
        
        Format your response as a JSON array of objects with the following structure:
        [
          {
            "type": "issue type",
            "severity": "severity level",
            "line": line_number,
            "message": "problem description",
            "recommendation": "how to fix it"
          },
          ...
        ]
        """
        
        try:
            response = openai.chat.completions.create(
                model="gpt-4o",
                messages=[
                    {"role": "system", "content": system_prompt},
                    {"role": "user", "content": f"Analyze this C++ code:\n\n```cpp\n{source_code}\n```"}
                ],
                response_format={"type": "json_object"},
                temperature=0.2,
            )
            
            # Parse the AI's response
            ai_analysis = json.loads(response.choices[0].message.content)
            ai_issues = ai_analysis.get("issues", []) if isinstance(ai_analysis, dict) else ai_analysis
            
            # Process and add AI-detected issues
            for ai_issue in ai_issues:
                if not isinstance(ai_issue, dict):
                    continue
                    
                issues.append({
                    "type": ai_issue.get("type", IssueType.OTHER),
                    "severity": ai_issue.get("severity", Severity.INFO),
                    "message": ai_issue.get("message", "Issue detected"),
                    "line": ai_issue.get("line", 0),
                    "code_snippet": extract_code_snippet(source_code, ai_issue.get("line", 1)),
                    "recommendation": ai_issue.get("recommendation", "No specific recommendation")
                })
        except Exception as e:
            print(f"Warning: Full code analysis failed: {e}", file=sys.stderr)
            print("Continuing with pattern-based analysis only.", file=sys.stderr)
        
        # Now enhance each previously detected issue with AI explanations
        enhanced_issues = []
        for issue in issues:
            # Skip issues that already have recommendations (from AI analysis)
            if "recommendation" in issue:
                enhanced_issues.append(issue)
                continue
                
            try:
                issue_prompt = f"""
                Explain this C++ code issue and provide a fix:
                
                Issue Type: {issue['type']}
                Severity: {issue['severity']}
                Message: {issue['message']}
                Code:
                ```cpp
                {issue['code_snippet']}
                ```
                
                Provide a JSON response with these fields:
                1. "explanation": Detailed explanation of why this is a problem
                2. "recommendation": Specific code example showing how to fix it
                """
                
                response = openai.chat.completions.create(
                    model="gpt-4o",
                    messages=[
                        {"role": "system", "content": "You are an expert C++ programming assistant focused on code review."},
                        {"role": "user", "content": issue_prompt}
                    ],
                    response_format={"type": "json_object"},
                    temperature=0.2,
                )
                
                # Parse the explanation and recommendation
                explanation_data = json.loads(response.choices[0].message.content)
                
                # Add AI-generated explanation and recommendation
                issue["explanation"] = explanation_data.get("explanation", "No explanation available")
                issue["recommendation"] = explanation_data.get("recommendation", "No recommendation available")
                
            except Exception as e:
                # If enhancement fails, keep the original issue
                print(f"Warning: Failed to enhance issue explanation: {e}", file=sys.stderr)
                issue["explanation"] = "Could not generate detailed explanation due to API error."
                issue["recommendation"] = "No specific recommendation available."
            
            enhanced_issues.append(issue)
        
        return enhanced_issues
        
    except Exception as e:
        # Handle common API errors more gracefully
        error_msg = str(e)
        if "insufficient_quota" in error_msg:
            print("Error: Your OpenAI API quota has been exceeded.", file=sys.stderr)
            print("Please check your billing details at https://platform.openai.com/account/billing", file=sys.stderr)
        elif "invalid_api_key" in error_msg:
            print("Error: Invalid OpenAI API key provided.", file=sys.stderr)
            print("Please ensure you've set a valid API key in the OPENAI_API_KEY environment variable.", file=sys.stderr)
        else:
            print(f"Error during AI analysis: {e}", file=sys.stderr)
            
        print("Continuing with pattern-based analysis only.", file=sys.stderr)
        # Return the original issues if AI analysis fails
        return issues

def format_review_results(issues: List[Dict[str, Any]]) -> str:
    """Format the review results into a readable string."""
    if not issues:
        return "No issues found in the code. Great job!\n"
    
    # Count issues by severity
    error_count = sum(1 for issue in issues if issue["severity"] == Severity.ERROR)
    warning_count = sum(1 for issue in issues if issue["severity"] == Severity.WARNING)
    info_count = sum(1 for issue in issues if issue["severity"] == Severity.INFO)
    optimization_count = sum(1 for issue in issues if issue["severity"] == Severity.OPTIMIZATION)
    
    result = []
    result.append("=============================================")
    result.append("            C++ CODE REVIEW RESULTS            ")
    result.append("=============================================\n")
    
    result.append("Summary:")
    result.append(f"  - Errors: {error_count}")
    result.append(f"  - Warnings: {warning_count}")
    result.append(f"  - Information: {info_count}")
    result.append(f"  - Optimization suggestions: {optimization_count}")
    result.append(f"  - Total issues: {len(issues)}\n")
    
    result.append("DETAILED ISSUES:")
    result.append("=============================================\n")
    
    for i, issue in enumerate(issues, 1):
        severity = issue["severity"]
        color = get_color_for_severity(severity)
        
        result.append(f"[{i}] {color}{severity}{Colors.RESET}: {issue['type']}")
        result.append(f"Line: {issue['line']}")
        result.append(f"Message: {issue['message']}\n")
        
        result.append("Code Snippet:")
        result.append("-------------")
        result.append(issue["code_snippet"])
        result.append("")
        
        if "explanation" in issue:
            result.append("Explanation:")
            result.append("------------")
            result.append(issue["explanation"])
            result.append("")
        
        if "recommendation" in issue:
            result.append("Recommended Fix:")
            result.append("----------------")
            result.append(issue["recommendation"])
        
        result.append("=============================================\n")
    
    return "\n".join(result)

def main():
    """Main function to handle command-line arguments and orchestrate the review process."""
    parser = argparse.ArgumentParser(description="AI-Powered C++ Code Reviewer")
    parser.add_argument("-f", "--file", required=True, help="C++ source file to review")
    parser.add_argument("-o", "--output", help="Output file for review results (defaults to console)")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose output")
    args = parser.parse_args()
    
    # Read the source file
    if args.verbose:
        print(f"Reviewing file: {args.file}", file=sys.stderr)
    
    source_code = read_file(args.file)
    
    # Detect common patterns
    if args.verbose:
        print("Performing pattern-based analysis...", file=sys.stderr)
    
    issues = detect_common_patterns(source_code)
    
    # Enhance with AI analysis
    if args.verbose:
        print("Enhancing analysis with OpenAI...", file=sys.stderr)
    
    enhanced_issues = analyze_code_with_openai(source_code, issues)
    
    # Format and output results
    review_results = format_review_results(enhanced_issues)
    
    if args.output:
        try:
            with open(args.output, 'w') as output_file:
                output_file.write(review_results)
            if args.verbose:
                print(f"Review written to {args.output}", file=sys.stderr)
        except Exception as e:
            print(f"Error writing to output file: {e}", file=sys.stderr)
            print(review_results)
    else:
        print(review_results)

if __name__ == "__main__":
    main()