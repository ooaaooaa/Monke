#ifndef AST_H
#define AST_H
#include <stdlib.h>

typedef struct AST_STRUCT {
    enum {
        AST_VARIABLE_DEFINITION,
        AST_FUNCTION_DEFINITION,
        AST_VARIABLE,
        AST_FUNCTION_CALL,
        AST_STRING,
        AST_COMPOUND
    }type;

    struct SCOPE_STRUCT* scope;

    // AST VARIABLE DEFINITION
    char* variable_definition_variable_name;
    struct AST_STRUCT* variable_definition_value;

    // AST FUCNTION DEFINITION
    struct AST_STRUCT* function_definition_body;
    char* function_definition_name;
    struct AST_STRUCT** function_definition_args;
    size_t function_definition_args_size;

    // AST VARIABLE
    char* variable_name;

    // AST FUNCTION CALL
    char* function_call_name;
    struct AST_STRUCT** function_call_arguments;
    size_t function_call_arguments_size;

    // AST STRING
    char* string_value;

    // AST COMPOUND
    struct AST_STRUCT** compound_value;
    size_t compound_size;
}AST_T;

AST_T* init_ast(int type);
#endif

