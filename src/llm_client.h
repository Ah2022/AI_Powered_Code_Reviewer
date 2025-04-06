/**
 * llm_client.h - Interface for LLM API integration
 * 
 * This file declares the LlmClient class which is responsible for communicating
 * with Large Language Model APIs (like OpenAI) to enhance code review with
 * natural language explanations and suggestions.
 */

#ifndef LLM_CLIENT_H
#define LLM_CLIENT_H

#include <string>
#include <vector>
#include <memory>
#include <curl/curl.h>
#include "../include/json.hpp"
#include "code_analyzer.h"

using json = nlohmann::json;

/**
 * Main class for LLM API integration
 */
class LlmClient {
public:
    /**
     * Constructor
     * 
     * @param apiKey API key for the LLM service
     * @param model LLM model name to use (default: gpt-4o)
     */
    LlmClient(const std::string& apiKey, const std::string& model = "gpt-4o");
    
    /**
     * Destructor
     */
    ~LlmClient();
    
    /**
     * Enhance code issues with LLM-provided explanations and fixes
     * 
     * @param issues Vector of code issues detected by the analyzer
     * @param sourceCode Original source code for context
     * @return Enhanced vector of issues with explanations and suggestions
     */
    std::vector<CodeIssue> enhanceWithExplanations(const std::vector<CodeIssue>& issues, const std::string& sourceCode);
    
private:
    std::string apiKey;
    std::string model;
    CURL* curl;
    
    /**
     * Send a request to the LLM API
     * 
     * @param prompt The prompt to send to the LLM
     * @return LLM's response as a string
     */
    std::string queryLLM(const std::string& prompt);
    
    /**
     * Generate a prompt for a specific code issue
     * 
     * @param issue The code issue to explain
     * @param sourceCode Original source code for context
     * @return Formatted prompt string
     */
    std::string generatePromptForIssue(const CodeIssue& issue, const std::string& sourceCode);
    
    /**
     * Parse the LLM response to extract explanation and fix
     * 
     * @param response The LLM response text
     * @return Pair containing explanation and recommended fix
     */
    std::pair<std::string, std::string> parseResponse(const std::string& response);
    
    /**
     * CURL write callback function to capture response
     */
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* s);
};

#endif // LLM_CLIENT_H
