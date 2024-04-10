#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "lexer.h"
#include "parser.h"
#include "ad.h"
#include "vm.h"


int main(){
    char* inbuf = loadFile("tests/testad.c");
    // puts(inbuf);
    Token *tokens=tokenize(inbuf);
    // showTokens(tokens);
    free(inbuf);
    
    pushDomain();
    
    parse(tokens);
    
    
    showDomain(symTable, "global");
    dropDomain();
    free(tokens);

    printf("\nByeMain");
    
    return 0;
}