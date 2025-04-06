/**
 * main.cpp - Entry point for the C++ Code Reviewer
 * 
 * This file contains the main function and CLI setup for the code reviewer tool.
 * It processes command line arguments, initializes components, and coordinates
 * the code review workflow.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#include "../include/CLI11.hpp"
#include "clang_parser.h"
#include "code_analyzer.h"
#include "llm_client.h"
#include "utils.h"

int main(int argc, char** argv) {
    CLI::App app{"AI-Powered C++ Code Reviewer"};
    
    // Define command line arguments
    std::string inputFile;
    std::string outputFile;
    bool verbose = false;
    std::string apiKey;
    std::string llmModel = "gpt-4o"; // Default to OpenAI's latest model
    
    app.add_option("-f,--file", inputFile, "C++ source file to review")->required();
    app.add_option("-o,--output", outputFile, "Output file for review results (defaults to console)");
    app.add_flag("-v,--verbose", verbose, "Enable verbose output");
    app.add_option("--api-key", apiKey, "API key for LLM service (defaults to OPENAI_API_KEY env var)");
    app.add_option("--model", llmModel, "LLM model to use (defaults to gpt-4o)");
    
    // Parse CLI arguments
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }
    
    // If no API key provided, try to get from environment
    if (apiKey.empty()) {
        apiKey = Utils::getEnvVar("OPENAI_API_KEY");
        if (apiKey.empty()) {
            std::cerr << "Error: No API key provided. Either use --api-key option or set OPENAI_API_KEY environment variable." << std::endl;
            return 1;
        }
    }
    
    if (verbose) {
        std::cout << "Reviewing file: " << inputFile << std::endl;
    }
    
    try {
        // Read the source file
        std::string sourceCode = Utils::readFile(inputFile);
        
        // Initialize components
        auto parser = std::make_unique<ClangParser>();
        auto analyzer = std::make_unique<CodeAnalyzer>();
        auto llmClient = std::make_unique<LlmClient>(apiKey, llmModel);
        
        // Parse the C++ code using Clang
        if (verbose) {
            std::cout << "Parsing C++ code with Clang..." << std::endl;
        }
        
        auto astContext = parser->parseCode(sourceCode, inputFile);
        
        // Analyze the code for issues
        if (verbose) {
            std::cout << "Analyzing code for issues..." << std::endl;
        }
        
        auto issues = analyzer->analyzeCode(astContext);
        
        // Generate explanations and suggestions using LLM
        if (verbose) {
            std::cout << "Generating explanations with " << llmModel << "..." << std::endl;
        }
        
        auto reviewResults = llmClient->enhanceWithExplanations(issues, sourceCode);
        
        // Format and output the results
        std::string formattedReview = Utils::formatReviewResults(reviewResults, sourceCode);
        
        if (!outputFile.empty()) {
            Utils::writeFile(outputFile, formattedReview);
            if (verbose) {
                std::cout << "Review written to " << outputFile << std::endl;
            }
        } else {
            std::cout << formattedReview << std::endl;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
