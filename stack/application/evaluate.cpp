//This code is buggy!

#include "../../def.hpp"  

#define N_OPTR 9
typedef enum {ADD, SUB, MUL, DIV, POW, FAC, L_P, R_P, EOE} Operator;

const char pri[N_OPTR][N_OPTR] = { //运算符优先等级 [栈顶] [当前]
    /*              |-------------------- 当 前 运 算 符 --------------------| */
    /*              +      -      *      /      ^      !      (      )      \0 */
    /* --  + */    '>',   '>',   '<',   '<',   '<',   '<',   '<',   '>',   '>',
    /* |   - */    '>',   '>',   '<',   '<',   '<',   '<',   '<',   '>',   '>',
    /* 栈  * */    '>',   '>',   '>',   '>',   '<',   '<',   '<',   '>',   '>',
    /* 顶  / */    '>',   '>',   '>',   '>',   '<',   '<',   '<',   '>',   '>',
    /* 运  ^ */    '>',   '>',   '>',   '>',   '>',   '<',   '<',   '>',   '>',
    /* 算  ! */    '>',   '>',   '>',   '>',   '>',   '>',   ' ',   '>',   '>',
    /* 符  ( */    '<',   '<',   '<',   '<',   '<',   '<',   '<',   '=',   ' ',
    /* |   ) */    ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',
    /* -- \0 */    '<',   '<',   '<',   '<',   '<',   '<',   '<',   ' ',   '='
};

void readNumber(char* p, Stack<float>& stk){
    stk.push((float)(*p - '0'));
    while(isdigit(*(++p)))//If there exist any other numbers behind it
        stk.push(stk.pop()*10+(*p-'0'));
    if('.' != *p) 
        return;
    float fraction = 1;
    while(isdigit(*(++p)))
        stk.push(stk.pop() + (*p - '0')*(fraction /= 10));
}
  
Operator optr2rank(char op){
    switch(op){
        case '+': return ADD;
        case '-': return SUB;
        case '*': return MUL;
        case '/': return DIV;
        case '^': return POW;
        case '!': return FAC;
        case '(': return L_P;
        case ')': return R_P;
        case '\0': return EOE;
        default: exit(-1);
    }
}

char orderBetween(char op1, char op2){
    return pri[optr2rank(op1)][optr2rank(op2)];
}
void append(char*& rpn, float opnd){
    int n = strlen(rpn);
    char buf[64];
    if(opnd != (float)(int)opnd) sprintf(buf, "%.2f ", opnd);
    else                         sprintf(buf, "%d ", (int)opnd);
    //rpn = (char*)realloc(rpn, sizeof(char)*(n + strlen(buf) + 1));
    strcat(rpn, buf);
}

void append(char*& rpn, char optr){
    int n = strlen(rpn);
    //rpn = (char*)realloc(rpn, sizeof(char)*(n+3));
    sprintf(rpn + n, "%c ", optr);
    rpn[n+2] = '\0';
}

double calcu ( double a, char op, double b ) { //执行二元运算
   switch ( op ) {
      case '+' : return a + b;
      case '-' : return a - b;
      case '*' : return a * b;
      case '/' : if ( 0==b ) exit ( -1 ); return a/b; //注意：如此判浮点数为零可能不安全
      case '^' : return pow ( a, b );
      default  : exit ( -1 );
   }
}

double calcu ( char op, double b ) { //执行一元运算
   switch ( op ) {
      case '!' : return ( double ) facI ( ( int ) b ); //目前仅有阶乘，可照此方式添加
      default  : exit ( -1 );
   }
}

float evaluate(char* S, char*& RPN){
    Stack<float> opnd;
    Stack<char> optr;
    optr.push('\0');
    while(!optr.empty()){
        if(isdigit(*S)){
            readNumber(S, opnd);
            append(RPN, opnd.top());
        } else {
            switch(orderBetween(optr.top(), *S)){
                case '<':
                    optr.push(*S);
                    S++;
                    break;
                case '=':
                    optr.pop();
                    S++;
                    break;
                case '>': {
                    char op = optr.pop();
                    append(RPN, op);
                    if('!' == op){
                        float pOpnd = opnd.pop();
                        opnd.push(calcu(op, pOpnd));
                    } else {
                        float pOpnd2 = opnd.pop(), pOpnd1 = opnd.pop();
                        opnd.push(calcu(pOpnd1, op, pOpnd2));
                    }
                    break;
                }
                default: exit(-1);
            }
        }
    }
    return opnd.pop();
}

int main(){
    char RPN[50];
    char* ptr = RPN;
    char src[50] =  "(0!+1)*2^(3!+4)-(5!-67-(8+9))";
    evaluate(src, ptr);
    for(int i = 0; i < 20; i++){
        printf("%c", RPN[i]);
    }
    printf("\n");
    return 0;
}