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

bool expr(){
	return false;
}


bool stm(){
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
	iTk = start;
	return false;
}


bool stmCompound(){
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
	if(typeBase() | consume(VOID)){
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
