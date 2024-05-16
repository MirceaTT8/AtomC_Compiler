
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "ad.h"
#include "vm.h"

Instr *addInstr(Instr **list,Opcode op){
	Instr *i=(Instr*)safeAlloc(sizeof(Instr));
	i->op=op;
	i->next=NULL;
	if(*list){
		Instr *p=*list;
		while(p->next)p=p->next;
		p->next=i;
		}else{
		*list=i;
		}
	return i;
	}

Instr *addInstrWithInt(Instr **list,Opcode op,int argVal){
	Instr *i=addInstr(list,op);
	i->arg.i=argVal;
	return i;
	}

Instr *addInstrWithDouble(Instr **list,Opcode op,double argVal){
	Instr *i=addInstr(list,op);
	i->arg.f=argVal;
	return i;
	}

// added these functions
// inserts a new instruction after the specified instruction and sets its "op" field
// returns the newly added instruction
Instr *insertInstr(Instr *before, int op) {
    Instr *i = (Instr *) safeAlloc(sizeof(Instr));
    i->op = op;
    i->next = before->next;
    before->next = i;
    return i;
}

// deletes all the instructions after the given one
void delInstrAfter(Instr *instr) {
    if (instr) {
        for (Instr *next = instr->next, *i = next; i; i = next) {
            next = i->next;
            free(i);
        }
        instr->next = NULL;
    }
}

// returns the last instruction from list
Instr *lastInstr(Instr *list) {
    if (list) {
        while (list->next) list = list->next;
    }
    return list;
}

Val stack[10000];		// the stack
Val *SP=stack-1;		// Stack pointer - the stack's top - points to the value from the top of the stack
Val *FP=NULL;		// the initial value doesn't matter

void pushv(Val v){
	if(SP+1==stack+10000)err("trying to push into a full stack");
	*++SP=v;
	}

Val popv(){
	if(SP==stack-1)err("trying to pop from empty stack");
	return *SP--;
	}

void pushi(int i){
	if(SP+1==stack+10000)err("trying to push into a full stack");
	(++SP)->i=i;
	}

int popi(){
	if(SP==stack-1)err("trying to pop from empty stack");
	return SP--->i;
	}

void pushp(void *p){
	if(SP+1==stack+10000)err("trying to push into a full stack");
	(++SP)->p=p;
	}

// added pop function for float
double popf() {
    if (SP == stack - 1) err("trying to pop from empty stack");
    return SP--->f;
}

// added push function for float
void pushf(double f) {
    if (SP + 1 == stack + 1000) err("trying to push into a full stack");
    (++SP)->f = f;
}

void *popp(){
	if(SP==stack-1)err("trying to pop from empty stack");
	return SP--->p;
	}

void put_i(){
	printf("=> %d",popi());
	}

void put_d(){ 
	printf("=> %f\n", popf()); 
	}

void vmInit(){
	Symbol *fn=addExtFn("put_i",put_i,(Type){TB_VOID,NULL,-1});
	addFnParam(fn,"i",(Type){TB_INT,NULL,-1});

	fn = addExtFn("put_d", put_d, (Type){TB_VOID, NULL, -1});
    addFnParam(fn, "d", (Type){TB_DOUBLE, NULL, -1});
	}

void run(Instr *IP){
	Val v;
	int iArg,iTop,iBefore;
	 double iArg_f, iTop_f, iBefore_f;
    double fTop;
    void *pTop;

	void(*extFnPtr)();
	for(;;){
		// shows the index of the current instruction and the number of values from stack
		printf("%p/%d\t",IP,(int)(SP-stack+1));
		switch(IP->op){
			case OP_HALT:
				printf("HALT\n");
				return;
			case OP_PUSH_I:
				printf("PUSH.i\t%d",IP->arg.i);
				pushi(IP->arg.i);
				IP=IP->next;
				break;
			case OP_CALL:
				pushp(IP->next);
				printf("CALL\t%p",IP->arg.instr);
				IP=IP->arg.instr;
				break;
			case OP_CALL_EXT:
				extFnPtr=IP->arg.extFnPtr;
				printf("CALL_EXT\t%p\n",extFnPtr);
				extFnPtr();
				IP=IP->next;
				break;
			case OP_ENTER:
				pushp(FP);
				FP=SP;
				SP+=IP->arg.i;
				printf("ENTER\t%d",IP->arg.i);
				IP=IP->next;
				break;
			case OP_RET_VOID:
				iArg=IP->arg.i;
				printf("RET_VOID\t%d",iArg);
				IP=FP[-1].p;
				SP=FP-iArg-2;
				FP=FP[0].p;
				break;
			case OP_JMP:
				printf("JMP\t%p",IP->arg.instr);
				IP=IP->arg.instr;
				break;
			case OP_JF:
				iTop=popi();
				printf("JF\t%p\t// %d",IP->arg.instr,iTop);
				IP=iTop ? IP->next : IP->arg.instr;
				break;
			case OP_FPLOAD:
				v=FP[IP->arg.i];
				pushv(v);
				printf("FPLOAD\t%d\t// i:%d, f:%g",IP->arg.i,v.i,v.f);
				IP=IP->next;
				break;
			case OP_FPSTORE:
				v=popv();
				FP[IP->arg.i]=v;
				printf("FPSTORE\t%d\t// i:%d, f:%g",IP->arg.i,v.i,v.f);
				IP=IP->next;
				break;
			case OP_ADD_I:
				iTop=popi();
				iBefore=popi();
				pushi(iBefore+iTop);
				printf("ADD.i\t// %d+%d -> %d",iBefore,iTop,iBefore+iTop);
				IP=IP->next;
				break;
			case OP_LESS_I:
				iTop=popi();
				iBefore=popi();
				pushi(iBefore<iTop);
				printf("LESS.i\t// %d<%d -> %d",iBefore,iTop,iBefore<iTop);
				IP=IP->next;
				break;
			//added for code generation
            case OP_CONV_F_I:
                fTop = popf();
                pushi((int) fTop);
                printf("CONV.f.i\t// %g -> %d", fTop, (int) fTop);
                IP = IP->next;
                break;
            case OP_DROP:
                popv();
                printf("DROP");
                IP = IP->next;
                break;
            case OP_PUSH_F:
                printf("PUSH.f\t%g", IP->arg.f);
                pushf(IP->arg.f);
                IP = IP->next;
                break;
            case OP_FPADDR_I:
                pTop = &FP[IP->arg.i].i;
                pushp(pTop);
                printf("FPADDR\t%d\t// %p", IP->arg.i, pTop);
                IP = IP->next;
                break;
            case OP_LOAD_I:
                pTop = popp();
                pushi(*(int *) pTop);
                printf("LOAD.i\t// *(int*)%p -> %d", pTop, *(int *) pTop);
                IP = IP->next;
                break;
            case OP_NOP:
                printf("NOP");
                IP = IP->next;
                break;
            case OP_RET:
                v = popv();
                iArg = IP->arg.i;
                printf("RET\t%d\t// i:%d, f:%g", iArg, v.i, v.f);
                IP = FP[-1].p;
                SP = FP - iArg - 2;
                FP = FP[0].p;
                pushv(v);
                break;
            case OP_SUB_I:
                iTop = popi();
                iBefore = popi();
                pushi(iBefore - iTop);
                printf("SUB.i\t// %d-%d -> %d", iBefore, iTop, iBefore - iTop);
                IP = IP->next;
                break;
            case OP_MUL_I:
                iTop = popi();
                iBefore = popi();
                pushi(iBefore * iTop);
                printf("MUL.i\t// %d*%d -> %d", iBefore, iTop, iBefore * iTop);
                IP = IP->next;
                break;
            case OP_STORE_I:
                iTop = popi();
                v = popv();
                *(int *) v.p = iTop;
                pushi(iTop);
                printf("STORE.i\t// *(int*)%p=%d", v.p, iTop);
                IP = IP->next;
                break;
            case OP_LESS_F:
                iTop_f = popf();
                iBefore_f = popf();
                pushi(iBefore_f < iTop_f);
                printf("LESS.f\t// %f<%f -> %d", iBefore_f, iTop_f, iBefore_f < iTop_f);
                IP = IP->next;
                break;
            case OP_ADD_F:
                iTop_f = popf();
                iBefore_f = popf();
                pushf(iBefore_f + iTop_f);
                printf("ADD.f\t// %f+%f -> %f", iBefore_f, iTop_f, iBefore_f + iTop_f);
                IP = IP->next;
                break;
			default:err("run: instructiune neimplementata: %d",IP->op);
			}
		putchar('\n');
		}
	}

/* The program implements the following AtomC source code:
f(2);
void f(int n){		// stack frame: n[-2] ret[-1] oldFP[0] i[1]
	int i=0;
	while(i<n){
		put_i(i);
		i=i+1;
		}
	}
*/
Instr *genTestProgram(){
	Instr *code=NULL;
	addInstrWithInt(&code,OP_PUSH_I,2);
	Instr *callPos=addInstr(&code,OP_CALL);
	addInstr(&code,OP_HALT);
	callPos->arg.instr=addInstrWithInt(&code,OP_ENTER,1);
	// int i=0;
	addInstrWithInt(&code,OP_PUSH_I,0);
	addInstrWithInt(&code,OP_FPSTORE,1);
	// while(i<n){
	Instr *whilePos=addInstrWithInt(&code,OP_FPLOAD,1);
	addInstrWithInt(&code,OP_FPLOAD,-2);
	addInstr(&code,OP_LESS_I);
	Instr *jfAfter=addInstr(&code,OP_JF);
	// put_i(i);
	addInstrWithInt(&code,OP_FPLOAD,1);
	Symbol *s=findSymbol("put_i");
	if(!s)err("undefined: put_i");
	addInstr(&code,OP_CALL_EXT)->arg.extFnPtr=s->fn.extFnPtr;
	// i=i+1;
	addInstrWithInt(&code,OP_FPLOAD,1);
	addInstrWithInt(&code,OP_PUSH_I,1);
	addInstr(&code,OP_ADD_I);
	addInstrWithInt(&code,OP_FPSTORE,1);
	// } ( the next iteration)
	addInstr(&code,OP_JMP)->arg.instr=whilePos;
	// returns from function
	jfAfter->arg.instr=addInstrWithInt(&code,OP_RET_VOID,1);
	return code;
	}

/*f(2.0);
void f(double n){
    double i=0.0;
    while(i<n){
        put_d(i);
        i=i+0.5;
    }
}*/
Instr *genTestProgramHomeWork() {
    Instr *code = NULL;
    addInstrWithDouble(&code, OP_PUSH_F, 2.0);
    Instr *callPos = addInstr(&code, OP_CALL);
    addInstr(&code, OP_HALT);
    callPos->arg.instr = addInstrWithInt(&code, OP_ENTER, 1);

    // double i = 0.0;
    addInstrWithDouble(&code, OP_PUSH_F, 0.0);
    addInstrWithInt(&code, OP_FPSTORE, 1);

    // while (i < n)
    Instr *whilePos = addInstrWithInt(&code, OP_FPLOAD, 1);// extrag elem. de pe poz 1 din fp (adica i)
    addInstrWithInt(&code, OP_FPLOAD, -2);                 // extrag elem. de pe poz -2 din fp (adica n)
    addInstr(&code, OP_LESS_F);                            // compara cele 2 elem. din varful stivei
    Instr *jfAfter = addInstr(&code, OP_JF);

    addInstrWithInt(&code, OP_FPLOAD, 1);
    Symbol *s = findSymbol("put_d");
    if (!s) {
        err("undefined: put_d");
    }
    addInstr(&code, OP_CALL_EXT)->arg.extFnPtr = s->fn.extFnPtr;

    addInstrWithInt(&code, OP_FPLOAD, 1);
    addInstrWithDouble(&code, OP_PUSH_F, 0.5);
    addInstr(&code, OP_ADD_F);
    addInstrWithInt(&code, OP_FPSTORE, 1);

    addInstr(&code, OP_JMP)->arg.instr = whilePos;
    jfAfter->arg.instr = addInstrWithInt(&code, OP_RET_VOID, 1);
    return code;
}
