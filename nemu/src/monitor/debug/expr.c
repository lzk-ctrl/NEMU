#include "nemu.h"
#include "elf.h"
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

extern int var_read(char *);

enum
{
	NOTYPE = 256,
	EQ,
	UEQ,
	ADDR,
	XNUM,
	NUM,
	VARIABLE,
	DEREF,
	NEG,
	LOGNOT,
	REG,
	AND,
	OR,
	XOR

	/* TODO: Add more token types */

};

static struct rule
{
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", NOTYPE},					 // spaces
	{"==", EQ},						 // equal
	{"!=", UEQ},					 // unequal
	{"\\*0x[0-9a-fA-F]+", ADDR},	 // address
	{"0x[0-9a-fA-F]+", XNUM},		 // 0xnumber
	{"[0-9]+", NUM},				 // number
	{"[a-zA-Z,\\_,0-9]+", VARIABLE}, // object
	{"\\!", LOGNOT},				 // logical not
	{"\\$[a-z]{2,3}", REG},			 // regname
	{"\\&\\&", AND},				 // and
	{"\\|\\|", OR},					 // or
	{"\\^", XOR},					 // xor
	{"\\(", '('},					 // left partentheses
	{"\\)", ')'},					 // right partentheses
	{"\\*", '*'},					 // mut
	{"\\/", '/'},					 // dev
	{"\\+", '+'},					 // plus
	{"\\-", '-'},					 // min
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++)
	{
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0)
		{
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token
{
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e)
{
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0')
	{
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++)
		{
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
			{
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				// Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch (rules[i].token_type)
				{
				case NOTYPE:
					break;
				default:
					tokens[nr_token].type = rules[i].token_type;
					sprintf(tokens[nr_token].str, "%.*s", substr_len, substr_start);
					++nr_token;
				}
				break;
			}
		}

		if (i == NR_REGEX)
		{
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

int find_dominant_opterater(int p, int q)
{
	int op = -1;
	int rank = 6;
	int part = 0;
	int i;
	for (i = p; i < q; i++)
	{
		if (tokens[i].type == '(')
			part++;
		else if (tokens[i].type == ')')
			part--;
		if ((part == 0) && (tokens[i].type == LOGNOT || tokens[i].type == NEG || tokens[i].type == DEREF) && (rank > 5))
		{
			rank = 5;
			op = i;
		}
		else if ((part == 0) && ((tokens[i].type == '*') || (tokens[i].type == '/')) && (rank > 4))
		{
			rank = 4;
			op = i;
		}
		else if ((part == 0) && ((tokens[i].type == '+') || (tokens[i].type == '-')) && (rank > 3))
		{
			rank = 3;
			op = i;
		}
		else if ((part == 0) && ((tokens[i].type == EQ) || (tokens[i].type == UEQ)) && (rank > 2))
		{
			rank = 2;
			op = i;
		}
		else if ((part == 0) && ((tokens[i].type == AND) || (tokens[i].type == XOR)) && (rank > 1))
		{
			rank = 1;
			op = i;
		}
		else if ((part == 0) && (tokens[i].type == OR) && (rank > 0))
		{
			rank = 0;
			op = i;
		}
	}
	return op;
}
bool check_error = true;
bool check_partentheses(int p, int q)
{
	int i;
	int lpth = 0;
	if (tokens[p].type != '(' || tokens[q].type != ')')
	{
		return false;
	}
	for (i = p; i <= q; i++)
	{
		if (lpth == 0 && i != p && i != q)
		{
			check_error = false;
			return false;
		}
		if (tokens[i].type == '(')
			lpth++;
		else if (tokens[i].type == ')')
			lpth--;
	}
	if (lpth == 0)
	{
		return true;
	}
	return false;
}

int eval(int p, int q)
{
	if (check_error == false)
	{
		return 0;
	}
	if (p > q)
	{
		check_error = false;
		return 0;
	}
	else if (p == q)
	{
		int fin = 0;
		switch (tokens[p].type)
		{
		case XNUM:
			sscanf(tokens[p].str, "0x%x", &fin);
			return fin;
		case NUM:
			sscanf(tokens[p].str, "%d", &fin);
			return fin;
		case ADDR:
			sscanf(tokens[p].str, "*0x%x", &fin);
			return swaddr_read(fin, 4, R_DS);
		case REG:
			if (strcmp(tokens[p].str, "$eip") == 0)
				return cpu.eip;
		case VARIABLE:
			return var_read(tokens[p].str);
		}
		int i;
		for (i = R_EAX; i < R_EDI; i++)
		{
			if (strcmp(tokens[p].str + 1, regsl[i]) == 0)
				return reg_l(i);
		}
	}
	else if (check_partentheses(p, q) == true)
	{
		return eval(p + 1, q - 1);
	}
	else
	{
		int op;
		int n1, n2;
		op = find_dominant_opterater(p, q);
		n2 = eval(op + 1, q);
		if (tokens[op].type == LOGNOT)
			return !n2;
		if (tokens[op].type == NEG)
			return -n2;
		if (tokens[op].type == DEREF)
			return swaddr_read(n2, 4, R_DS);
		n1 = eval(p, op - 1);
		switch (tokens[op].type)
		{
		case '+':
			return n1 + n2;
		case '-':
			return n1 - n2;
		case '*':
			return n1 * n2;
		case '/':
			return n1 / n2;
		case AND:
			return n1 & n2;
		case OR:
			return n1 | n2;
		case XOR:
			return n1 ^ n2;
		case EQ:
			return n1 == n2;
		case UEQ:
			return n1 != n2;
		}
	}
	return 0;
}

int pre_type = 0;
#define expr_test (pre_type == '(' ||                     \
				   pre_type == NUM || pre_type == XNUM || \
				   pre_type == ADDR || pre_type == REG || \
				   pre_type == VARIABLE)

uint32_t expr(char *e, bool *success)
{
	if (!make_token(e))
	{
		*success = false;
		return 0;
	}
	int i;
	for (i = 0; i < nr_token; i++)
	{
		if (tokens[i].type == '-')
		{
			if (i == 0)
			{
				tokens[i].type = NEG;
				continue;
			}
			pre_type = tokens[i - 1].type;
			if (!expr_test)
				tokens[i].type = NEG;
		}

		if (tokens[i].type == '*')
		{
			if (i == 0)
			{
				tokens[i].type = DEREF;
				continue;
			}
			pre_type = tokens[i - 1].type;
			if (!expr_test)
				tokens[i].type = DEREF;
		}
	}
	*success = check_error;
	return eval(0, nr_token - 1);
}
