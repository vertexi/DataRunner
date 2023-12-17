SET ORIGINAL=%CD%
cd /D "%~dp0"
CALL antlr4-parse ..\src\antlr\C.g4 compilationUnit ./parser_test_case.c -gui -tokens -tree
cd %ORIGINAL%
