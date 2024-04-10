#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"
#include "ad.h"
#include "vm.h"
#include "lexer.h"
#include "utils.h"

bool typeBase(Type *t);
bool arrayDecl(Type *t);

bool stmCompound(bool newDomain);
Token *iTk;		// the iterator in the tokens list
Token *consumedTk;		// the last consumed token

Symbol *owner = NULL;


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
	// printf("consume(%s)",getTokenNameByPosition(code));
	if(iTk->code==code){
		consumedTk=iTk;
		iTk=iTk->next;
		// printf(" => consumed\n");
		return true;
	}
		// printf(" => found %s\n",getTokenNameByPosition(iTk->code));
		return false;
}

bool typeBase(Type *t) {
	// printf("#typeBase %d\n",iTk->code);//pentru debugging !!!
	t->n = -1;
	Token *start = iTk;
	if (consume(TYPE_INT)) {
		t->tb=TB_INT;
		return true;
	}
	if (consume(TYPE_DOUBLE)) {
		t->tb=TB_DOUBLE;
		return true;
	}
	if (consume(TYPE_CHAR)) {
		t->tb=TB_CHAR;
		return true;
	}
	if (consume(STRUCT)) {
		if (consume(ID)) {
			Token *tkName = consumedTk;
			t->tb = TB_STRUCT;
			t->s = findSymbol(tkName->text);
			if(!t->s) tkerr("structura nedefinita: %s",tkName->text);
			return true;
		} else {
			tkerr("Lipseste numele structurii");
		}
	}
	iTk = start;
	return false;
}


bool arrayDecl(Type *t){
	if(consume(LBRACKET)){
		if(consume(INT)){
			Token *tkSize=consumedTk;
			t->n=tkSize->i;
		} else {
			t->n=0; // array fara dimensiune: int v[]
		}
		if(consume(RBRACKET)){
			return true;
		}else tkerr("missing ] or invalid expression inside [...]");
	}
return false;
}

bool varDef(){
	// printf("varDef\n ");
	Token* start = iTk;
	Type t;
	if(typeBase(&t)){
		// printf("typeBase->varDef\n ");
		if(consume(ID)){
			Token *tkName = consumedTk;
			if (arrayDecl(&t)) {
				if (t.n == 0) tkerr("a vector variable must have a specified dimension");
			}
			if(consume(SEMICOLON)){
				Symbol *var=findSymbolInDomain(symTable,tkName->text);
				if(var)tkerr("symbol redefinition: %s",tkName->text);
				var=newSymbol(tkName->text,SK_VAR);
				var->type=t;
				var->owner=owner;
				addSymbolToDomain(symTable,var);
				if (owner) {
					switch(owner->kind) {
						case SK_FN:
							var->varIdx=symbolsLen(owner->fn.locals);
							addSymbolToList(&owner->fn.locals,dupSymbol(var));
							break;
						case SK_STRUCT:
							var->varIdx=typeSize(&owner->type);
							addSymbolToList(&owner->structMembers,dupSymbol(var));
							break;
						default:
							break;
					}
				} else {
					var->varMem=safeAlloc(typeSize(&t));
				}
				return true;
			}else tkerr("; missing when declaring variable");
		}else tkerr("id missing when declaring variable");
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
	// printf("expr\n");
	Token *start = iTk;
	if(exprAssign()){
		// printf("exprAssign->expr\n");
		return true;
	}
	// printf("exprfalse\n");
	iTk = start;
	return false;
}

// exprAssign: exprUnary ASSIGN exprAssign | exprOr

bool exprAssign(){
	// printf("exprAssign\n");
	Token *start = iTk;
	if(exprUnary()){
		// printf("exprUnary->exprAssign\n");
		if(consume(ASSIGN)){
			if(exprAssign()){
				return true;
			}else tkerr("no expression after assign operator");
		}
		iTk=start;
	}
	if(exprOr()){
		// printf("exprOr->exprAssign\n");
		return true;
	}
	// printf("exprAssignfalse\n");
	iTk = start;
	return false;
}

//exprOr: exprAnd exprOrPrim
//exprOrPrim: OR exprAnd exprOrPrim | epsilon

bool exprOr(){
	// printf("exprOr\n");
	Token* start = iTk;
	if(exprAnd()){
		// printf("exprAnd->exprOr\n");
		if(exprOrPrim()){
			return true;
		}
	}
	iTk = start;
	// printf("exprOrfalse\n");
	return false;

}
bool exprOrPrim() {
    if (consume(OR)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprAnd()) {
            if (exprOrPrim()) {
                return true;
            }
        }else tkerr("no expression after or operator");
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprAnd: exprAnd AND exprEq | exprEq
// exprAnd: exprEq exprAndPrim
// exprAndPrime: AND exprEq exprAndPrim | ε

bool exprAnd() {
	// printf("exprAnd\n");
    Token* start = iTk;
	if(exprEq()){
		// printf("exprEq->exprAnd\n");
		if(exprAndPrim()){
			return true;
		}
	}
	iTk = start;
	// printf("exprAndfalse\n");
	return false;
}

bool exprAndPrim() {
    if (consume(AND)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprEq()) {
            if (exprAndPrim()) {
                return true;
            }
        }tkerr("No expression after and operator");
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel
// exprEq: exprRel exprEqPrim
// exprEqPrim: ( EQUAL | NOTEQ ) exprRel exprEqPrim | ε

bool exprEq() {
	// printf("exprEq\n");
    Token* start = iTk;
	if(exprRel()){
		// printf("exprRel->exprEq\n");
		if(exprEqPrim()){
			return true;
		}
	}
	// printf("exprEqfalse\n");
	iTk = start;
	return false;
}

bool exprEqPrim() {
    if (consume(EQUAL) || consume(NOTEQ)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprRel()) {
            if (exprEqPrim()) {
                return true;
            }
        }else tkerr("No expression after equality operator");
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd
// exprRel: exprAdd exprRelPrim
// exprRelPrim: ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRelPrim | ε

bool exprRel() {
	// printf("exprRel\n");
    Token* start = iTk;
	if(exprAdd()){
		// printf("exprAdd->exprRel\n");
		if(exprRelPrim()){
			return true;
		}
	}
	// printf("exprRelfalse\n");
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
	// printf("exprAdd\n");
    Token* start = iTk;
	if(exprMul()){
		// printf("exprMul->exprAdd\n");
		if(exprAddPrim()){
			return true;
		}
	}
	// printf("exprAddfalse\n");
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
	// printf("exprMul\n");
    Token* start = iTk;
	if(exprCast()){
		// printf("exprCast->exprMul\n");	
		if(exprMulPrim()){
			return true;
		}
	}
	// printf("exprMulfalse\n");
	iTk = start;
	return false;
}

bool exprMulPrim() {
    if (consume(MUL) || consume(DIV)) { // Prima alternativă: OR exprAnd exprOrPrim
        if (exprCast()) {
            if (exprMulPrim()) {
                return true;
            }
        }else tkerr("No expression after multiplier operator");
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

bool exprCast(){
	// printf("exprCast\n");
	Token* start = iTk;
	Type t;
	if(consume(LPAR)){
		if(typeBase(&t)){
			if(arrayDecl(&t)){}
			if(consume(RPAR)){
				if(exprCast()){
					return true;
				}
			}else tkerr(") missing");
		}
	}
	if(exprUnary()){
		// printf("exprUnary->exprCast\n");	
		return true;
	}
	// printf("exprCastfalse\n");
	iTk = start;
	return false;
}

bool exprUnary(){
	// printf("exprUnary\n");
	Token* start = iTk;
	if(consume(SUB)|| consume(NOT)){
		if(exprUnary()){
			return true;
		}else tkerr("no expression after unary operator");
	}
	if(exprPostfix()){
		// printf("exprPostfix->exprUnary\n");
		return true;
	}
	// printf("exprUnaryfalse\n");
	iTk = start;
	return false;
}

// exprPostfix: exprPostfix LBRACKET expr RBRACKET
//            | exprPostfix DOT ID
//            | exprPrimary
// exprPostfix: exprPrimary exprPostfixPrime
// exprPostfixPrime: ( LBRACKET expr RBRACKET | DOT ID ) exprPostfixPrime | ε

bool exprPostfix() {
	// printf("exprPostfix\n");
    Token* start = iTk;
	if(exprPrimary()){
			// printf("exprPrimary->exprPostfix\n");

		if(exprPostfixPrim()){
			return true;
		}
	}
	// printf("exprPostfixfalse\n");
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
		}else tkerr("missing ID after .");
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
	// printf("exprPrimary\n");
	Token* start = iTk;
	if(consume(ID)){
		if(consume(LPAR)){
			if(expr()){
				// printf("expr->exprPrimary");s
				while(consume(COMMA)){
					if(expr()){}
					else tkerr("Missing expression after ,");
				}
			}else tkerr("Missing expression after (");
			if(consume(RPAR)){
				return true;
			}else tkerr("Missing )");
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
			}else tkerr("Missing )");
		}else tkerr("Missing expression after (");
	}
	// printf("exprPrimaryfalse\n");
	iTk = start;
	return false;
}

bool stm(){
	// printf("Stm\n");
	Token* start= iTk;
	if(stmCompound(true)){
		// printf("stmCompound->stm\n ");
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
				}else tkerr("Missing ) for if");
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
					}else tkerr("Missing while body");
				}else tkerr("Missing ) for while");
			}else tkerr("Missing while condition");
		}else tkerr("Missing ( after while keyword");
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

bool stmCompound(bool newDomain){
	// printf("StmCompound\n");
	Token *start = iTk;
	if (consume(LACC)) {
		if(newDomain)pushDomain();
		for (;;) {
			if (varDef() || stm()) {}
			else break;
		}
		if (consume(RACC)) {
			if(newDomain)dropDomain();
			return true;
		} else {
			tkerr("Lipseste '}'");
		}
	}
	iTk = start;
	return false;
}


bool fnParam(){
	// printf("fnParam\n");
	Token *start = iTk;
	Type t;
	if (typeBase(&t)) {
		if (consume(ID)) {
			Token *tkName = consumedTk;
			if (arrayDecl(&t)) {
				t.n=0;
			}
			Symbol *param=findSymbolInDomain(symTable,tkName->text);
			if(param)tkerr("symbol redefinition: %s",tkName->text);
			param=newSymbol(tkName->text,SK_PARAM);
			param->type=t;
			param->owner=owner;
			param->paramIdx=symbolsLen(owner->fn.params);
			// parametrul este adaugat atat la domeniul curent, cat si la parametrii fn
			addSymbolToDomain(symTable,param);
			addSymbolToList(&owner->fn.params,dupSymbol(param));
			return true;
		} else {
			tkerr("Lipseste numele parametrului");
		}
	}
	iTk = start;
	return false;
}

bool fnDef(){
	// printf("fnDef\n");
	Type t;
	Token *start = iTk;
	if (typeBase(&t)) {
		if (consume(ID)) {
			Token *tkName = consumedTk;
			if (consume(LPAR)) {
				Symbol *fn=findSymbolInDomain(symTable,tkName->text);
				if(fn)tkerr("symbol redefinition: %s",tkName->text);
				fn=newSymbol(tkName->text,SK_FN);
				fn->type=t;
				addSymbolToDomain(symTable,fn);
				owner=fn;
				pushDomain();
				if (fnParam()) {
					for (;;) {
						if (consume(COMMA)) {
							if (fnParam()) {}
							else tkerr("Lipseste parametrul functiei dupa ,");
						} else break;
					}
				}
				if (consume(RPAR)) {
					if (stmCompound(false)) {
						dropDomain();
						owner=NULL;
						return true;
					} else {
						tkerr("Lipseste corpul functiei");
					}
				} else {
					tkerr("Lipseste ')' la finalul functiei");
				}
			}
		} else {
			tkerr("Lipseste numele functiei");
		}
	} else if (consume(VOID)) {
		t.tb=TB_VOID;
		if (consume(ID)) {
			Token *tkName = consumedTk;
			if (consume(LPAR)) {
				Symbol *fn=findSymbolInDomain(symTable,tkName->text);
				if(fn)tkerr("symbol redefinition: %s",tkName->text);
				fn=newSymbol(tkName->text,SK_FN);
				fn->type=t;
				addSymbolToDomain(symTable,fn);
				owner=fn;
				pushDomain();
				if (fnParam()) {
					for (;;) {
						if (consume(COMMA)) {
							if (fnParam()) {}
							else tkerr("Lipseste parametrul functiei dupa ,");
						} else break;
					}
				}
				if (consume(RPAR)) {
					if (stmCompound(false)) {
						dropDomain();
						owner=NULL;
						return true;
					} else {
						tkerr("Lipseste corpul functiei");
					}
				} else {
					tkerr("Lipseste ')' la finalul functiei");
				}
			}
		} else {
			tkerr("Lipseste numele functiei");
		}
	}
	iTk = start;
	return false;
}

bool structDef(){
	// printf("structDef\n");
	Token* start = iTk;
	if(consume(STRUCT)){
		if(consume(ID)){
			Token *tkName = consumedTk;
			if(consume(LACC)){
				Symbol *s = findSymbolInDomain(symTable, tkName->text);
				if(s) tkerr("symbol redefinition: %s", tkName->text);
				s = addSymbolToDomain(symTable,newSymbol(tkName->text, SK_STRUCT));
				s->type.tb = TB_STRUCT;
				s->type.s = s;
				s->type.n = -1;
				pushDomain();
				owner = s;
				while(varDef()){
					// printf("varDef->structDef\n");
				}
				if(consume(RACC)){
					if(consume(SEMICOLON))
					{
						owner = NULL;
						dropDomain();
						return true;
					}else tkerr("; expected at struct defining");
				}else tkerr("Missing } when defining struct");
			}
		}else tkerr("Id of the struct missing");
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
	// printf("HiParse");
}