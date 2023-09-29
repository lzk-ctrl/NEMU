#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include "memory/cache.h"

uint32_t dram_read(hwaddr_t addr, size_t len);

uint32_t findSymbol(char *args,int len);

enum {
	NOTYPE = 256, EQ,HEXNUM,NUM,NOTEQ,REG,AND,OR,ADD,DEREF,NEG,VAR,

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"==", EQ},						// equal
        {"0x[0-9,a-f,A-F]+",HEXNUM},                                //hexadecimal number
        {"[0-9]+",NUM},                                         //number
        {"!=",NOTEQ},                                            //not equal
        {"\\(",'('},                                            //left parentheses
        {"\\)",')'},                                            //right parentheses
        {"\\-",'-'},                                            //minus
        {"\\*",'*'},                                            //multiply
        {"\\/",'/'},                                            //divide
        {"\\$[a-z]+",REG},                                      //register
        {"&&",AND},                                             //and
        {"\\|\\|",OR},                                           //or
        {"\\*0x[0-9,a-f,A-F]+",ADD},                               //adress
        {"!",'!'},                                                 //not
	{"[a-z,A-Z,_][a-z,A-Z,0-9,_]*",VAR},			//
};                  

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}         
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i,temp;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {          
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				//Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */
				substr_len=substr_len>31?31:substr_len;
				switch(rules[i].token_type) {
                                        case NOTYPE:
                                        break;  
                                        case HEXNUM:
                                        case REG:
                                        case NUM:   
                                        
                                        case '+':       
                                        case EQ:
                                        case NOTEQ:
                                        case '(':
                                        case ')':
                                        case '-':
                                        case '*':
                                        case '/':
                                        case AND:
                                        case OR:
                                        case '!':
                                        strncpy(tokens[nr_token].str, substr_start, substr_len);
					tokens[nr_token].str[substr_len] = '\0'; 
                                        tokens[nr_token].type=rules[i].token_type;
                                        nr_token++;
					break;
					case VAR:
					temp = findSymbol(substr_start,substr_len);
					if(temp)
					{	sprintf(tokens[nr_token].str,"%u",temp);
						tokens[nr_token].type = NUM;
						++nr_token;
					}
					else
					{
						substr_start[substr_len]='\0';
						printf("The symbol \"%s\" was not found.\n",substr_start);
						return false;
					}
					break;
                                        default:
					printf("Unimplemented token %s \n", rules[i].regex);
                                        break;
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}                         
	}

	return true; 
}




bool check_parentheses(int p, int q, bool *success)
{
	int cnt = 0, i;
	if (tokens[p].type == '(' && tokens[q].type == ')')
	{
		
		for (i = p; i < q; i++)
		{
			if (tokens[i].type == '(')
				++cnt;
			else if (tokens[i].type == ')')
			{
				if (!cnt)
				{
					*success = false; 
					return false;
				}
				else
					--cnt;
			}
			if (!cnt)
				return false;
		}
		if (cnt > 1)
		{
			*success = false; 
			return false;
		}
		return true;
	}
	else if (tokens[p].type == ')' || tokens[q].type == '(')
		*success = false; 
	return false;
}

uint32_t order(int idx){
	switch (idx)
	{
		
                case NEG:
		case '!': 
		case DEREF: return 1;
		case '*': 
	        case '/': return 2;
                case '+': return 3;
 		case '-': return 3;
	        case  EQ:  return 4;
	        case NOTEQ: return 4;
	        case AND: return 5;
	        case OR:  return 6;
                default :
                return 0;
	}
}

uint32_t eval(int p,int q,bool *success){
        int res;
        int op;
        int a,b;
        if(p>q){
        /*Bad expression*/
        return *success=false;
        }              
        else if(p==q){
             
             if(tokens[p].type == NUM){
                   sscanf(tokens[p].str,"%d",&res);
                   return res;
             }
             if(tokens[p].type == HEXNUM){
                   sscanf(tokens[p].str,"%x",&res);
                   return res;
             }
             if(tokens[p].type == ADD){
                   sscanf(tokens[p].str,"%x",&res);
                   return res;
             }
             if(tokens[p].type == REG){
                   int i;
                   if(strcmp(tokens[p].str+1,"eip") == 0)
                   return cpu.eip;
                   for(i=0;i<8;i++){
                        if(strcmp(tokens[p].str+1,regsl[i]) == 0)
                        return reg_l(i);
                   }
             }
        }
        else if(check_parentheses(p,q,success))
             return eval(p+1,q-1,success);
	else if(!(*success))
	{
		return 0;	
	}
        else {
	op=p;
	int par=0;
        int i;
	for(i=p;i<=q;++i)
	{	
		if(tokens[i].type=='(')
			par++;
		else if(tokens[i].type==')')
			par--;
		if(par==0&&order(tokens[i].type)>=order(tokens[op].type))
			op=i;
			
	}        

        
	
        switch(tokens[op].type){
             case '+': 
		a=eval(p,op-1,success);
        	b=eval(op+1,q,success);
		return a + b;
	     case '*': 
		
		a=eval(p,op-1,success);
        	b=eval(op+1,q,success);
		return a * b;
	     case '/': 
		a=eval(p,op-1,success);
        	b=eval(op+1,q,success);
		return a / b;	
	     case '-': 
		a=eval(p,op-1,success);
        	b=eval(op+1,q,success);
		return a - b;
	     case  EQ:  
		a=eval(p,op-1,success);
        	b=eval(op+1,q,success);
		return a == b;
	     case NOTEQ: 
		a=eval(p,op-1,success);
       		b=eval(op+1,q,success);
		return a != b;
	     case AND: 
		a=eval(p,op-1,success);
        	b=eval(op+1,q,success);
		return a && b;
	     case OR: 
		a=eval(p,op-1,success);
        	b=eval(op+1,q,success);
		return a || b;
 	     case NEG: 
		b=eval(op+1,q,success);
		return -b;
             case DEREF: 
		b=eval(op+1,q,success);
		b= swaddr_read(b,4,R_DS);
		return b;
             case '!': 
		b=eval(op+1,q,success);
		return !b;
             default :
		
		*success=false;
		return 0;
             }
        }
        
        

   
        return 0;
}




uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */

        int i;
	for(i=0;i<nr_token;i++){
                if(tokens[i].type == '*' && (i==0||(tokens[i-1].type!= NUM && tokens[i-1].type!=HEXNUM && tokens[i-1].type!=REG && tokens[i-1].type!='*'&&tokens[i-1].type!=')'))){
                tokens[i].type = DEREF;
                }
                else if(tokens[i].type == '-' && (i==0||(tokens[i-1].type!= NUM && tokens[i-1].type!=HEXNUM && tokens[i-1].type!=REG && tokens[i-1].type!='*'&&tokens[i-1].type!=')'))){
                tokens[i].type = NEG;
                }
        } 
        
	return eval(0,nr_token-1,success);
}



