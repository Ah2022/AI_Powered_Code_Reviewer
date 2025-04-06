/**
 * code_analyzer.cpp - Implementation of C++ code analysis functionality
 */

#include "code_analyzer.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <set>

CodeAnalyzer::CodeAnalyzer() {
    // Constructor
}

CodeAnalyzer::~CodeAnalyzer() {
    // Destructor
}

std::vector<CodeIssue> CodeAnalyzer::analyzeCode(std::shared_ptr<ASTContext> astContext) {
    if (!astContext) {
        throw std::runtime_error("Invalid AST context");
    }
    
    std::vector<CodeIssue> issues;
    
    // Run various detection algorithms
    detectMemoryIssues(astContext->rootNode, issues, astContext->sourceCode);
    detectUninitializedVariables(astContext->rootNode, issues, astContext->sourceCode);
    detectResourceLeaks(astContext->rootNode, issues, astContext->sourceCode);
    detectNullPointerDereferences(astContext->rootNode, issues, astContext->sourceCode);
    detectBufferOverflows(astContext->rootNode, issues, astContext->sourceCode);
    detectDeadCode(astContext->rootNode, issues, astContext->sourceCode);
    detectPerformanceIssues(astContext->rootNode, issues, astContext->sourceCode);
    detectCppStyleIssues(astContext->rootNode, issues, astContext->sourceCode);
    
    return issues;
}

void CodeAnalyzer::detectMemoryIssues(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode) {
    // Look for new operators without corresponding delete
    if (node.kind == CXCursor_CXXNewExpr) {
        // This is simplified. A proper implementation would track variables and their scope
        CodeIssue issue;
        issue.type = IssueType::MemoryLeak;
        issue.severity = IssueSeverity::Warning;
        issue.message = "Potential memory leak: 'new' used without matching 'delete'";
        issue.location = node.location;
        issue.codeSnippet = extractCodeSnippet(node.location, sourceCode);
        issue.optimizationSuggestion = "Consider using smart pointers like std::unique_ptr or std::shared_ptr";
        issues.push_back(issue);
    }
    
    // Recursively check all children
    for (const auto& child : node.children) {
        detectMemoryIssues(child, issues, sourceCode);
    }
}

void CodeAnalyzer::detectUninitializedVariables(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode) {
    // Check for variable declarations without initializers
    if (node.kind == CXCursor_VarDecl) {
        // This is a simplified check. A proper implementation would analyze if variables are used before initialization
        bool hasInitializer = false;
        
        // Search for initializer in children
        for (const auto& child : node.children) {
            if (child.kind == CXCursor_IntegerLiteral || 
                child.kind == CXCursor_FloatingLiteral || 
                child.kind == CXCursor_StringLiteral ||
                child.kind == CXCursor_CXXBoolLiteralExpr ||
                child.kind == CXCursor_CallExpr) {
                hasInitializer = true;
                break;
            }
        }
        
        if (!hasInitializer && node.typeSpelling.find("*") == std::string::npos) {
            // Report issue for primitive types without initializer
            // This is a simplified check - in a real implementation, we'd check if it's a primitive type
            CodeIssue issue;
            issue.type = IssueType::UninitializedVariable;
            issue.severity = IssueSeverity::Warning;
            issue.message = "Variable '" + node.spelling + "' may be used uninitialized";
            issue.location = node.location;
            issue.codeSnippet = extractCodeSnippet(node.location, sourceCode);
            issue.optimizationSuggestion = "Initialize all variables when declared";
            issues.push_back(issue);
        }
    }
    
    // Recursively check all children
    for (const auto& child : node.children) {
        detectUninitializedVariables(child, issues, sourceCode);
    }
}

void CodeAnalyzer::detectResourceLeaks(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode) {
    // Check for file operations or other resource acquisitions without proper cleanup
    if (node.kind == CXCursor_CallExpr) {
        // Simplified check for common resource acquisition calls
        const std::set<std::string> resourceFunctions = {
            "fopen", "open", "CreateFile", "socket", "malloc", "SDL_CreateWindow"
        };
        
        if (resourceFunctions.find(node.spelling) != resourceFunctions.end()) {
            // This is a simplification. A proper implementation would track variables and their scope
            CodeIssue issue;
            issue.type = IssueType::ResourceLeak;
            issue.severity = IssueSeverity::Warning;
            issue.message = "Potential resource leak: '" + node.spelling + "' call without corresponding release";
            issue.location = node.location;
            issue.codeSnippet = extractCodeSnippet(node.location, sourceCode);
            issue.optimizationSuggestion = "Use RAII pattern with appropriate smart handles for resources";
            issues.push_back(issue);
        }
    }
    
    // Recursively check all children
    for (const auto& child : node.children) {
        detectResourceLeaks(child, issues, sourceCode);
    }
}

void CodeAnalyzer::detectNullPointerDereferences(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode) {
    // Look for pointer dereferences without null checks
    if ((node.kind == CXCursor_MemberRefExpr || node.kind == CXCursor_ArraySubscriptExpr) && 
        node.typeSpelling.find("*") != std::string::npos) {
        
        // This is simplified. A proper implementation would track null checks in the code flow
        CodeIssue issue;
        issue.type = IssueType::NullPointerDereference;
        issue.severity = IssueSeverity::Warning;
        issue.message = "Potential null pointer dereference";
        issue.location = node.location;
        issue.codeSnippet = extractCodeSnippet(node.location, sourceCode);
        issue.optimizationSuggestion = "Add null check before dereferencing pointers";
        issues.push_back(issue);
    }
    
    // Recursively check all children
    for (const auto& child : node.children) {
        detectNullPointerDereferences(child, issues, sourceCode);
    }
}

void CodeAnalyzer::detectBufferOverflows(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode) {
    // Check for unsafe string/buffer operations
    if (node.kind == CXCursor_CallExpr) {
        // List of unsafe functions that can cause buffer overflows
        const std::set<std::string> unsafeFunctions = {
            "strcpy", "strcat", "sprintf", "gets", "scanf"
        };
        
        if (unsafeFunctions.find(node.spelling) != unsafeFunctions.end()) {
            CodeIssue issue;
            issue.type = IssueType::BufferOverflow;
            issue.severity = IssueSeverity::Warning;
            issue.message = "Use of unsafe function '" + node.spelling + "' may lead to buffer overflow";
            issue.location = node.location;
            issue.codeSnippet = extractCodeSnippet(node.location, sourceCode);
            issue.optimizationSuggestion = "Use safer alternatives like strcpy_s, strncpy, snprintf, etc.";
            issues.push_back(issue);
        }
    }
    
    // Recursively check all children
    for (const auto& child : node.children) {
        detectBufferOverflows(child, issues, sourceCode);
    }
}

void CodeAnalyzer::detectDeadCode(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode) {
    // Detect unreachable code after return/break/continue
    if (node.kind == CXCursor_CompoundStmt) {
        bool foundTerminator = false;
        
        for (const auto& child : node.children) {
            if (foundTerminator && 
                (child.kind == CXCursor_DeclStmt || 
                 child.kind == CXCursor_BinaryOperator ||
                 child.kind == CXCursor_CallExpr)) {
                
                CodeIssue issue;
                issue.type = IssueType::DeadCode;
                issue.severity = IssueSeverity::Warning;
                issue.message = "Unreachable code detected after control flow terminator";
                issue.location = child.location;
                issue.codeSnippet = extractCodeSnippet(child.location, sourceCode);
                issue.optimizationSuggestion = "Remove or fix unreachable code";
                issues.push_back(issue);
            }
            
            if (child.kind == CXCursor_ReturnStmt || 
                child.kind == CXCursor_BreakStmt || 
                child.kind == CXCursor_ContinueStmt) {
                foundTerminator = true;
            }
        }
    }
    
    // Recursively check all children
    for (const auto& child : node.children) {
        detectDeadCode(child, issues, sourceCode);
    }
}

void CodeAnalyzer::detectPerformanceIssues(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode) {
    // Check for pass-by-value of large objects
    if (node.kind == CXCursor_ParmDecl) {
        // Check if parameter is passed by value and is a large type
        // In a real implementation, you'd need to analyze the actual type size
        if (node.typeSpelling.find("std::vector") != std::string::npos && 
            node.typeSpelling.find("&") == std::string::npos) {
            
            CodeIssue issue;
            issue.type = IssueType::PerformanceIssue;
            issue.severity = IssueSeverity::Optimization;
            issue.message = "Large object '" + node.spelling + "' passed by value";
            issue.location = node.location;
            issue.codeSnippet = extractCodeSnippet(node.location, sourceCode);
            issue.optimizationSuggestion = "Consider passing by const reference for large objects";
            issues.push_back(issue);
        }
    }
    
    // Check for inefficient loop patterns
    if (node.kind == CXCursor_ForStmt || node.kind == CXCursor_WhileStmt) {
        // This would need more sophisticated analysis in a real implementation
        // to detect inefficient loop patterns like repeated container lookups
        
        // Just a placeholder for the concept
        // In a real implementation, we'd analyze the loop body for specific patterns
    }
    
    // Recursively check all children
    for (const auto& child : node.children) {
        detectPerformanceIssues(child, issues, sourceCode);
    }
}

void CodeAnalyzer::detectCppStyleIssues(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode) {
    // Check for C-style casts instead of C++ casts
    if (node.kind == CXCursor_CStyleCastExpr) {
        CodeIssue issue;
        issue.type = IssueType::StyleViolation;
        issue.severity = IssueSeverity::Info;
        issue.message = "C-style cast detected";
        issue.location = node.location;
        issue.codeSnippet = extractCodeSnippet(node.location, sourceCode);
        issue.optimizationSuggestion = "Use C++ style casts (static_cast, dynamic_cast, etc.)";
        issues.push_back(issue);
    }
    
    // Check for use of 'using namespace std'
    if (node.kind == CXCursor_UsingDirective && node.spelling == "std") {
        CodeIssue issue;
        issue.type = IssueType::StyleViolation;
        issue.severity = IssueSeverity::Info;
        issue.message = "Using directive brings all names from namespace 'std' into global namespace";
        issue.location = node.location;
        issue.codeSnippet = extractCodeSnippet(node.location, sourceCode);
        issue.optimizationSuggestion = "Prefer selective using declarations or namespace qualifiers";
        issues.push_back(issue);
    }
    
    // Check for lack of override keyword in virtual method overrides
    if (node.kind == CXCursor_CXXMethod && node.isVirtual && !node.spelling.empty()) {
        // This is simplified. A real implementation would check base classes to determine if this is an override
        
        // Check if the method has an override specifier already
        bool hasOverrideSpecifier = false;
        std::string codeSnippet = extractCodeSnippet(node.location, sourceCode);
        
        // Very simple check for 'override' keyword in the snippet
        if (codeSnippet.find("override") == std::string::npos) {
            CodeIssue issue;
            issue.type = IssueType::StyleViolation;
            issue.severity = IssueSeverity::Info;
            issue.message = "Virtual method '" + node.spelling + "' might be missing 'override' specifier";
            issue.location = node.location;
            issue.codeSnippet = codeSnippet;
            issue.optimizationSuggestion = "Add 'override' specifier to methods that override virtual functions";
            issues.push_back(issue);
        }
    }
    
    // Recursively check all children
    for (const auto& child : node.children) {
        detectCppStyleIssues(child, issues, sourceCode);
    }
}

std::string CodeAnalyzer::extractCodeSnippet(const SourceLocation& location, const std::string& sourceCode, int contextLines) {
    std::istringstream stream(sourceCode);
    std::string line;
    std::vector<std::string> lines;
    
    // Read all lines
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    
    // Calculate the range of lines to include
    unsigned startLine = (location.line > contextLines) ? location.line - contextLines : 1;
    unsigned endLine = std::min(static_cast<unsigned>(lines.size()), location.line + contextLines);
    
    // Extract the relevant lines
    std::stringstream snippet;
    for (unsigned i = startLine; i <= endLine; ++i) {
        // Line numbers are 1-based, vector indices are 0-based
        if (i > 0 && i <= lines.size()) {
            snippet << i << ": " << lines[i-1] << "\n";
        }
    }
    
    return snippet.str();
}
