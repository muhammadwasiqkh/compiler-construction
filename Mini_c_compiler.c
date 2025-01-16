
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// -------------------------
// 1. Lexical Analysis
// -------------------------
typedef struct {
    char *type;
    char *value;
} Token;

Token *tokens[100];
int token_count = 0;

void add_token(const char *type, const char *value) {
    Token *token = (Token *)malloc(sizeof(Token));
    token->type = strdup(type);
    token->value = strdup(value);
    tokens[token_count++] = token;
}

void tokenize(const char *code) {
    const char *token_specification[][2] = {
        {"NUMBER", "\d+"},
        {"IDENTIFIER", "[a-zA-Z_][a-zA-Z_\d]*"},
        {"ASSIGN", "="},
        {"END", ";"},
        {"OP", "[+\-*/]"},
        {"WHITESPACE", "[ \t]+"}
    };

    const char *ptr = code;
    while (*ptr != '\0') {
        int matched = 0;
        for (int i = 0; i < 6; i++) {
            char regex[100];
            snprintf(regex, sizeof(regex), "^%s", token_specification[i][1]);
            char buffer[100];
            if (sscanf(ptr, regex, buffer) == 1) {
                if (strcmp(token_specification[i][0], "WHITESPACE") != 0) {
                    add_token(token_specification[i][0], buffer);
                }
                ptr += strlen(buffer);
                matched = 1;
                break;
            }
        }
        if (!matched) {
            fprintf(stderr, "Unexpected character: %c\n", *ptr);
            exit(1);
        }
    }
}

// -------------------------
// 2. Syntax Analysis
// -------------------------
typedef struct Node {
    char *type;
    char *value;
    struct Node *left;
    struct Node *right;
} Node;

int position = 0;

Node *new_node(const char *type, const char *value) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = strdup(type);
    node->value = strdup(value);
    node->left = NULL;
    node->right = NULL;
    return node;
}

Node *match(const char *expected_type) {
    if (position < token_count && strcmp(tokens[position]->type, expected_type) == 0) {
        Node *node = new_node(tokens[position]->type, tokens[position]->value);
        position++;
        return node;
    }
    return NULL;
}

Node *expression();

Node *assignment() {
    Node *identifier = match("IDENTIFIER");
    if (!identifier) return NULL;

    if (!match("ASSIGN")) {
        fprintf(stderr, "Syntax Error: Expected '='\n");
        exit(1);
    }

    Node *expr = expression();
    if (!match("END")) {
        fprintf(stderr, "Syntax Error: Expected ';'\n");
        exit(1);
    }

    Node *node = new_node("ASSIGN", "=");
    node->left = identifier;
    node->right = expr;
    return node;
}

Node *expression() {
    Node *term = match("NUMBER");
    if (!term) term = match("IDENTIFIER");

    if (position < token_count && strcmp(tokens[position]->type, "OP") == 0) {
        Node *op = match("OP");
        op->left = term;
        op->right = expression();
        return op;
    }

    return term;
}

// -------------------------
// 3. Code Generation
// -------------------------
void generate_code(Node *node) {
    if (!node) return;

    if (strcmp(node->type, "ASSIGN") == 0) {
        generate_code(node->right);
        printf("STORE %s\n", node->left->value);
    } else if (strcmp(node->type, "OP") == 0) {
        generate_code(node->left);
        generate_code(node->right);
        printf("%s\n", node->value);
    } else if (strcmp(node->type, "NUMBER") == 0 || strcmp(node->type, "IDENTIFIER") == 0) {
        printf("LOAD %s\n", node->value);
    }
}

// -------------------------
// Main Function
// -------------------------
int main() {
    const char *code = "x = 3 + 5;";

    tokenize(code);
    for (int i = 0; i < token_count; i++) {
        printf("Token: Type=%s, Value=%s\n", tokens[i]->type, tokens[i]->value);
    }

    Node *ast = assignment();
    if (!ast) {
        fprintf(stderr, "Syntax Error\n");
        exit(1);
    }

    printf("Generated Code:\n");
    generate_code(ast);

    // Free memory
    for (int i = 0; i < token_count; i++) {
        free(tokens[i]->type);
        free(tokens[i]->value);
        free(tokens[i]);
    }
    return 0;
}
