/**
 * code_analyzer.h - Interface for C++ code analysis functionality
 * 
 * This file declares the CodeAnalyzer class which is responsible for analyzing
 * the AST to detect common programming errors, anti-patterns, and potential
 * optimizations in C++ code.
 */

#ifndef CODE_ANALYZER_H
#define CODE_ANALYZER_H

#include <string>
#include <vector>
#include <memory>
#include "clang_parser.h"

/**
 * Represents the severity level of an issue
 */
enum class IssueSeverity {
    Error,      // Definite error in the code
    Warning,    // Potential issue that might cause problems
    Info,       // Informational message or style suggestion
    Optimization // Potential performance improvement
};

/**
 * Represents the type of issue found
 */
enum class IssueType {
    MemoryLeak,
    NullPointerDereference,
    UninitializedVariable,
    ResourceLeak,
    UseAfterFree,
    BufferOverflow,
    IntegerOverflow,
    DivisionByZero,
    DeadCode,
    RedundantCode,
    StyleViolation,
    PerformanceIssue,
    ConcurrencyIssue,
    ApiMisuse,
    Other
};

/**
 * Represents a detected issue in the code
 */
struct CodeIssue {
    IssueType type;
    IssueSeverity severity;
    std::string message;
    SourceLocation location;
    std::string codeSnippet;
    
    // For optimization issues
    std::string optimizationSuggestion;
    
    // Filled in by LLM
    std::string explanation;
    std::string recommendedFix;
};

/**
 * Main class for code analysis
 */
class CodeAnalyzer {
public:
    CodeAnalyzer();
    ~CodeAnalyzer();
    
    /**
     * Analyze the AST to find issues in the code
     * 
     * @param astContext The AST context from the parser
     * @return Vector of detected issues
     */
    std::vector<CodeIssue> analyzeCode(std::shared_ptr<ASTContext> astContext);
    
private:
    // Analysis methods for different types of issues
    void detectMemoryIssues(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode);
    void detectUninitializedVariables(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode);
    void detectResourceLeaks(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode);
    void detectNullPointerDereferences(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode);
    void detectBufferOverflows(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode);
    void detectDeadCode(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode);
    void detectPerformanceIssues(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode);
    void detectCppStyleIssues(const ASTNode& node, std::vector<CodeIssue>& issues, const std::string& sourceCode);
    
    // Helper methods
    std::string extractCodeSnippet(const SourceLocation& location, const std::string& sourceCode, int contextLines = 2);
};

#endif // CODE_ANALYZER_H
