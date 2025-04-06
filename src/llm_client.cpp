/**
 * llm_client.cpp - Implementation of LLM API integration
 */

#include "llm_client.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

// Static callback function for CURL
size_t LlmClient::writeCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
        return newLength;
    } catch(std::bad_alloc& e) {
        // Handle memory allocation failure
        return 0;
    }
}

LlmClient::LlmClient(const std::string& apiKey, const std::string& model) 
    : apiKey(apiKey), model(model) {
    // Initialize CURL
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
}

LlmClient::~LlmClient() {
    // Clean up
    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

std::vector<CodeIssue> LlmClient::enhanceWithExplanations(const std::vector<CodeIssue>& issues, const std::string& sourceCode) {
    std::vector<CodeIssue> enhancedIssues;
    
    for (const auto& issue : issues) {
        CodeIssue enhancedIssue = issue;
        
        try {
            // Generate a prompt specifically for this issue
            std::string prompt = generatePromptForIssue(issue, sourceCode);
            
            // Query the LLM API
            std::string response = queryLLM(prompt);
            
            // Parse the response to extract explanation and recommended fix
            auto [explanation, recommendedFix] = parseResponse(response);
            
            // Enhance the issue with LLM-provided information
            enhancedIssue.explanation = explanation;
            enhancedIssue.recommendedFix = recommendedFix;
        } catch (const std::exception& e) {
            // If there's an error, provide a default message
            enhancedIssue.explanation = "Failed to generate explanation: " + std::string(e.what());
            enhancedIssue.recommendedFix = "No fix available due to API error.";
        }
        
        enhancedIssues.push_back(enhancedIssue);
    }
    
    return enhancedIssues;
}

std::string LlmClient::queryLLM(const std::string& prompt) {
    if (!curl) {
        throw std::runtime_error("CURL not initialized");
    }
    
    // Prepare request to OpenAI API
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string authHeader = "Authorization: Bearer " + apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());
    
    // the newest OpenAI model is "gpt-4o" which was released May 13, 2024.
    // do not change this unless explicitly requested by the user
    
    // Build the request JSON
    json requestJson = {
        {"model", model},
        {"messages", json::array({
            {{"role", "system"}, {"content", "You are a C++ programming expert providing code review assistance."}},
            {{"role", "user"}, {"content", prompt}}
        })},
        {"temperature", 0.2},
        {"max_tokens", 500},
        {"response_format", {{"type", "json_object"}}}
    };
    
    std::string requestString = requestJson.dump();
    
    // Set up the request
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestString.c_str());
    
    // Set up response handling
    std::string responseString;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
    
    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    
    // Clean up headers
    curl_slist_free_all(headers);
    
    // Check for errors
    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("CURL request failed: ") + curl_easy_strerror(res));
    }
    
    // Parse the response
    try {
        json responseJson = json::parse(responseString);
        std::string content = responseJson["choices"][0]["message"]["content"];
        return content;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to parse API response: ") + e.what() + "\nResponse: " + responseString);
    }
}

std::string LlmClient::generatePromptForIssue(const CodeIssue& issue, const std::string& sourceCode) {
    std::stringstream prompt;
    
    // Build a detailed prompt with all relevant information
    prompt << "I need help understanding and fixing a C++ code issue. Please provide a detailed explanation and suggested fix in JSON format.\n\n";
    
    // Include the issue details
    prompt << "Issue Type: ";
    switch (issue.type) {
        case IssueType::MemoryLeak: prompt << "Memory Leak"; break;
        case IssueType::NullPointerDereference: prompt << "Null Pointer Dereference"; break;
        case IssueType::UninitializedVariable: prompt << "Uninitialized Variable"; break;
        case IssueType::ResourceLeak: prompt << "Resource Leak"; break;
        case IssueType::UseAfterFree: prompt << "Use After Free"; break;
        case IssueType::BufferOverflow: prompt << "Buffer Overflow"; break;
        case IssueType::IntegerOverflow: prompt << "Integer Overflow"; break;
        case IssueType::DivisionByZero: prompt << "Division By Zero"; break;
        case IssueType::DeadCode: prompt << "Dead Code"; break;
        case IssueType::RedundantCode: prompt << "Redundant Code"; break;
        case IssueType::StyleViolation: prompt << "Style Violation"; break;
        case IssueType::PerformanceIssue: prompt << "Performance Issue"; break;
        case IssueType::ConcurrencyIssue: prompt << "Concurrency Issue"; break;
        case IssueType::ApiMisuse: prompt << "API Misuse"; break;
        case IssueType::Other: prompt << "Other Issue"; break;
    }
    prompt << "\n";
    
    prompt << "Severity: ";
    switch (issue.severity) {
        case IssueSeverity::Error: prompt << "Error"; break;
        case IssueSeverity::Warning: prompt << "Warning"; break;
        case IssueSeverity::Info: prompt << "Info"; break;
        case IssueSeverity::Optimization: prompt << "Optimization"; break;
    }
    prompt << "\n";
    
    prompt << "Message: " << issue.message << "\n";
    prompt << "Location: " << issue.location.toString() << "\n";
    prompt << "Code Snippet:\n" << issue.codeSnippet << "\n";
    
    if (!issue.optimizationSuggestion.empty()) {
        prompt << "Initial Suggestion: " << issue.optimizationSuggestion << "\n";
    }
    
    prompt << "\nPlease provide your response in this JSON format:\n";
    prompt << "{\n";
    prompt << "  \"explanation\": \"Detailed explanation of the issue, why it's a problem, and the potential consequences\",\n";
    prompt << "  \"recommended_fix\": \"Specific code example showing how to fix the issue\"\n";
    prompt << "}\n";
    
    return prompt.str();
}

std::pair<std::string, std::string> LlmClient::parseResponse(const std::string& response) {
    try {
        // Parse the JSON response
        json responseJson = json::parse(response);
        
        // Extract the explanation and recommended fix
        std::string explanation = responseJson["explanation"];
        std::string recommendedFix = responseJson["recommended_fix"];
        
        return std::make_pair(explanation, recommendedFix);
    } catch (const std::exception& e) {
        // If we can't parse as JSON, try to extract information heuristically
        // This is a fallback for when the LLM doesn't properly format as JSON
        
        std::string explanation = "Failed to parse response as JSON. Raw response:\n\n" + response;
        std::string recommendedFix = "No structured fix available.";
        
        return std::make_pair(explanation, recommendedFix);
    }
}
