#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "utils.h"

Token *tokens;	// single linked list of tokens
Token *lastTk;		// the last token in list

int line=1;		// the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token *addTk(int code){
	Token *tk=safeAlloc(sizeof(Token));
	tk->code=code;
	tk->line=line;
	tk->next=NULL;
	if(lastTk){
		lastTk->next=tk;
		}else{
		tokens=tk;
		}
	lastTk=tk;
	return tk;
	}

char *extract(const char *begin,const char *end){
    ptrdiff_t length = end - begin;

    char* result = safeAlloc(length + 1);

    memcpy(result, begin, length);

    result[length] = '\0';

    return result;
}

const char* getTokenNameByPosition(int position) {
    switch (position) {
        case ID: return "ID";
        case TYPE_CHAR: return "TYPE_CHAR";
        case TYPE_DOUBLE: return "TYPE_DOUBLE";
        case TYPE_INT: return "TYPE_INT";
        case ELSE: return "ELSE";
        case IF: return "IF";
        case RETURN: return "RETURN";
        case STRUCT: return "STRUCT";
        case VOID: return "VOID";
        case WHILE: return "WHILE";
        case COMMA: return "COMMA";
        case END: return "END";
        case SEMICOLON: return "SEMICOLON";
        case LPAR: return "LPAR";
        case RPAR: return "RPAR";
        case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";
        case RACC: return "RACC";
        case LACC: return "LACC";
        case ASSIGN: return "ASSIGN";
        case EQUAL: return "EQUAL";
        case NOTEQ: return "NOTEQ";
        case LESS: return "LESS";
        case LESSEQ: return "LESSEQ";
        case GREATER: return "GREATER";
        case GREATEREQ: return "GREATEREQ";
        case ADD: return "ADD";
        case SUB: return "SUB";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case DOT: return "DOT";
        case AND: return "AND";
        case OR: return "OR";
        case NOT: return "NOT";
		case INT: return "INT";
		case DOUBLE: return "DOUBLE";
		case CHAR: return "CHAR";
		case STRING: return "STRING";
        default: return "UNKNOWN";
    }
}

Token *tokenize(const char *pch){
	const char *start;
	Token *tk;
	for(;;){
		switch(*pch){
			case ' ':case '\t':pch++;break;
			case '\r':		// handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
				if(pch[1]=='\n')pch++;
				// fallthrough to \n
			case '\n':
				line++; //urm linie
				pch++; //trecem la urm caracter
				break;
			case '\0':addTk(END);return tokens;
			case ',':addTk(COMMA);pch++;break;
			case '.':addTk(DOT);pch++;break;
			case ';':addTk(SEMICOLON);pch++;break;
			case '(':addTk(LPAR);pch++;break;
			case ')':addTk(RPAR);pch++;break;
			case '[':addTk(LBRACKET);pch++;break;
			case ']':addTk(RBRACKET);pch++;break;
			case '{':addTk(LACC);pch++;break;
			case '}':addTk(RACC);pch++;break;
			case '=':
				if(pch[1]=='='){
					addTk(EQUAL);
					pch+=2;
					}else{
					addTk(ASSIGN);
					pch++;
					}
				break;
			case '<':
				if(pch[1]=='='){
				addTk(LESSEQ);
				pch+=2;
				}
				else{
					addTk(LESS);
					pch++;
				}
				break;
			case '>':
				if(pch[1]=='='){
				addTk(GREATEREQ);pch+=2;
				}
				else{
					addTk(GREATER);
					pch++;
				}
				break;
			case '&':
				if(pch[1]=='&'){
					addTk(AND);
					pch+=2;
				}
				else{
					pch++;
				}
				break;
			case '/':
				if(pch[1]=='/'){
					while(*pch != '\0' && *pch != '\n'){
						pch++;
					}
				}
				else{
					addTk(DIV);
					pch++;
				}
				break;
			case '+':addTk(ADD);pch++;break;
			case '-':addTk(SUB);pch++;break;
			case '*':addTk(MUL);pch++;break;
			case '|':
				if(pch[1]=='|'){
					addTk(OR);
					pch+=2;
				}
				else{
					pch++;
				}
				break;
			case '!':
				if(pch[1]=='='){
					addTk(NOTEQ);
					pch+=2;
					}
					else{
						addTk(NOT);
						pch++;
					}
				break;
			default: //INT poate sa inceapa cu digiti etc de astea, nu au nevoie de case uri separate
				if(isalpha(*pch)||*pch=='_'){
					for(start=pch++;isalnum(*pch)||*pch=='_';pch++){}
					char *text=extract(start,pch); //extrage subsiru dintre cei 2 pointeri [start,pch)
					if(strcmp(text,"char")==0)
						addTk(TYPE_CHAR);
					else if(strcmp(text,"double")==0)
						addTk(TYPE_DOUBLE);
					else if(strcmp(text,"int")==0)
						addTk(TYPE_INT);	
					else if(strcmp(text,"while")==0)
						addTk(WHILE);	
					else if(strcmp(text,"if")==0)
						addTk(IF);
					else if(strcmp(text,"else")==0)
						addTk(ELSE);
					else if(strcmp(text,"struct")==0)
						addTk(STRUCT);
					else if(strcmp(text,"void")==0)
						addTk(VOID);
					else if(strcmp(text,"return")==0)
						addTk(RETURN);	
					else{
						tk=addTk(ID);
						tk->text=text;
						}
					}
				else if(isdigit(*pch)){
					for(start=pch++;isdigit(*pch);pch++){}
						if(*pch =='.'){
							pch++;
							while(isdigit(*pch)){
								pch++;
							}
						}
						if(*pch =='e' || *pch=='E'){
							pch++;
							if(*pch == '+' || *pch == '-'){
								pch++;
							}
							while(isdigit(*pch)){
								pch++;
							}
						} 
					char *text=extract(start,pch);
					if (strchr(text,'.')|| strchr(text,'e') || strchr(text,'E')){
						tk=addTk(DOUBLE);
						char* end;
						tk->d=strtod(text,&end);
					}
					else{
						tk=addTk(INT);
						tk->i=atoi(text);
					}
				}
				else if(*pch == '\'' || *pch == '\"'){
					char quoteType = *pch; 
					pch++; 
					start = pch; 

					while(*pch && *pch != quoteType) {
						pch++;
					}
				
					if(quoteType == '\'' && (pch - start) == 1) {
						char character = *start; 
						tk = addTk(CHAR);
						tk->c = character;
					}
					
					else if(quoteType == '\"') {
						char* text = extract(start, pch); 
						tk = addTk(STRING);
						tk->text = text;
					}

					if(*pch == quoteType) {
						pch++; 
					}
				}
				else {
						err("invalid char: %c (%d)",*pch,*pch);
					}
				// else{
				// 	pch++;
				// }
			}
		}
	}

void showTokens(const Token *tokens){
	for(const Token *tk=tokens;tk;tk=tk->next){
		const char* token = getTokenNameByPosition(tk->code);
		printf("%d %s",tk->line,token); //tre sa arate linia, numarul atomului, si daca e cazul, din ce e constituit
		
		if(!strcmp(token,"INT")){
			printf(":%d\n",tk->i);
		}
		else if(!strcmp(token,"CHAR")){
			printf(":%c\n",tk->c);
		}
		else if(!strcmp(token,"DOUBLE")){
			printf(":%g\n",tk->d);
		}
		else if(!strcmp(token,"STRING")||!strcmp(token,"ID")){
			printf(":%s\n",tk->text);
		}
		else{
			printf("\n");
		}

		}
	}