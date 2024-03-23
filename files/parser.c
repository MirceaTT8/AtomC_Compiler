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
			}else tkerr("Lipseste id-ul struct-ului");
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
		}else tkerr("lipseste ]");
	}
	iTk = start;
	return false;
}

bool varDef(){
	printf("varDef\n ");
	Token* start = iTk;
	if(typeBase()){
		printf("typeBase->varDef\n ");
		if(consume(ID)){
			if(arrayDecl()){}
			if(consume(SEMICOLON)){
				return true;
			}else tkerr("lipseste ;");
		}else tkerr("id missing in a type statement");
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

// expr: exprAssign

bool expr(){
	printf("expr\n");
	Token *start = iTk;
	if(exprAssign()){
		printf("exprAssign->expr\n");
		return true;
	}
	printf("exprfalse\n");
	iTk = start;
	return false;
}

// exprAssign: exprUnary ASSIGN exprAssign | exprOr

bool exprAssign(){
	printf("exprAssign\n");
	Token *start = iTk;
	if(exprUnary()){
		printf("exprUnary->exprAssign\n");
		if(consume(ASSIGN)){
			if(exprAssign()){
				return true;
			}else tkerr("no expression after assign");
		}
		iTk=start;
	}
	if(exprOr()){
		printf("exprOr->exprAssign\n");
		return true;
	}
	printf("exprAssignfalse\n");
	iTk = start;
	return false;
}

//exprOr: exprAnd exprOrPrim
//exprOrPrim: OR exprAnd exprOrPrim | epsilon

bool exprOr(){
	printf("exprOr\n");
	Token* start = iTk;
	if(exprAnd()){
		printf("exprAnd->exprOr\n");
		if(exprOrPrim()){
			return true;
		}
	}
	iTk = start;
	printf("exprOrfalse\n");
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

// exprAnd: exprAnd AND exprEq | exprEq
// exprAnd: exprEq exprAndPrim
// exprAndPrime: AND exprEq exprAndPrim | ε

bool exprAnd() {
	printf("exprAnd\n");
    Token* start = iTk;
	if(exprEq()){
		printf("exprEq->exprAnd\n");
		if(exprAndPrim()){
			return true;
		}
	}
	iTk = start;
	printf("exprAndfalse\n");
	return false;
}

bool exprAndPrim() {
    if (consume(AND)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprEq()) {
            if (exprAndPrim()) {
                return true;
            }
        }tkerr("No expression after &&");
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel
// exprEq: exprRel exprEqPrim
// exprEqPrim: ( EQUAL | NOTEQ ) exprRel exprEqPrim | ε

bool exprEq() {
	printf("exprEq\n");
    Token* start = iTk;
	if(exprRel()){
		printf("exprRel->exprEq\n");
		if(exprEqPrim()){
			return true;
		}
	}
	printf("exprEqfalse\n");
	iTk = start;
	return false;
}

bool exprEqPrim() {
    if (consume(EQUAL) || consume(NOTEQ)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprRel()) {
            if (exprEqPrim()) {
                return true;
            }
        }tkerr("No expression after equality operator");
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd
// exprRel: exprAdd exprRelPrim
// exprRelPrim: ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRelPrim | ε

bool exprRel() {
	printf("exprRel\n");
    Token* start = iTk;
	if(exprAdd()){
		printf("exprAdd->exprRel\n");
		if(exprRelPrim()){
			return true;
		}
	}
	printf("exprRelfalse\n");
	iTk = start;
	return false;
}

bool exprRelPrim() {
    if (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprAdd()) {
            if (exprRelPrim()) {
                return true;
            }
        }tkerr("No expression after relation operator");
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul
// exprAdd: exprMul exprAddPrim
// exprAddPrim: ( ADD | SUB ) exprMul exprAddPrim | ε

bool exprAdd() {
	printf("exprAdd\n");
    Token* start = iTk;
	if(exprMul()){
		printf("exprMul->exprAdd\n");
		if(exprAddPrim()){
			return true;
		}
	}
	printf("exprAddfalse\n");
	iTk = start;
	return false;
}

bool exprAddPrim() {
    if (consume(ADD) || consume(SUB)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprMul()) {
            if (exprAddPrim()) {
                return true;
            }
        }tkerr("No expression after adder operator");
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprMul: exprMul ( MUL | DIV ) exprCast | exprCast
// exprMul: exprCast exprMulPrime
// exprMulPrime: ( MUL | DIV ) exprCast exprMulPrime | ε

bool exprMul() {
	printf("exprMul\n");
    Token* start = iTk;
	if(exprCast()){
		printf("exprCast->exprMul\n");	
		if(exprMulPrim()){
			return true;
		}
	}
	printf("exprMulfalse\n");
	iTk = start;
	return false;
}

bool exprMulPrim() {
    if (consume(MUL) || consume(DIV)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprCast()) {
            if (exprMulPrim()) {
                return true;
            }
        }tkerr("No expression after multiplier operator");
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

bool exprCast(){
	printf("exprCast\n");
	Token* start = iTk;
	if(consume(LPAR)){
		if(typeBase()){
			if(arrayDecl()){}
			if(consume(RPAR)){
				if(exprCast()){
					return true;
				}
			}
		}
	}
	if(exprUnary()){
		printf("exprUnary->exprCast\n");	
		return true;
	}
	printf("exprCastfalse\n");
	iTk = start;
	return false;
}

bool exprUnary(){
	printf("exprUnary\n");
	Token* start = iTk;
	if(consume(SUB)|| consume(ADD)){
		if(exprUnary()){
			return true;
		}
	}
	if(exprPostfix()){
		printf("exprPostfix->exprUnary\n");
		return true;
	}
	printf("exprUnaryfalse\n");
	iTk = start;
	return false;
}

// exprPostfix: exprPostfix LBRACKET expr RBRACKET
//            | exprPostfix DOT ID
//            | exprPrimary
// exprPostfix: exprPrimary exprPostfixPrime
// exprPostfixPrime: ( LBRACKET expr RBRACKET | DOT ID ) exprPostfixPrime | ε

bool exprPostfix() {
	printf("exprPostfix\n");
    Token* start = iTk;
	if(exprPrimary()){
			printf("exprPrimary->exprPostfix\n");

		if(exprPostfixPrim()){
			return true;
		}
	}
	printf("exprPostfixfalse\n");
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
			}else tkerr("lipseste ]");
		}
		iTk=start;
	}
	if(consume(DOT)){
		if(consume(ID)){
			if(exprPostfixPrim()){
				return true;
			}
		}
		iTk=start;
	}
	return true;
}

// exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
//            | INT
//            | DOUBLE
//            | CHAR
//            | STRING
//            | LPAR expr RPAR

bool exprPrimary(){
	printf("exprPrimary\n");
	Token* start = iTk;
	if(consume(ID)){
		if(consume(LPAR)){
			if(expr()){
				printf("expr->exprPrimary");
				while(consume(COMMA)){
					if(!expr()){
						return false;
					}
				}
			}
			if(!consume(RPAR)){
				return false;
			}
		}
		return true;
	}
	else if(consume(DOUBLE)){
		return true;
	}
	else if(consume(CHAR)){
		return true;
	}
	else if(consume(STRING)){
		return true;
	}
	else if(consume(INT)){
		return true;
	}
	else if(consume(LPAR)){
		if(expr()){
			if(consume(RPAR)){
				return true;
			}
		}
		iTk = start;
	}
	printf("exprPrimaryfalse\n");
	iTk = start;
	return false;
}

bool stm(){
	printf("Stm\n");
	Token* start= iTk;
	if(stmCompound()){
		printf("stmCompound->stm\n ");
		return true;
	}
	if(consume(IF)){
		if(consume(LPAR)){
			if(expr()){
				if(consume(RPAR)){
					if(stm()){
						if(consume(ELSE)){
							if(stm()){}else tkerr("else statement mssing");
						}
						return true;
					}else tkerr("if statement mssing");
				}else tkerr("Missing )");
			}else tkerr("Missing if expression");
		}else tkerr("Missing ( after if");
		iTk=start;
	}
	else if(consume(WHILE)){
		if(consume(LPAR)){
			if(expr()){
				if(consume(RPAR)){
					if(stm()){
						return true;
					}
				}
			}
		}
		iTk=start;
	}
	else if(consume(RETURN)){
		if(expr()){}
		if(consume(SEMICOLON)){
			return true;
		}else tkerr("missing ; at return statement");
		iTk=start;
	}
	else if(expr())
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
		// while(varDef() || stm()){
		// 	printf("varDef()/stm()->stmCompound\n ");
		// }
		for(;;){
			if(varDef()){ printf("varDef() -> stmCompound");}
			else if(stm()){ printf("stm() -> stmCompound"); }
			else break;
		}
		if(consume(RACC)){
			return true;
		}else tkerr("Missing } for a statement");
	}
	iTk = start;
	return false;
}


bool fnParam(){
	printf("fnParam\n");
	Token *start=iTk;
	if(typeBase()){
		printf("typeBase->fnParam\n");
		if(consume(ID)){
			if(arrayDecl()){}
			return true;
		}else tkerr("Missing id after typebase");
	}
	iTk=start;
	return false;
}

bool fnDef(){
	printf("fnDef\n");
	Token *start= iTk;
	if(typeBase() || consume(VOID)){
		printf("typeBase->fnDef\n");
		if(consume(ID)){
			if(consume(LPAR)){
				if(fnParam()){
					// while(consume(COMMA) && fnParam()){
					// 	printf("fnParam->fnDef\n");
					// }
					while(consume(COMMA)){
						if(fnParam()){}
						else tkerr("Expected type specifier in function after ,");
					}
				}
				if(consume(RPAR)){
					if(stmCompound()){
						printf("stmCompound->fnDef\n");
						return true;
					}else tkerr("Missing body function");
				}else tkerr("Missing ) in function");
			}else tkerr("Missing arguments of function");
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
				while(varDef()){
					printf("varDef->structDef\n");
				}
				if(consume(RACC)){
					if(consume(SEMICOLON))
					{
						return true;
					}else tkerr("; expected at struct defining");
				}else tkerr("Missing } when defining struct");
			}
		}else tkerr("Lipseste id-ul struct-ului");
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