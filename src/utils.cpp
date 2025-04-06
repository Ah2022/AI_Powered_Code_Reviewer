/**
 * utils.cpp - Implementation of utility functions
 */

#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <iomanip>

std::string Utils::readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Utils::writeFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filePath);
    }
    
    file << content;
    
    if (file.fail()) {
        throw std::runtime_error("Failed to write to file: " + filePath);
    }
}

std::string Utils::getEnvVar(const std::string& name) {
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : "";
}

std::string Utils::formatReviewResults(const std::vector<CodeIssue>& issues, const std::string& sourceCode) {
    // Skip processing if there are no issues
    if (issues.empty()) {
        return "No issues found in the code.\n";
    }
    
    std::stringstream result;
    
    // Count issues by severity
    int errorCount = 0;
    int warningCount = 0;
    int infoCount = 0;
    int optimizationCount = 0;
    
    for (const auto& issue : issues) {
        switch (issue.severity) {
            case IssueSeverity::Error: errorCount++; break;
            case IssueSeverity::Warning: warningCount++; break;
            case IssueSeverity::Info: infoCount++; break;
            case IssueSeverity::Optimization: optimizationCount++; break;
        }
    }
    
    // Print summary
    result << "=============================================\n";
    result << "            C++ CODE REVIEW RESULTS            \n";
    result << "=============================================\n\n";
    
    result << "Summary:\n";
    result << "  - Errors: " << errorCount << "\n";
    result << "  - Warnings: " << warningCount << "\n";
    result << "  - Information: " << infoCount << "\n";
    result << "  - Optimization suggestions: " << optimizationCount << "\n";
    result << "  - Total issues: " << issues.size() << "\n\n";
    
    // Print detailed issues
    result << "DETAILED ISSUES:\n";
    result << "=============================================\n\n";
    
    for (size_t i = 0; i < issues.size(); ++i) {
        const auto& issue = issues[i];
        
        result << "[" << (i + 1) << "] " << getColorCode(issue.severity) 
               << issueSeverityToString(issue.severity) << resetColor() 
               << ": " << issueTypeToString(issue.type) << "\n";
        
        result << "Location: " << issue.location.toString() << "\n";
        result << "Message: " << issue.message << "\n\n";
        
        result << "Code Snippet:\n";
        result << "-------------\n";
        result << issue.codeSnippet << "\n";
        
        result << "Explanation:\n";
        result << "------------\n";
        result << issue.explanation << "\n\n";
        
        result << "Recommended Fix:\n";
        result << "----------------\n";
        result << issue.recommendedFix << "\n";
        
        result << "=============================================\n\n";
    }
    
    return result.str();
}

std::string Utils::issueTypeToString(IssueType type) {
    switch (type) {
        case IssueType::MemoryLeak: return "Memory Leak";
        case IssueType::NullPointerDereference: return "Null Pointer Dereference";
        case IssueType::UninitializedVariable: return "Uninitialized Variable";
        case IssueType::ResourceLeak: return "Resource Leak";
        case IssueType::UseAfterFree: return "Use After Free";
        case IssueType::BufferOverflow: return "Buffer Overflow";
        case IssueType::IntegerOverflow: return "Integer Overflow";
        case IssueType::DivisionByZero: return "Division By Zero";
        case IssueType::DeadCode: return "Dead Code";
        case IssueType::RedundantCode: return "Redundant Code";
        case IssueType::StyleViolation: return "Style Violation";
        case IssueType::PerformanceIssue: return "Performance Issue";
        case IssueType::ConcurrencyIssue: return "Concurrency Issue";
        case IssueType::ApiMisuse: return "API Misuse";
        case IssueType::Other: return "Other Issue";
        default: return "Unknown Issue";
    }
}

std::string Utils::issueSeverityToString(IssueSeverity severity) {
    switch (severity) {
        case IssueSeverity::Error: return "ERROR";
        case IssueSeverity::Warning: return "WARNING";
        case IssueSeverity::Info: return "INFO";
        case IssueSeverity::Optimization: return "OPTIMIZATION";
        default: return "UNKNOWN";
    }
}

std::string Utils::getColorCode(IssueSeverity severity) {
    // ANSI color codes for terminal output
    switch (severity) {
        case IssueSeverity::Error: return "\033[1;31m"; // Bold Red
        case IssueSeverity::Warning: return "\033[1;33m"; // Bold Yellow
        case IssueSeverity::Info: return "\033[1;34m"; // Bold Blue
        case IssueSeverity::Optimization: return "\033[1;32m"; // Bold Green
        default: return "\033[0m"; // Reset
    }
}

std::string Utils::resetColor() {
    return "\033[0m"; // ANSI reset code
}
