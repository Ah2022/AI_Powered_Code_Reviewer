/**
 * utils.h - Utility functions for the code reviewer
 * 
 * This file declares various utility functions used throughout the application.
 */

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include "code_analyzer.h"

/**
 * Utility functions for the application
 */
class Utils {
public:
    /**
     * Read the contents of a file
     * 
     * @param filePath Path to the file
     * @return String containing the file contents
     * @throws std::runtime_error if the file cannot be read
     */
    static std::string readFile(const std::string& filePath);
    
    /**
     * Write content to a file
     * 
     * @param filePath Path to the file
     * @param content Content to write
     * @throws std::runtime_error if the file cannot be written
     */
    static void writeFile(const std::string& filePath, const std::string& content);
    
    /**
     * Get the value of an environment variable
     * 
     * @param name Name of the environment variable
     * @return Value of the environment variable, or empty string if not set
     */
    static std::string getEnvVar(const std::string& name);
    
    /**
     * Format review results into a readable string
     * 
     * @param issues Vector of code issues
     * @param sourceCode Original source code
     * @return Formatted review results as a string
     */
    static std::string formatReviewResults(const std::vector<CodeIssue>& issues, const std::string& sourceCode);
    
    /**
     * Convert enum values to strings
     */
    static std::string issueTypeToString(IssueType type);
    static std::string issueSeverityToString(IssueSeverity severity);
    
    /**
     * Get syntax highlighting or formatting for terminal output
     */
    static std::string getColorCode(IssueSeverity severity);
    static std::string resetColor();
};

#endif // UTILS_H
