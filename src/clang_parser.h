/**
 * clang_parser.h - Interface for Clang AST parsing functionality
 * 
 * This file declares the ClangParser class which is responsible for parsing
 * C++ source code using Clang libraries and providing an abstract syntax tree
 * for further analysis.
 */

#ifndef CLANG_PARSER_H
#define CLANG_PARSER_H

#include <string>
#include <memory>
#include <vector>
#include <clang-c/Index.h>

/**
 * Represents the location of an issue in source code
 */
struct SourceLocation {
    std::string filename;
    unsigned line;
    unsigned column;
    
    std::string toString() const;
};

/**
 * Represents a node in the AST with relevant attributes
 */
struct ASTNode {
    CXCursorKind kind;
    std::string spelling;
    std::string typeSpelling;
    SourceLocation location;
    std::vector<ASTNode> children;
    
    // Additional useful information extracted from the AST
    bool isDefinition;
    bool isDeclaration;
    bool isVirtual;
    bool isConst;
    bool isStatic;
    
    std::string getKindName() const;
};

/**
 * Container for the parsed AST and additional context
 */
struct ASTContext {
    ASTNode rootNode;
    std::string sourceCode;
    std::string filename;
};

/**
 * Main class for C++ code parsing using Clang
 */
class ClangParser {
public:
    ClangParser();
    ~ClangParser();
    
    /**
     * Parse C++ source code and create an AST context
     * 
     * @param sourceCode The C++ source code to parse
     * @param filename Optional name of the source file
     * @return ASTContext containing the root node and additional information
     * @throws std::runtime_error if parsing fails
     */
    std::shared_ptr<ASTContext> parseCode(const std::string& sourceCode, const std::string& filename = "input.cpp");
    
private:
    /**
     * Convert a Clang cursor to our internal ASTNode representation
     */
    ASTNode convertCursorToNode(CXCursor cursor, CXTranslationUnit tu);
    
    /**
     * Recursively visit all child nodes of a cursor
     */
    void visitChildren(CXCursor cursor, ASTNode& parent, CXTranslationUnit tu);
    
    /**
     * Extract source location information for a cursor
     */
    SourceLocation getSourceLocation(CXCursor cursor, CXTranslationUnit tu);
};

#endif // CLANG_PARSER_H
