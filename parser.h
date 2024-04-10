#pragma once

#include "lexer.h"
#include <stdbool.h>

void parse(Token *tokens);
bool stm(); // Prototipul funcției stm
bool stmCompound(bool newDomain); // Prototipul funcției stmCompound
bool expr();
bool exprAssign();
bool exprOr();
bool exprOrPrim();
bool exprAnd();
bool exprAndPrim();
bool exprEq();
bool exprEqPrim();
bool exprRel();
bool exprRelPrim();
bool exprAdd();
bool exprAddPrim();
bool exprMul();
bool exprMulPrim();
bool exprCast();
bool exprUnary();
bool exprPostfix();
bool exprPostfixPrim();
bool exprPrimary();

