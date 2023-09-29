#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <elf.h>
uint32_t get_mark_value(char* str);
enum {
	NOTYPE = 256, EQ, NUM, REGNAME, HEXNUM, AND, OR, NOTEQ, NOT, POINTER, MINUS,MARK

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
	int priority;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", NOTYPE, 0},				// spaces
	{"\\+", '+', 3},					// plus
	{"==", EQ, 2},	  					// equal
	{"\\-", '-', 3},					// minus
	{"\\*", '*', 4},					// multiply
	{"\\/", '/', 4},					// divide
 	{"\\$[a-z]{2,3}", REGNAME, 0},		// a register name
	{"\\b0[xX][0-9a-fA-F]+\\b", HEXNUM, 0},		// a hexadecimal num
    {"[0-9]+", NUM, 0},				// a decimal number
	{"\\(", '(', 6},					// left parenthesis
	{"\\)", ')', 6},					// right parenthesis
	{"&&", AND, 1},						// logical and
	{"\\|\\|", OR, 1},					// logical or
	{"!=", NOTEQ, 2},					// not equal
	{"!", NOT, 1},						// not
	{"\\-",MINUS,1},							// minus
	{"\\*",POINTER,1},						// pointer
	{"\\b[a-zA-Z0-9_]+\\b",MARK},
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
	int priority;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
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

				switch(rules[i].token_type) {
					case REGNAME://regname don't need '$'
						tokens[nr_token].type=rules[i].token_type;
						tokens[nr_token].priority=rules[i].priority;	
						strncpy(tokens[nr_token].str,substr_start+1,substr_len-1);
						tokens[nr_token].str[substr_len-1]='\0';
						nr_token++;
						break;
						
					case NOTYPE:break;
					//case MINUS:break;					
					default: //other conditions
						tokens[nr_token].type=rules[i].token_type;
     		            tokens[nr_token].priority=rules[i].priority;                        
						strncpy(tokens[nr_token].str,substr_start,substr_len);
						tokens[nr_token].str[substr_len]='\0';
						nr_token++;
						//panic("please implement me");
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
bool check_parentheses(int p,int q){
	int l=0,r=0;
	int i;
	if(tokens[p].type=='(' && tokens[q].type==')'){
		for(i=p+1;i<q;i++){
			if(tokens[i].type==')') l++;
			if(tokens[i].type=='(') r++;
			if(l>r) return false;
		}
		if(l==r) return true;
 	}
	return false;
}

int dominant_operator(int p,int q){
	int i,j;
	int min=5;//Minimum priority 
	int operator=0;
	for(i=p;i<=q;i++){
		if(tokens[i].type==NUM || tokens[i].type==HEXNUM || tokens[i].type==REGNAME)
		continue;
		int cnt=0;
		bool flag=true;
		for(j=i-1;j>=p;j--){		
			if(tokens[j].type=='(' && !cnt){
				flag=false;
				break;
			}		
			if(tokens[j].type=='(')	cnt++;
			if(tokens[j].type==')') cnt--;
		}//Check if it is in parentheses
		if(!flag) continue;
		if(tokens[i].priority<=min){
			min=tokens[i].priority;
			operator=i;
		}

	}
	return operator;
}//The last calculated operator

int eval(int p,int q){
	if(p>q){
		//Assert(0,"bad expression\n");
		return 0;
	}
	else if(p==q){
		int num;
		num = 0;
		switch(tokens[p].type){
			case MARK: {
				num=get_mark_value(tokens[p].str);
			}
			case NUM: sscanf(tokens[p].str,"%d",&num);return num;
			case HEXNUM: sscanf(tokens[p].str,"%x",&num);return num;
			case REGNAME: 
			{
				if(strlen(tokens[p].str)==3)
				{
					int i;
					for(i=R_EAX;i<=R_EDI;i++)
					{
						if(!strcmp(tokens[p].str,regsl[i]))break;
					}
					if(i>R_EDI)
					{
						if(!strcmp(tokens[p].str,"eip"))
						num=cpu.eip;
						else
						Assert(0,"there is no this register!\n");
					}
					else num=reg_l(i);					
				}
				else if(strlen(tokens[p].str)==2)
				{
					if(tokens[p].str[1]=='x'||tokens[p].str[1]=='p'||tokens[p].str[1]=='i')
					{
						int i;
						for(i=R_AX;i<=R_DI;i++)
							if(!strcmp(tokens[p].str,regsw[i]))break;
						num=reg_w(i);
					}
					if(tokens[p].str[1]=='l'||tokens[p].str[1]=='h')
					{
						int i;
						for(i=R_AL;i<=R_BH;i++)
							if(!strcmp(tokens[p].str,regsb[i]))break;
						num=reg_b(i);
					}
					else assert(1);
				}
				return num;
			}
		}

	}
	else if(check_parentheses(p,q)==true){
	return eval(p+1,q-1);
	}
	else {
		int op= dominant_operator(p,q);
		if(p==op||tokens[op].type==POINTER||tokens[op].type==MINUS||tokens[op].type==NOT){
			int val=eval(p+1,q);
			switch(tokens[op].type){
				case NOT: return !val;
				case MINUS: return -val;
				case POINTER: return swaddr_read(val,4);
				default: Assert(0,"wrong symbols exist\n");		
			}
		}
		int val1=eval(p,op-1);
		int val2=eval(op+1,q);
		
		switch(tokens[op].type){
			case '+': return val1+val2;
			case '-': return val1-val2;
			case '/': return val1/val2;
			case '*': return val1*val2;
			case EQ: return val1==val2;
			case NOTEQ: return val1!=val2;
			case AND: return val1&&val2;
			case OR: return val1||val2;
 			default: Assert(0,"Undocumented symbols exist\n");
			
		}
	}
	return 0;
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	int i;
	for(i=0;i<nr_token;i++){
		if(tokens[i].type=='*'&&(i==0||(tokens[i-1].type!=NUM&&tokens[i-1].type!=HEXNUM&&tokens[i-1].type!=REGNAME))){
			tokens[i].type=POINTER;
			tokens[i].priority=5;
		}
		if(tokens[i].type=='-'&&(i==0||(tokens[i-1].type!=NUM&&tokens[i-1].type!=HEXNUM&&tokens[i-1].type!=REGNAME))){
			tokens[i].type=MINUS;
			tokens[i].priority=5;
		}
	}
	*success = true;

	/* TODO: Insert codes to evaluate the expression. */
	//panic("please implement me");
	return eval(0,nr_token-1);
}

