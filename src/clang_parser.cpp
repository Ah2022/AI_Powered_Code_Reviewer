/**
 * clang_parser.cpp - Implementation of Clang AST parsing functionality
 */

#include "clang_parser.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>

std::string SourceLocation::toString() const {
    std::stringstream ss;
    ss << filename << ":" << line << ":" << column;
    return ss.str();
}

std::string ASTNode::getKindName() const {
    switch (kind) {
        case CXCursor_FunctionDecl: return "Function";
        case CXCursor_ClassDecl: return "Class";
        case CXCursor_StructDecl: return "Struct";
        case CXCursor_VarDecl: return "Variable";
        case CXCursor_ParmDecl: return "Parameter";
        case CXCursor_FieldDecl: return "Field";
        case CXCursor_IfStmt: return "If";
        case CXCursor_WhileStmt: return "While";
        case CXCursor_ForStmt: return "For";
        case CXCursor_ReturnStmt: return "Return";
        case CXCursor_CallExpr: return "FunctionCall";
        case CXCursor_CXXMethod: return "Method";
        case CXCursor_Constructor: return "Constructor";
        case CXCursor_Destructor: return "Destructor";
        case CXCursor_Namespace: return "Namespace";
        default: return "Unknown";
    }
}

ClangParser::ClangParser() {
    // Constructor
}

ClangParser::~ClangParser() {
    // Destructor
}

std::shared_ptr<ASTContext> ClangParser::parseCode(const std::string& sourceCode, const std::string& filename) {
    // Create index with excludeDeclsFromPCH=1, displayDiagnostics=1
    CXIndex index = clang_createIndex(1, 1);
    if (!index) {
        throw std::runtime_error("Failed to create Clang index");
    }
    
    // Define compiler arguments
    const char* args[] = {
        "-std=c++17",
        "-x", "c++",
        "-I/usr/include",
        "-I/usr/local/include"
    };
    int numArgs = sizeof(args) / sizeof(args[0]);
    
    // Create a temporary file containing the source code
    // In a real application, you might want to use a memory buffer or a better temporary file solution
    const char* tempFilename = filename.c_str();
    FILE* tempFile = fopen(tempFilename, "wb");
    if (!tempFile) {
        clang_disposeIndex(index);
        throw std::runtime_error("Failed to create temporary file");
    }
    
    fwrite(sourceCode.c_str(), sizeof(char), sourceCode.length(), tempFile);
    fclose(tempFile);
    
    // Parse the translation unit
    CXTranslationUnit tu = nullptr;
    CXErrorCode error = clang_parseTranslationUnit2(
        index,
        tempFilename,
        args, numArgs,
        nullptr, 0,
        CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_KeepGoing,
        &tu
    );
    
    if (error != CXError_Success || !tu) {
        clang_disposeIndex(index);
        std::stringstream ss;
        ss << "Failed to parse translation unit. Error code: " << error;
        throw std::runtime_error(ss.str());
    }
    
    // Check for diagnostics (errors/warnings)
    unsigned numDiagnostics = clang_getNumDiagnostics(tu);
    bool hasFatalError = false;
    
    for (unsigned i = 0; i < numDiagnostics; ++i) {
        CXDiagnostic diagnostic = clang_getDiagnostic(tu, i);
        CXDiagnosticSeverity severity = clang_getDiagnosticSeverity(diagnostic);
        
        if (severity == CXDiagnostic_Error || severity == CXDiagnostic_Fatal) {
            hasFatalError = true;
        }
        
        clang_disposeDiagnostic(diagnostic);
    }
    
    if (hasFatalError) {
        clang_disposeTranslationUnit(tu);
        clang_disposeIndex(index);
        throw std::runtime_error("Failed to parse source code due to compilation errors");
    }
    
    // Get the root cursor
    CXCursor rootCursor = clang_getTranslationUnitCursor(tu);
    
    // Create a new AST context and populate it
    auto context = std::make_shared<ASTContext>();
    context->sourceCode = sourceCode;
    context->filename = filename;
    context->rootNode = convertCursorToNode(rootCursor, tu);
    
    // Recursively visit children
    visitChildren(rootCursor, context->rootNode, tu);
    
    // Clean up
    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);
    
    return context;
}

ASTNode ClangParser::convertCursorToNode(CXCursor cursor, CXTranslationUnit tu) {
    ASTNode node;
    
    // Basic information
    node.kind = clang_getCursorKind(cursor);
    node.spelling = clang_getCString(clang_getCursorSpelling(cursor));
    
    // Type information
    CXType type = clang_getCursorType(cursor);
    node.typeSpelling = clang_getCString(clang_getTypeSpelling(type));
    
    // Location
    node.location = getSourceLocation(cursor, tu);
    
    // Additional attributes
    node.isDefinition = clang_isCursorDefinition(cursor);
    node.isDeclaration = clang_isDeclaration(node.kind);
    node.isVirtual = clang_CXXMethod_isVirtual(cursor);
    node.isConst = clang_CXXMethod_isConst(cursor);
    node.isStatic = clang_CXXMethod_isStatic(cursor);
    
    return node;
}

void ClangParser::visitChildren(CXCursor cursor, ASTNode& parent, CXTranslationUnit tu) {
    // Use a lambda to visit each child node
    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData clientData) {
            auto data = reinterpret_cast<std::tuple<ASTNode*, ClangParser*, CXTranslationUnit>*>(clientData);
            ASTNode* parentNode = std::get<0>(*data);
            ClangParser* parser = std::get<1>(*data);
            CXTranslationUnit tu = std::get<2>(*data);
            
            // Convert the current child to an ASTNode
            ASTNode childNode = parser->convertCursorToNode(child, tu);
            
            // Recursively visit its children
            parser->visitChildren(child, childNode, tu);
            
            // Add to parent's children
            parentNode->children.push_back(childNode);
            
            return CXChildVisit_Continue;
        },
        &std::make_tuple(&parent, this, tu)
    );
}

SourceLocation ClangParser::getSourceLocation(CXCursor cursor, CXTranslationUnit tu) {
    SourceLocation location;
    CXSourceLocation cxLocation = clang_getCursorLocation(cursor);
    
    CXFile file;
    unsigned line, column, offset;
    clang_getSpellingLocation(cxLocation, &file, &line, &column, &offset);
    
    location.line = line;
    location.column = column;
    
    if (file) {
        CXString fileName = clang_getFileName(file);
        location.filename = clang_getCString(fileName);
        clang_disposeString(fileName);
    }
    
    return location;
}
