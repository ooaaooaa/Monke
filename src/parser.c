#include "include/parser.h"
#include <stdio.h>
#include <string.h>

parser_t* init_parser(lexer_t* lexer) {
    parser_t* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->current_token = lexer_get_next_token(lexer);

    return parser;
}

void parser_eat(parser_t* parser, int token_type) {
    if (parser->current_token->type == token_type) {
        parser->current_token = lexer_get_next_token(parser->lexer);
    } else {
        printf(
                "Unexpected token '%s', with type %d",
                parser->current_token->value,
                parser->current_token->type
        );
        exit(1);
    }
}

AST_T* parser_parse(parser_t* parser, scope_t* scope) {
    return parser_parse_statements(parser, scope);
}

AST_T* parser_parse_statement(parser_t* parser, scope_t* scope) {
    switch (parser->current_token->type) {
        case TOKEN_ID: return parser_parse_id(parser, scope);
    }
}

AST_T* parser_parse_statements(parser_t* parser, scope_t* scope) {
    AST_T* compound = init_ast(AST_COMPOUND);

    compound->compound_value = calloc(1, sizeof(struct AST_STRUCT*));

    AST_T* ast_statement = parser_parse_statement(parser, scope);
    compound->compound_value[0] = ast_statement;

    // Check for semicolon
    while (parser->current_token->type == TOKEN_SEMI) {
        parser_eat(parser, TOKEN_SEMI);

        AST_T* ast_statement = parser_parse_statement(parser, scope);
        compound->compound_size += 1;
        compound->compound_value = realloc(
            compound->compound_value,
            compound->compound_size * sizeof(struct AST_STRUCT*)
        );
        compound->compound_value[compound->compound_size - 1] = ast_statement;
    }

    return compound;
}

AST_T* parser_parse_expression(parser_t* parser, scope_t* scope) {
    switch(parser->current_token->type) {
        case TOKEN_STRING: return parser_parse_string(parser, scope);
        case TOKEN_ID: return parser_parse_id(parser, scope);
    }
}

AST_T* parser_parse_factor(parser_t* parser, scope_t* scope) {}

AST_T* parser_parse_term(parser_t* parser, scope_t* scope) {}

AST_T* parser_parse_function_call(parser_t* parser, scope_t* scope) {
    AST_T* function_call = init_ast(AST_FUNCTION_CALL);

    function_call->function_call_arguments = calloc(1, sizeof(struct AST_STRUCT*));

    AST_T* ast_expression = parser_parse_expression(parser, scope);
    function_call->function_call_arguments[0] = ast_expression;
    function_call->function_call_arguments_size += 1;

    while (parser->current_token->type == TOKEN_COMMA) {
        parser_eat(parser, TOKEN_COMMA);

        AST_T* ast_expression = parser_parse_expression(parser, scope);
        function_call->function_call_arguments_size += 1;
        function_call->function_call_arguments =
            realloc(
                function_call->function_call_arguments,
                function_call->function_call_arguments_size * sizeof(struct AST_STRUCT*)
            );
        function_call->function_call_arguments[
            function_call->function_call_arguments_size - 1] = ast_expression;
    }
    parser_eat(parser, TOKEN_RPAREN);

    function_call->scope = scope;

    return function_call;
}

AST_T* parser_parse_function_definition(parser_t* parser, scope_t* scope) {
    AST_T* ast = init_ast(AST_FUNCTION_DEFINITION);
    parser_eat(parser, TOKEN_ID); // function

    char* function_name = parser->current_token->value;
    ast->function_definition_name = calloc(strlen(function_name) + 1, sizeof(char));
    strcpy(ast->function_definition_name, function_name);

    parser_eat(parser, TOKEN_ID); // function name

    parser_eat(parser, TOKEN_LPAREN);

    ast->function_definition_args = calloc(1, sizeof(struct AST_STRUCT*));

    AST_T* arg = parser_parse_variable_definition(parser, scope);
    ast->function_definition_args_size += 1;
    ast->function_definition_args[ast->function_definition_args_size - 1] = arg;

    while (parser->current_token->type == TOKEN_COMMA) {
        parser_eat(parser, TOKEN_COMMA);

        ast->function_definition_args_size += 1;

        ast->function_definition_args =
            realloc(
                ast->function_definition_args,
                ast->function_call_arguments_size * sizeof(struct AST_STRUCT*)
            );
        AST_T* arg = parser_parse_variable(parser, scope);
        ast->function_definition_args[ast->function_definition_args_size - 1] = arg;
    }

    parser_eat(parser, TOKEN_RPAREN);

    parser_eat(parser, TOKEN_LBRACE);

    ast->function_definition_body = parser_parse_statements(parser, scope);

    parser_eat(parser, TOKEN_RBRACE);

    ast->scope = scope;

    return ast;
}

AST_T* parser_parse_variable_definition(parser_t* parser, scope_t* scope) {
    parser_eat(parser, TOKEN_ID); // Var
    char* variable_definition_variable_name = parser->current_token->value;
    parser_eat(parser, TOKEN_ID); // Var name
    parser_eat(parser, TOKEN_EQUALS);
    AST_T* variable_definition_value = parser_parse_expression(parser, scope); // Var value

    // Define a var with a name and value
    AST_T* variable_definition = init_ast(AST_VARIABLE_DEFINITION);
    variable_definition->variable_definition_variable_name = variable_definition_variable_name;
    variable_definition->variable_definition_value = variable_definition_value;

    return variable_definition;
}

AST_T* parser_parse_variable(parser_t* parser, scope_t* scope) {
    char* token_value = parser->current_token->value;
    parser_eat(parser, TOKEN_ID); // Var name or function call name

    // If there are paranthesis, assume we are in a function
    if (parser->current_token->type == TOKEN_LPAREN)
        return parser_parse_function_call(parser, scope);

    AST_T* ast_variable = init_ast(AST_VARIABLE);
    ast_variable->variable_name = token_value;

    return ast_variable;
}

AST_T* parser_parse_string(parser_t* parser, scope_t* scope) {
    AST_T* ast_string = init_ast(AST_STRING);
    ast_string->string_value = parser->current_token->value;

    parser_eat(parser, TOKEN_STRING);

    return ast_string;
}

AST_T* parser_parse_id(parser_t* parser, scope_t* scope) {
    // Checking if we are trying to define a varable
    // Naming for this will change later

    // If we are defining a variable
    if (strcmp(parser->current_token->value, "var") == 0) {
        return parser_parse_variable_definition(parser, scope);
    }
    else
    if (strcmp(parser->current_token->value, "function") == 0) {
        return parser_parse_function_definition(parser, scope);
    }
    else {
        return parser_parse_variable(parser, scope);
    }
}

