#include "parser.hpp"

#include <charconv>

#include "antlr4-runtime.h"
#include "CLexer.h"
#include "CParser.h"
#include "CBaseVisitor.h"
#include "CVisitor.h"

#include "hello_imgui/hello_imgui.h"

std::string parseC(const std::string & codeToParse)
{
    antlr4::ANTLRInputStream input(codeToParse);
    CLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    CParser parser(&tokens);
    std::string declarationTypeRes = "";

    // get variable declarations from CST
    auto declarations = parser.translationUnit()->externalDeclaration();

    for (auto declaration : declarations)
    {
        // if is a variable declaration
        if (auto variableDeclaration = declaration->declaration())
        {
            bool isArray = false;
            bool hasInit = false;
            bool parseError = false;
            uint64_t arraySize = 0;
            std::string typeName = "";
            std::string varName = "";
            std::string errorInfo = "";

            auto variableInitList = variableDeclaration->initDeclaratorList();
            auto variableInit = variableInitList ? variableInitList->initDeclarator() : std::vector<CParser::InitDeclaratorContext *>();
            if (variableInit.size() >= 1)
            {
                // got init declarator, perphaps a variable declaration with init value or array declaration
                hasInit = true;
                auto variableNameAndSize = variableInit[0]->declarator()->directDeclarator();
                if (variableNameAndSize->LeftBracket())
                {
                    // array declaration
                    // TODO: Hex conversion is not supported yet maybe use stoi instead
                    isArray = true;
                    if (variableNameAndSize->assignmentExpression())
                    {
                        uint64_t arraySizeTmp = 0;
                        auto arraySizeStr = variableNameAndSize->assignmentExpression()->getText();
                        if (std::from_chars(arraySizeStr.data(), arraySizeStr.data() + arraySizeStr.size(), arraySizeTmp).ec  == std::errc{})
                        {
                            arraySize = arraySizeTmp;
                        } else {
                            parseError = true;
                            errorInfo = "array size convert error: " + arraySizeStr;
                        }
                    } else {
                        parseError = true;
                        errorInfo = "array size not found";
                    }
                }
                auto variableName = variableNameAndSize->directDeclarator();
                varName = variableName ? variableName->getText() : "unknown";
            }
            auto declarationSpecifiers = variableDeclaration->declarationSpecifiers()->declarationSpecifier();
            if (hasInit)
            {
                if (declarationSpecifiers.size() > 1)
                {
                    // TODO: handle multiple declaration specifiers
                    typeName = "error: Multiple declaration specifiers";
                } else {
                    typeName = declarationSpecifiers[0] -> typeSpecifier()->getText();
                }
            } else {
                if (declarationSpecifiers.size() > 2)
                {
                    // TODO: handle multiple declaration specifiers
                    typeName = "error: Multiple declaration specifiers";
                } else if (declarationSpecifiers.size() == 2) {
                    typeName = declarationSpecifiers[0]->getText();
                    varName = declarationSpecifiers[1]->getText();
                } else {
                    typeName = "error: No declaration specifiers";
                }
            }

            declarationTypeRes += "type: ";
            declarationTypeRes += typeName;
            declarationTypeRes += " ";
            declarationTypeRes += "name: ";
            declarationTypeRes += varName;
            declarationTypeRes += " ";
            if (isArray)
            {
                declarationTypeRes += "arrysize: ";
                declarationTypeRes += std::to_string(arraySize);
                declarationTypeRes += " ";
            }
            if (parseError)
            {
                declarationTypeRes += "error: ";
                declarationTypeRes += errorInfo;
                declarationTypeRes += " ";
            }
            declarationTypeRes += "\n";
        }
    }

    return declarationTypeRes;
}
