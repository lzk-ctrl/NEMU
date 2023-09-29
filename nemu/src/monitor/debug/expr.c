#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <elf.h>

enum {
	NOTYPE = 256, EQ = 257, NOTEQ =258, DEC = 259, HEX = 260, REG = 261, NEG=262, ADD=263, NAME=264

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{"0x[0-9,a-f,A-F]+", HEX},      // hexadecimal
    {"[0-9]+", DEC},                // decimalism
    {"\\$[a-z]{2,3}", REG},         // register 
    {" +", NOTYPE},                 // spaces
    {"\\+", '+'},                   // plus
    {"\\-", '-'},                   // sub
    {"\\*", '*'},                   // mul
    {"\\/", '/'},                   // divide
    {"\\(", '('},                   // left parenthesis
    {"\\)", ')'},                   // right parenthesis
    {"!=", NOTEQ},                  // not equal
    {"==", EQ},	                    // equal
    {"\\!", '!'},                   // not
    {"\\&\\&", '&'},                // and
    {"\\|\\|", '|'},                // or
    {"[a-z,A-Z,0-9,_]+", NAME}
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
				    case HEX:
				        tokens[nr_token].type=HEX;
				        strncpy(tokens[nr_token].str,substr_start+2,substr_len-2);
				        tokens[nr_token].str[substr_len-2]='\0';
				        nr_token ++;
				        break;
				    case REG:
				        tokens[nr_token].type=REG;
				        strncpy(tokens[nr_token].str,substr_start+1,substr_len-1);
				        tokens[nr_token].str[substr_len-1]='\0';
				        nr_token ++;
				        break;
				    case '-':
				        if(nr_token==0) tokens[nr_token].type=NEG;
				        else if(tokens[nr_token-1].type!=DEC&&tokens[nr_token-1].type!=HEX&&tokens[nr_token-1].type!=REG&&tokens[nr_token-1].type!=')') tokens[nr_token].type=NEG;
				        else tokens[nr_token].type='-';
				        tokens[nr_token].str[0]='-';
				        tokens[nr_token].str[1]='\0';
				        nr_token ++;
				        break;
				    case '*':
				        if(nr_token==0) tokens[nr_token].type=ADD;
				        else if(tokens[nr_token-1].type!=DEC&&tokens[nr_token-1].type!=HEX&&tokens[nr_token-1].type!=REG&&tokens[nr_token-1].type!=')') tokens[nr_token].type=ADD;
				        else tokens[nr_token].type='*';
				        tokens[nr_token].str[0]='*';
				        tokens[nr_token].str[1]='\0';
				        nr_token ++;
				        break;
				   
				    case DEC:
				    case '+':
				    case '/':
				    case '(':
				    case ')':
				    case '!':
				    case '&':
				    case '|':
				    case EQ:
				    case NOTEQ:
				    case NAME:
				        tokens[nr_token].type=rules[i].token_type;;
				        strncpy(tokens[nr_token].str,substr_start,substr_len);
				        tokens[nr_token].str[substr_len]='\0';
				        nr_token ++;
				        break;
				      
				    case NOTYPE: break;
					default: panic("please implement me");
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

static bool is_valid(char*e) {
    int pos=0,num=0;
    while(e[pos] != '\0') {
        if(e[pos] == '(') num++;
        if(e[pos] == ')') {
            num--;
            if(num<0) return false;
        }
        pos++;
    }
    if(num==0) return true;
    else return false;
}

static uint32_t check_parentheses(int start, int end) {
    int pos=start,num=0;
    if(tokens[start].type!='('||tokens[end].type!=')') return false;
    while(pos<end) {
        if(tokens[pos].type == '(') num++;
        if(tokens[pos].type == ')') {
            num--;
            if(num<=0) return false;
        }
        pos++;
    }
    if(num==1)return true;
    return false;
}	

uint32_t dominant_operator(int start,int end) {
    int pos,ans=-1;
    int in_par=0;
    int priority=6;
 //  printf("start: %d  end: %d     ",start,end);
    for(pos=end;pos>=start;pos--) {
        if(tokens[pos].type==')') in_par++;
        if(tokens[pos].type=='(') in_par--;
        if(!in_par&&(tokens[pos].type==NEG||tokens[pos].type==ADD)&&priority==6) ans=pos;
        if(!in_par&&tokens[pos].type=='!'&&priority>5) {
            ans=pos;
            priority=5; 
        }
        if(!in_par&&(tokens[pos].type=='*'||tokens[pos].type=='/')&&priority>4) {
            ans=pos;
            priority=4; 
        }
        if(!in_par&&(tokens[pos].type=='+'||tokens[pos].type=='-')&&priority>3) {
            ans=pos;
            priority=3; 
        }
        if(!in_par&&(tokens[pos].type==EQ||tokens[pos].type==NOTEQ)&&priority>2) {
            ans=pos;
            priority=2; 
        }
        if(!in_par&&tokens[pos].type=='&'&&priority>1) {
            ans=pos;
            priority=1; 
        }
         if(!in_par&&tokens[pos].type=='|'&&priority>0) {
            ans=pos;
            priority=0; 
        }
    }
 //  printf("priority: %d\n",priority);
    return ans;
}   

uint32_t get_address(const char*, bool*); 

uint32_t eval(uint32_t p, uint32_t q, bool* suc) {
    if(*suc==false) return 0;
    if(p>q) {
        printf("incorrect operator use.\n");
        *suc=false;
        return 0;
    }
    else if(p==q) {
        if(tokens[p].type==REG) {
       //    printf("str: %s  \n",tokens[p].str);
            if(strcmp(tokens[p].str,"eip")==0) return cpu.eip;
            int i;
            for(i=0;i<8;i++) {
                if(strcmp(tokens[p].str,regsl[i])==0) return cpu.gpr[i]._32;
       //         printf("str: %s\n",tokens[p].str);
            }
        }
        else if(tokens[p].type==DEC) {
            int res;
            sscanf(tokens[p].str,"%d",&res);
        //    printf("res: %d\n",res);
            return res;
        }
        else if(tokens[p].type==HEX) {
            int temp,ans=0;
		    char val;
		//    printf("str: %s\n",tokens[p].str);
			for(temp=0;temp<32;temp++) {
				val=tokens[p].str[temp];
				if(val>='0'&&val<='9') ans=val-'0'+ans*16;
			    else if(val>='a'&&val<='f') ans=val-'a'+10+ans*16;
				else if(val>='A'&&val<='F') ans=val-'A'+10+ans*16;
				else break;
			}
			return ans;
	    }
	    else if(tokens[p].type==NAME) {
	    	uint32_t ans;
	    //	 printf("str: %s\n",tokens[p].str);
	    	ans=get_address(tokens[p].str,suc);
	    	return ans;
	    }
        else {
            printf("incorrect operator use.\n");
            *suc=false;
            return 0;
        }
    }
  
    else if(check_parentheses(p,q)) {
        return eval(p+1,q-1,suc);
    }
    
    else { 
        int flag,val1=0,val2;
        flag=dominant_operator(p,q);
    //   printf("flag: %d\n",flag);
        if(flag==-1) {
            printf("wrong expression.\n");
            *suc=false;
            return 0;
        }
        if(tokens[flag].type==NEG||tokens[flag].type==ADD||tokens[flag].type=='!') flag=p;
    //  printf("operator: %d   %d \n",flag,tokens[flag].type);
        if(flag!=p) val1=eval(p,flag-1,suc);
        val2=eval(flag+1,q,suc);
    //    printf("val1: %d  val2: %d\n",val1,val2);
        switch(tokens[flag].type) {
            case ADD: return swaddr_read(val2,4/*1*/,R_DS);
            case NEG: return -val2;
            case EQ: return val1==val2;
            case NOTEQ: return val1!=val2;
            case '!': return !val2;
			case '&': return val1&&val2;
			case '|': return val1||val2;
            case '+': return val1+val2;
			case '-': return val1-val2;
			case '*': return val1*val2;
			case '/': return val1/val2;
		    default:
		        printf("incorrect operator use.\n");
		        *suc=false;
		        return 0;
		}
    }		   
    return 0;
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
            *success = false; 
            printf("invaild expression.\n");
            return 0;
	}
	if(!is_valid(e)) {
	    *success = false;
	    printf("incorrect parentheses use.\n");
	    return 0;
	}
	uint32_t result;
	result=eval(0, nr_token-1, success);
	if(*success==true) return result;
	else return 0;
	/* TODO: Insert codes to evaluate the expression. */
	panic("please implement me");
	return 0;
}
