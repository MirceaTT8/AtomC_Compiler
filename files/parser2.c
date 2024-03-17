#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"
#include "lexer.h"

Token *iTk;		// the iterator in the tokens list
Token *consumedTk;		// the last consumed token

void tkerr(const char *fmt,...){
	fprintf(stderr,"error in line %d: ",iTk->line);
	va_list va;
	va_start(va,fmt);
	vfprintf(stderr,fmt,va);
	va_end(va);
	fprintf(stderr,"\n");
	exit(EXIT_FAILURE);
	}

// consume pentru atomii lexicali, primeste cod de atomi

bool consume(int code){
	printf("consume(%s)",getTokenNameByPosition(code));
	if(iTk->code==code){
		consumedTk=iTk;
		iTk=iTk->next;
		printf(" => consumed\n");
		return true;
	}
		printf(" => found %s\n",getTokenNameByPosition(iTk->code));
		return false;
}

bool typeBase(){
	printf("typeBase\n ");
	Token *start=iTk;
	if(consume(TYPE_INT)){
		return true;
		}
	else if(consume(TYPE_DOUBLE)){
		return true;
		}
	else if(consume(TYPE_CHAR)){
		return true;
		}
	else if(consume(STRUCT)){
		if(consume(ID)){
			return true;
			}
		}
	iTk=start; // refacere pozitie initiala
	return false;
}


bool arrayDecl(){
	printf("arrayDecl\n");
	Token* start = iTk;
	if(consume(LBRACKET)){
		if(consume(INT)){}
		if(consume(RBRACKET)){
			return true;
		}
	}
	iTk = start;
	return false;
}

bool varDef(){
	printf("varDef\n ");
	Token* start = iTk;
	if(typeBase()){
		if(consume(ID)){
			if(arrayDecl()){}
			if(consume(SEMICOLON)){
				return true;
			}
		}
	}
	iTk = start;
	return false;
}

 //are recursivitate stanga: exprOr: exprOr OR exprAnd | exprAnd
//A:exprOr
//alfa1: OR exprAnd
//beta1: exprAnd
//noul exprOR dupa transformarea asta:




// de folosit si pentru afisa atomul cu care am inceput regula
// const char *tkName(int code){
// 	...
// }

bool expr(){
	Token *start = iTk;
	if(exprAssign()){
		return true;
	}
	iTk = start;
	return false;
}

bool exprAssign(){
	Token *start = iTk;
	if(exprUnary()){
		if(consume(ASSIGN)){
			if(exprAssign()){
				return true;
			}
			if(exprOr()){
				return true;
			}
		}
	}
	iTk = start;
	return false;
}

//exprOr: exprAnd exprOrPrim
//exprOrPrim: OR exprAnd exprOrPrim | epsilon

bool exprOr(){
	Token* start = iTk;
	if(exprAnd()){
		if(exprOrPrim()){
			return true;
		}
	}
	iTk = start;
	return false;

}
bool exprOrPrim() {
    if (consume(OR)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprAnd()) {
            if (exprOrPrim()) {
                return true;
            }
        }
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// bool exprOr(){
// 	if(exprOr()){
// 		if(consume(OR)){
// 			if(exprAnd()){
// 				return true;
// 			}
// 		}
// 	}
// 	if(exprAnd()){
// 		return true;
// 	}
// 	return false;
// }

// exprAnd: exprAnd AND exprEq | exprEq
// exprAnd: exprEq exprAndPrim
// exprAndPrime: AND exprEq exprAndPrim | ε

bool exprAnd() {
    Token* start = iTk;
	if(exprEq()){
		if(exprAndPrim()){
			return true;
		}
	}
	iTk = start;
	return false;
}

bool exprAndPrim() {
    if (consume(AND)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprEq()) {
            if (exprAndPrim()) {
                return true;
            }
        }
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel
// exprEq: exprRel exprEqPrim
// exprEqPrim: ( EQUAL | NOTEQ ) exprRel exprEqPrim | ε

bool exprEq() {
    Token* start = iTk;
	if(exprRel()){
		if(exprEqPrim()){
			return true;
		}
	}
	iTk = start;
	return false;
}

bool exprEqPrim() {
    if (consume(EQUAL) || consume(NOTEQ)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprRel()) {
            if (exprEqPrim()) {
                return true;
            }
        }
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd
// exprRel: exprAdd exprRelPrim
// exprRelPrim: ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRelPrim | ε

bool exprRel() {
    Token* start = iTk;
	if(exprAdd()){
		if(exprRelPrim()){
			return true;
		}
	}
	iTk = start;
	return false;
}

bool exprRelPrim() {
    if (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprAdd()) {
            if (exprRelPrim()) {
                return true;
            }
        }
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul
// exprAdd: exprMul exprAddPrim
// exprAddPrim: ( ADD | SUB ) exprMul exprAddPrim | ε

bool exprAdd() {
    Token* start = iTk;
	if(exprMul()){
		if(exprAddPrim()){
			return true;
		}
	}
	iTk = start;
	return false;
}

bool exprAddPrim() {
    if (consume(ADD) || consume(SUB)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprMul()) {
            if (exprAddPrim()) {
                return true;
            }
        }
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprMul: exprMul ( MUL | DIV ) exprCast | exprCast
// exprMul: exprCast exprMulPrime
// exprMulPrime: ( MUL | DIV ) exprCast exprMulPrime | ε

bool exprMul() {
    Token* start = iTk;
	if(exprCast()){
		if(exprMulPrim()){
			return true;
		}
	}
	iTk = start;
	return false;
}

bool exprMulPrim() {
    if (consume(MUL) || consume(DIV)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprCast()) {
            if (exprMulPrim()) {
                return true;
            }
        }
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

bool exprCast(){
	Token* start = iTk;
	if(consume(LPAR)){
		if(typeBase()){
			if(arrayDecl()){}
			if(consume(RPAR)){
				if(exprCast()){
					return true;
				}
				if(exprUnary()){
					return true;
				}
			}
		}

	}
	iTk = start;
	return false;
}

bool exprUnary(){
	Token* start = iTk;
	if(consume(SUB)|| consume(ADD)){
		if(exprUnary()){
			return true;
		}
		if(exprPostfix()){
			return true;
		}
	}
	iTk = start;
	return false;
}

// exprPostfix: exprPostfix LBRACKET expr RBRACKET
//            | exprPostfix DOT ID
//            | exprPrimary
// exprPostfix: exprPrimary exprPostfixPrime
// exprPostfixPrime: ( LBRACKET expr RBRACKET | DOT ID ) exprPostfixPrime | ε

bool exprPostfix() {
    Token* start = iTk;
	if(exprPrimary()){
		if(exprPostfixPrim()){
			return true;
		}
	}
	iTk = start;
	return false;
}

bool exprPostfixPrim(){
	Token* start = iTk;
	if(consume(LBRACKET)){
		if(expr()){
			if(consume(RBRACKET)){
				if(exprPostfixPrim())
				{
					return true;
				}
			}
		}
	}
	if(consume(DOT)){
		if(consume(ID)){
			if(exprPostfixPrim()){
				return true;
			}
		}
	}
	iTk = start;
	return false;
}

// exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
//            | INT
//            | DOUBLE
//            | CHAR
//            | STRING
//            | LPAR expr RPAR

bool exprPrimary(){
	Token* start = iTk;
	if(consume(ID)){
		if(consume(LPAR)){
			if(expr()){
				while(consume(COMMA)){
					if(!expr()){
						return false;
					}
				}
			}
			if(consume(RPAR)){
				return true;
			}
		}
		return true;
	}
	iTk = start;
	return false;
}

bool stm(){
	printf("Stm\n");
	Token* start= iTk;
	if(stmCompound()){
		return true;
	}
	if(consume(IF)){
		if(consume(LPAR)){
			if(expr()){
				if(consume(RPAR)){
					if(stm()){
						if(consume(ELSE)){
							if(stm()){}
						}
						return true;
					}
				}
			}
		}
	}
	if(consume(WHILE)){
		if(consume(LPAR)){
			if(expr()){
				if(consume(RPAR)){
					if(stm()){
						return true;
					}
				}
			}
		}
	}
	if(consume(RETURN)){
		if(expr()){}
		if(consume(SEMICOLON)){
			return true;
		}
	}
	if(expr())
	if(consume(SEMICOLON)){
		return true;
	}
	iTk = start;
	return false;
}


bool stmCompound(){
	printf("StmCompound\n");
	Token* start = iTk;
	if(consume(LACC)){
		while(varDef() || stm()){}
		if(consume(RACC)){
			return true;
		}
	}
	iTk = start;
	return false;
}



bool fnParam(){
	printf("fnParam\n");
	Token *start=iTk;
	if(typeBase()){
		if(consume(ID)){
			if(arrayDecl()){}
			return true;
		}
	}
	iTk=start;
	return false;
}

bool fnDef(){
	printf("fnDef\n");
	Token *start= iTk;
	if(typeBase() || consume(VOID)){
		if(consume(ID)){
			if(consume(LPAR)){
				if(fnParam()){
					while(consume(COMMA) && fnParam()){}
				}
				if(consume(RPAR)){
					if(stmCompound()){
						return true;
					}
				}
			}
		}
	}

	iTk=start;
	return false;
}

bool structDef(){
	printf("structDef\n");
	Token* start = iTk;
	if(consume(STRUCT)){
		if(consume(ID)){
			if(consume(LACC)){
				while(varDef()){}
				if(consume(RACC)){
					if(consume(SEMICOLON))
					{
						return true;
					}
				}
			}
		}
	}
	iTk = start;
	return false;
}

// unit: ( structDef | fnDef | varDef )* END
bool unit(){
	for(;;){//sa putem verifica oricate reguli pentr "*", repetitie de la 1 la infinit
		if(structDef()){}
		else if(fnDef()){}
		else if(varDef()){}
		else break;
		}
	if(consume(END)){
		return true;
		}
	return false;
}

void parse(Token *tokens){
	iTk=tokens;
	if(!unit())tkerr("syntax error");
	printf("HiParse");
}
