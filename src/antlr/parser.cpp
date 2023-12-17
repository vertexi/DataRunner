#include "parser.hpp"

#include "antlr4-runtime.h"
#include "CLexer.h"
#include "CParser.h"
#include "CBaseVisitor.h"
#include "CVisitor.h"

std::string parseC(std::string input_str)
{
    antlr4::ANTLRInputStream input(input_str);
    CLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    CParser parser(&tokens);
    auto *tree = parser.declaration();
    auto str = tree->toStringTree(&parser);

    // std::cout << tree->toStringTree(&parser) << std::endl;

    // // Associate a visitor with the Suite context
    // CparseLib::CBaseVisitor visitor;

    // std::string str = visitor.visitExternalDeclaration(parser.externalDeclaration()).type().name();
    // // std::cout << "Visitor output: " << str << std::endl;

    // parser.compilationUnit()->translationUnit()->externalDeclaration();

    // // Associate a listener with the file_input context
    // // std::cout << "Listener output" << std::endl;
    // antlr4::tree::ParseTreeWalker walker;
    // antlr4::ParserRuleContext* fileInput = parser.TranslationUnitContext();
    // Python3BaseListener* listener = new Python3BaseListener();
    // walker.walk(listener, fileInput);

    return str;
}
