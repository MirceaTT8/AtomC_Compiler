#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "lexer.h"
#include "parser.h"

int main(){
    char* inbuf = loadFile("tests/testparser.c");
    puts(inbuf);
    Token *tokens=tokenize(inbuf);
    showTokens(tokens);
    parse(tokens);
    printf("\nByeMain");
    free(inbuf);
    free(tokens);
    return 0;
}