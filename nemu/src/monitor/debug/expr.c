#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <elf.h>

extern Elf32_Sym *symtab;
extern int nr_symtab_entry;
extern char* strtab;
uint32_t GetMarkValue(char *str,bool *success);

enum {
	NOTYPE = 256, EQ, NUM, HEXNUM, REG, NOTEQ, OR, AND, NOT, MARK

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{"0x[0-9a-fA-F]+", HEXNUM},		//HEXNUM
	{"[0-9]+", NUM},				//NUM
	{"\\$[a-z]{2,3}+",REG},
	{"\\|\\|", OR},					//OR
	{"&&", AND},					//AND
	{"!=", NOTEQ},					//NOTEQ
	{"!", NOT},						//NOT
	{" +",	NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"==", EQ},						// equal
	{"\\-",'-'},					//减
	{"\\*",'*'},					//乘
	{"\\/",'/'},					//除
	{"\\(",'('},					//(
	{"\\)", ')'},					//)
	{"\\b[a-zA-Z0-9_]+\\b",MARK},         // mark
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
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					case NOTYPE: break;
					default: 
						tokens[nr_token].type=rules[i].token_type;
						int j;
						for( j=0;j<substr_len;j++)
							tokens[nr_token].str[j]=substr_start[j];
						tokens[nr_token].str[substr_len]='\0';
						nr_token++;
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
int the_left(int p,int q)
{
	int i = 1;
	int flag=1;
	if(tokens[q].type != ')')
		return -1;
	else
	{
		for( i = q-1;i>=p;i--)
		{
			if(tokens[i].type == ')') flag++;
			else if(tokens[i].type == '(') flag--;
			if(flag == 0) return i;
		}
	}
	return -1;
}
bool check_parentheses(int p,int q)
{
	int ans=0;
	if(tokens[p].type!='('||tokens[q].type!=')')	return false;
	int i;
	for(i=p;i<=q;i++)
	{
		if(tokens[i].type=='(')	ans++;
		else if(tokens[i].type==')') ans--;

	}
	if(ans!=0)	return false;
	return true;
}
int eval(int p,int q)
{
	if(p>q)	return -1;
	else if(p==q)
	{
		int num;
		if(tokens[p].type==NUM)
		{
			sscanf(tokens[p].str,"%d",&num);
			return num;
		}
		else if(tokens[p].type==HEXNUM)
		{
			sscanf(tokens[p].str+2,"%x",&num);
			return num;
		}
		else if(tokens[p].type==REG)
		{
			int i;
			for(i=0;i<8;i++)
			{
				if(strcmp(regsl[i],tokens[p].str+1)==0)
					return cpu.gpr[i]._32;
			}
			for( i=0;i<8;i++)
			{
				if(strcmp(regsw[i],tokens[p].str+1)==0)
					return cpu.gpr[i]._16;
			}
			for(i=0;i<4;i++)
			{
				if(strcmp(regsb[i],tokens[p].str+1)==0)
					return cpu.gpr[i]._8[0];
				else if(strcmp(regsb[i+4],tokens[p].str+1)==0)
					return cpu.gpr[i]._8[1];
			}
			if(strcmp(tokens[p].str+1,"eip")==0)
				return cpu.eip;
		}
		else if(tokens[p].type==MARK)
		{
			bool success=false;
			return GetMarkValue(tokens[p].str , &success);
		}
	}
	else if(check_parentheses(p,q))
	{
		return eval(p+1,q-1);
	}
	else
	{
		int i;
		int j;
		for(i=0;i<4;i++)//按4个优先级进行运算
		{
			for(j=q;j>=p;j--)
			{
				if(tokens[j].type==')')
				{
					j=the_left(p,j);
				}
				if(i==0)
				{
					if(tokens[j].type==AND)
						return eval(p,j-1)&&eval(j+1,q);
					else if(tokens[j].type==OR)
						return eval(p,j-1)||eval(j+1,q);
					else if(tokens[j].type==NOT)
						return eval(p,j-1)&&!eval(j+1,q);
				}
				else if(i==1)
				{
					if(tokens[j].type==EQ)
						return eval(p,j-1)==eval(j+1,q);
					else if(tokens[j].type==NOTEQ)
						return eval(p,j-1)!=eval(j+1,q);
				}
				else if(i==2)
				{
					if(tokens[j].type=='+')
						return eval(p,j-1)+eval(j+1,q);
					else if(tokens[j].type=='-')
						return eval(p,j-1)-eval(j+1,q);
				}
				else
				{
					if(tokens[j].type=='*')
						return eval(p,j-1)*eval(j+1,q);
					else if(tokens[j].type=='/')
						return eval(p,j-1)/eval(j+1,q);
				}
			}
		}
	}
	return -1;
}
uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	*success = true;
	int result = 0; 
	result = eval(0,nr_token-1);
	return result;
	panic("please implement me");
	return 0;
}

