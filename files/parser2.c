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
	if(iTk->code==code){
		consumedTk=iTk;
		iTk=iTk->next;
		printf(" %s ",getTokenNameByPosition(iTk->code));
		return true;
		}
	return false;
	}

// typeBase: TYPE_INT | TYPE_DOUBLE | TYPE_CHAR | STRUCT ID
// atentie TOTUL SAU NIMIC
// daca functia consuma toata regula, va returna true

// bool exprOr(){ //are recursivitate stanga: exprOr: exprOr OR exprAnd | exprAnd
//A:exprOr
//alfa1: OR exprAnd
//beta1: exprAnd
//exprOr: exprAnd exprOrPrim
//exprOrPrim: OR exprAnd exprOrPrim | epsilon
//noul exprOR dupa transformarea asta:

/*


de folosit si pentru afisa atomul cu care am inceput regula
const char *tkName(int code){
	...
}

bool exprOr(){
	printf("#exprOr\n")
	if(exprAnd()){
		if(exprOrPrim){
			return true;
		}
	}
	return false;

}
bool exprOrPrim(){
	
	if(consume(OR)){
		if(exprAnd()){
			if(exprOrPrim()){
				return true;
			}
		}
	}
	return true; //epsilon
}
*/
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

bool typeBase(){
	printf("typeBase ");
	Token *start=iTk;
	if(consume(TYPE_INT)){
		return true;
		}
	if(consume(TYPE_DOUBLE)){
		return true;
		}
	if(consume(TYPE_CHAR)){
		return true;
		}
	if(consume(STRUCT)){
		if(consume(ID)){
			return true;
			}
		}
	iTk=start; // refacere pozitie initiala
	return false;
}


bool arrayDecl(){
	printf("arrayDecl ");
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
	printf("varDef ");
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

bool fnDef(){
	return false;
}

bool fnParam(){
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

// bool stm(){

// }

// bool stmCompound(){
// 	printf("StmCompound");

// 	Token *start = iTk;

// 	if(consume(LACC)){
// 		for(;;){
// 			if(varDef()){}
// 			else if(stm()){}
// 			else break;
// 		}
// 	}
// 	if(consume(RACC)){
// 		return true;
// 	}
// 	iTk = start;
// 	return false;
// }

// bool exprAssign(){
// 	printf("Expr Assign\n");

// 	Token *start = iTk;

// 	if(exprUnary()){
// 		return true;
// 	}

// 	if(expr)

// 	iTk = start;
// 	return false;

// }

// bool exprEq(){
	
// }

// bool expr(){
// 	return exprAssign();
// }


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
	printf("Hi");
}
