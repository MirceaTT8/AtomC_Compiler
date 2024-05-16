#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "lexer.h"
#include "parser.h"
#include "ad.h"
#include "vm.h"


int main(){
    char* inbuf = loadFile("tests/testat.c");
    // puts(inbuf);
    Token *tokens=tokenize(inbuf);
    // showTokens(tokens);
    free(inbuf);
    
    pushDomain();
    vmInit(); 
    
    parse(tokens);
    
    
    showDomain(symTable, "global");
    
    
    Instr *testCode=genTestProgram(); // genereaza cod de test pentru masina virtuala
    run(testCode); // executie cod masina virtuala
    
    dropDomain();
    free(tokens);

    printf("\nByeMain");
    
    return 0;
}
