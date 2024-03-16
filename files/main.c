#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "lexer.h"
#include "parser.h"

int main(){
    char* inbuf = loadFile("tests/mytestparser.c");
    puts(inbuf);
    Token *tokens=tokenize(inbuf);
    parse(tokens);
    // showTokens(tokens);
    printf("\nByeMain");
    free(inbuf);
    free(tokens);
    return 0;
}