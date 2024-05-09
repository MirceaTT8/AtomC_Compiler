#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"
#include "ad.h"
#include "vm.h"
#include "lexer.h"
#include "utils.h"
#include "at.h"

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

bool expr(Ret *r){
	// printf("expr\n");
	Token *start = iTk;
	if(exprAssign(r)){
		// printf("exprAssign->expr\n");
		return true;
	}
	// printf("exprfalse\n");
	iTk = start;
	return false;
}

// exprAssign: exprUnary ASSIGN exprAssign | exprOr

bool exprAssign(Ret *r){
	// printf("exprAssign\n");
	Ret rDst;
	Token *start = iTk;
	if(exprUnary(&rDst)){
		// printf("exprUnary->exprAssign\n");
		if(consume(ASSIGN)){
			if(exprAssign(r)){
				if(!rDst.lval)tkerr("the assign destination must be a left-value");
                if(rDst.ct)tkerr("the assign destination cannot be constant");
                if(!canBeScalar(&rDst))tkerr("the assign destination must be scalar");
                if(!canBeScalar(r))tkerr("the assign source must be scalar");
                if(!convTo(&r->type,&rDst.type))tkerr("the assign source cannot be converted to destination");
                r->lval=false;
                r->ct=true;
				return true;
			}else tkerr("no expression after assign operator");
		}
		iTk=start;
	}
	if(exprOr(r)){
		// printf("exprOr->exprAssign\n");
		return true;
	}
	// printf("exprAssignfalse\n");
	iTk = start;
	return false;
}

//exprOr: exprAnd exprOrPrim
//exprOrPrim: OR exprAnd exprOrPrim | epsilon

bool exprOr(Ret *r){
	// printf("exprOr\n");
	Token* start = iTk;
	if(exprAnd(r)){
		// printf("exprAnd->exprOr\n");
		if(exprOrPrim(r)){
			return true;
		}
	}
	iTk = start;
	// printf("exprOrfalse\n");
	return false;

}
bool exprOrPrim(Ret *r) {
	Token *start = iTk;
    if (consume(OR)) { // Prima alternativă: OR exprAnd exprOrPrim
		Ret right;
        if (exprAnd(&right)) {
            Type tDst;
            if(!arithTypeTo(&r->type,&right.type,&tDst)) {
                char errorMsg[100]; // Assuming a maximum error message length of 100 characters
                sprintf(errorMsg, "invalid operand type for || at line %d", iTk->line);
                tkerr(errorMsg);
            }
            *r=(Ret){{TB_INT,NULL,-1},false,true};
            exprOrPrim(r);
            return true;
        }else tkerr("no expression after or operator");
    }
	iTk = start;
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprAnd: exprAnd AND exprEq | exprEq
// exprAnd: exprEq exprAndPrim
// exprAndPrime: AND exprEq exprAndPrim | ε

bool exprAnd(Ret *r) {
	// printf("exprAnd\n");
    Token* start = iTk;
	if(exprEq(r)){
		// printf("exprEq->exprAnd\n");
		if(exprAndPrim(r)){
			return true;
		}
	}
	iTk = start;
	// printf("exprAndfalse\n");
	return false;
}

bool exprAndPrim(Ret *r) {
    if (consume(AND)) { // Prima alternativă: OR exprAnd exprOrPrim
		Ret right;
        if (exprEq(&right)) {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst))
                tkerr("invalid operand type for &&");
            *r = (Ret) {{TB_INT, NULL, -1}, false, true};
            exprAndPrim(r);
            return true;
        }tkerr("No expression after and operator");
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel
// exprEq: exprRel exprEqPrim
// exprEqPrim: ( EQUAL | NOTEQ ) exprRel exprEqPrim | ε

bool exprEq(Ret *r) {
	// printf("exprEq\n");
    Token* start = iTk;
	if(exprRel(r)){
		// printf("exprRel->exprEq\n");
		if(exprEqPrim(r)){
			return true;
		}
	}
	// printf("exprEqfalse\n");
	iTk = start;
	return false;
}

bool exprEqPrim(Ret *r) {
    if (consume(EQUAL) || consume(NOTEQ)) { // Prima alternativă: OR exprAnd exprOrPrim
		Ret right;
        if (exprRel(&right)) {
            Type tDst;
            if(!arithTypeTo(&r->type,&right.type,&tDst))
                tkerr("invalid operand type for == or!=");
            *r=(Ret){{TB_INT,NULL,-1},false,true};
            exprEqPrim(r);
            return true;
        }else tkerr("No expression after equality operator");
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd
// exprRel: exprAdd exprRelPrim
// exprRelPrim: ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRelPrim | ε

bool exprRel(Ret *r) {
	// printf("exprRel\n");
    Token* start = iTk;
	if(exprAdd(r)){
		// printf("exprAdd->exprRel\n");
		if(exprRelPrim(r)){
			return true;
		}
	}
	// printf("exprRelfalse\n");
	iTk = start;
	return false;
}

bool exprRelPrim(Ret *r) {
    if (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)) { // Prima alternativă: OR exprAnd exprOrPrim
		Ret right;
        if (exprAdd(&right)) {
            Type tDst;
			if(!arithTypeTo(&r->type,&right.type,&tDst))tkerr("invalid operand type for <, <=, >,>=");
			*r=(Ret){{TB_INT,NULL,-1},false,true};
			exprRelPrim(r);
			return true;
        }tkerr("No expression after relation operator");
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul
// exprAdd: exprMul exprAddPrim
// exprAddPrim: ( ADD | SUB ) exprMul exprAddPrim | ε

bool exprAdd(Ret *r) {
	// printf("exprAdd\n");
    Token* start = iTk;
	if(exprMul(r)){
		// printf("exprMul->exprAdd\n");
		if(exprAddPrim(r)){
			return true;
		}
	}
	// printf("exprAddfalse\n");
	iTk = start;
	return false;
}

bool exprAddPrim(Ret *r) {
    if (consume(ADD) || consume(SUB)) { // Prima alternativă: OR exprAnd exprOrPrim
		Ret right;
        if (exprMul(&right)) {
            Type tDst;
			if(!arithTypeTo(&r->type,&right.type,&tDst))tkerr("invalid operand type for + or -");
			*r=(Ret){tDst,false,true};
			exprAddPrim(r);
			return true;
        }tkerr("No expression after adder operator");
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

// exprMul: exprMul ( MUL | DIV ) exprCast | exprCast
// exprMul: exprCast exprMulPrime
// exprMulPrime: ( MUL | DIV ) exprCast exprMulPrime | ε

bool exprMul(Ret *r) {
	// printf("exprMul\n");
    Token* start = iTk;
	if(exprCast(r)){
		// printf("exprCast->exprMul\n");	
		if(exprMulPrim(r)){
			return true;
		}
	}
	// printf("exprMulfalse\n");
	iTk = start;
	return false;
}

bool exprMulPrim(Ret *r) {
    if (consume(MUL) || consume(DIV)) { // Prima alternativă: OR exprAnd exprOrPrim
		Ret right;
        if (exprCast(&right)) {
            Type tDst;
			if(!arithTypeTo(&r->type,&right.type,&tDst))tkerr("invalid operand type for * or /");
			*r=(Ret){tDst,false,true};
			exprMulPrim(r);
			return true;
        }else tkerr("No expression after multiplier operator");
    }
    return true; // ε - exprOrPrim returnează true chiar dacă nu consumă nimic
}

bool exprCast(Ret *r){
	// printf("exprCast\n");
	Token* start = iTk;
	if(consume(LPAR)){
		Type t;
		Ret op;
		if(typeBase(&t)){
			if(arrayDecl(&t)) {}
            if(consume(RPAR)) {
                if(exprCast(&op)) {
                    if(t.tb==TB_STRUCT)tkerr("cannot convert to a struct type");
                    if(op.type.tb==TB_STRUCT)tkerr("cannot convert a struct");
                    if(op.type.n>=0&&t.n<0)tkerr("an array can be converted only to another array");
                    if(op.type.n<0&&t.n>=0)tkerr("a scalar can be converted only to another scalar");
                    *r=(Ret){t,false,true};
                    return true;
                }
            }else tkerr("Missing ) from cast expression");
        }
	}
	
	if (exprUnary(r)){
		// printf("exprUnary->exprCast\n");	
		return true;
	}
	// printf("exprCastfalse\n");
	iTk = start;
	return false;
}

bool exprUnary(Ret *r){
	// printf("exprUnary\n");
	Token* start = iTk;
	if(consume(SUB)|| consume(NOT)){
		if(exprUnary(r)){
			if(!canBeScalar(r))tkerr("unary - or ! must have a scalar operand");
			r->lval=false;
			r->ct=true;
			return true;
		}else tkerr("no expression after unary operator");
	}
	if(exprPostfix(r)){
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

bool exprPostfix(Ret *r) {
	// printf("exprPostfix\n");
    Token* start = iTk;
	if(exprPrimary(r)){
			// printf("exprPrimary->exprPostfix\n");
		if(exprPostfixPrim(r)){
			return true;
		}
	}
	// printf("exprPostfixfalse\n");
	iTk = start;
	return false;
}

bool exprPostfixPrim(Ret *r){
	Token* start = iTk;
	if(consume(LBRACKET)){
		Ret idx;
		if(expr(&idx)){
			if(consume(RBRACKET)){
				if(r->type.n<0)
                    tkerr("only an array can be indexed");
                Type tInt={TB_INT,NULL,-1};
                if(!convTo(&idx.type,&tInt))tkerr("the index is not convertible to int");
                r->type.n=-1;
                r->lval=true;
                r->ct=false;
                exprPostfixPrim(r);
                return true;
			}else tkerr("lipseste ]");
		}
		iTk=start;
	}
	if(consume(DOT)){
		if(consume(ID)){
			Token *tkName = consumedTk;
                if(r->type.tb!=TB_STRUCT)tkerr("a field can only be selected from a struct");
                Symbol *s=findSymbolInList(r->type.s->structMembers,tkName->text);
                if(!s)
                    tkerr("the structure %s does not have a field%s",r->type.s->name,tkName->text);
                *r=(Ret){s->type,true,s->type.n>=0};
                exprPostfixPrim(r);
                return true;
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

bool exprPrimary(Ret *r){
	// printf("exprPrimary\n");
	Token* start = iTk;
	if(consume(ID)){
		Token *tkName = consumedTk;
        Symbol *s=findSymbol(tkName->text);
        if(!s)tkerr("undefined id: %s",tkName->text);
		if(consume(LPAR)){
			if(s->kind!=SK_FN)tkerr("only a function can be called");
			Ret rArg;
			Symbol *param=s->fn.params;
			if(expr(&rArg)){
				if(!param)tkerr("too many arguments in function call");
                if(!convTo(&rArg.type,&param->type))
                    tkerr("in call, cannot convert the argument type to the parameter type");
                param=param->next;
				for(;;) {
                    if(consume(COMMA)) {
                        if(expr(&rArg)) {
                            if(!param)tkerr("too many arguments in function call");
                            if(!convTo(&rArg.type,&param->type))
                                tkerr("in call, cannot convert the argument type to the parameter type");
                            param=param->next;
                        }else tkerr("Missing expression after ,");
                    }else break;
                }
			}else tkerr("Missing expression after (");
			if(consume(RPAR)){
				if(param)tkerr("too few arguments in function call");
                *r=(Ret){s->type,false,true};
                return true;
			}else tkerr("Missing )");
		}
		if(s->kind==SK_FN)tkerr("a function can only be called");
        *r=(Ret){s->type,true,s->type.n>=0};
		return true;
	}
	else if(consume(DOUBLE)){
		 *r=(Ret){{TB_DOUBLE,NULL,-1},false,true};
		return true;
	}
	else if(consume(CHAR)){
		*r=(Ret){{TB_CHAR,NULL,-1},false,true};
		return true;
	}
	else if(consume(STRING)){
		*r=(Ret){{TB_CHAR,NULL,0},false,true};
		return true;
	}
	else if(consume(INT)){
		*r=(Ret){{TB_INT,NULL,-1},false,true};
		return true;
	}
	else if(consume(LPAR)){
		if(expr(r)){
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
	Ret rCond,rExpr;
	if(stmCompound(true)){
		// printf("stmCompound->stm\n ");
		return true;
	}
	if(consume(IF)){
		if(consume(LPAR)){
			if(expr(&rCond)){
				if(!canBeScalar(&rCond))
                    tkerr("the if condition must be a scalar value");
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
	}
	else if(consume(WHILE)){
		if(consume(LPAR)){
			if(expr(&rCond)){
				if(!canBeScalar(&rCond))
                    tkerr("the while condition must be a scalar value");
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
			if(expr(&rExpr)) {
				if(owner->type.tb==TB_VOID)
					tkerr("a void function cannot return a value");
				if(!canBeScalar(&rExpr))
					tkerr("the return value must be a scalar value");
				if(!convTo(&rExpr.type,&owner->type))
					tkerr("cannot convert the return expression type to the function return type");
			} else {
				if(owner->type.tb!=TB_VOID)
					tkerr("a non-void function must return a value");
				}
			if(consume(SEMICOLON)){
				return true;
			}else tkerr("missing ; at return statement");
	}
	else if(expr(&rExpr)){
		if(consume(SEMICOLON)){
			return true;
		}else tkerr("Missing ; at the end of expression");
	}
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