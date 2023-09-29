#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <elf.h>
extern uint32_t getVariable();
enum
{
    NOTYPE = 256,
    EQ,
    NE,
    HEXNUMBER,
    NUMBER,
    REGNAME,
    AND,
    OR,
    BITAND,
    BITOR,
    BITXOR,
    NOT,
    NEGATIVE,
    DEREFERENCE,
    VARIABLE,

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

    {" +", NOTYPE},                  // spaces
    {"\\+", '+'},                    // plus
    {"\\-", '-'},                    // minus
    {"\\*", '*'},                    // multiply
    {"\\/", '/'},                    // devide
    {"\\(", '('},                    // left parentheses
    {"\\)", ')'},                    // right parentheses
    {"==", EQ},                      // equal
    {"!=", NE},                      // not equal
    {"0[xX][0-9, a-f]+", HEXNUMBER}, // huxnumber
    {"[0-9]+", NUMBER},              // number
    {"\\$[a-z]{2,3}", REGNAME},      //regname
    {"&&", AND},                     //and
    {"\\|\\|", OR},                  //or
    {"&", BITAND},                   //bit and
    {"\\|", BITOR},                  //bit or
    {"\\^", BITXOR},                 //bit xor
    {"!", NOT},                      //not
    {"[a-zA-Z][A-Za-z0-9_]*", VARIABLE},
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

                //char *ls = e + position + 1;  //ฒน
                //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
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
                    memset(&tokens[nr_token].str, 0, sizeof(tokens[nr_token].str));
                    strncpy(tokens[nr_token].str, substr_start, substr_len);
                    nr_token++;
                }

                /*if (substr_len > 32) { // in case str overflow
                    puts("Too Long expr!!!");
                    assert(0);
                }
                if (rules[i].token_type == NOTYPE) {
                    break;
                 }
                if (rules[i].token_type == REGNAME) {
                    strncpy(tokens[nr_token].str, ls, substr_len - 1);
                    tokens[nr_token].str[substr_len-1] = '\0';
                } 
                else if (rules[i].token_type == VARIABLE) {
                    strncpy(tokens[nr_token].str, e + position - substr_len, substr_len);
                    tokens[nr_token].str[substr_len] = '\0';
                } 
                else {
                strncpy(tokens[nr_token].str, substr_start, substr_len);
                tokens[nr_token].str[substr_len] = '\0';
                }
                tokens[nr_token].type = rules[i].token_type;
                //tokens[nr_token].prior = rules[i].prior;
                nr_token++;*/

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

bool check_parentheses(int p, int q)
{
    int i, num = 0;
    if (tokens[p].type != '(' || tokens[q].type != ')')
        return false;
    for (i = p; i <= q; i++)
    {
        if (tokens[i].type == '(')
            num++;
        else if (tokens[i].type == ')')
            num--;
        if (num == 0 && i < q)
            return false;
    }
    if (num != 0)
        return false;
    return true;
}

int getPriority(int type)
{
    switch (type)
    {
    case NEGATIVE:
        return 7;
    case DEREFERENCE:
        return 7;
    case NOT:
        return 7;
    case '*':
        return 6;
    case '/':
        return 6;
    case '+':
        return 5;
    case '-':
        return 5;
    case BITAND:
        return 4;
    case BITOR:
        return 3;
    case BITXOR:
        return 4;
    case AND:
        return 2;
    case OR:
        return 1;
    case EQ:
        return 0;
    case NE:
        return 0;
    default:
        return 10;
    }
}

int dominant_operator(int p, int q)
{
    int i, pos = p, num = 0, pri = 10;
    for (i = p; i <= q; i++)
    {
        if (tokens[i].type == '(')
            num++;
        else if (tokens[i].type == ')')
            num--;
        if (getPriority(tokens[i].type) <= pri && num == 0)
        {
            pri = getPriority(tokens[i].type);
            pos = i;
        }
    }
    return pos;
}

uint32_t eval(int p, int q)
{
    uint32_t res = 0;
    if (p > q)
        ;
    else if (p == q)
    {
        if (tokens[p].type == NUMBER)
        {
            res = atoi(tokens[p].str);
            return res;
        }
        else if (tokens[p].type == HEXNUMBER)
        {
            res = strtol(tokens[p].str, NULL, 16);
            return res;
        }
        else if (tokens[p].type == REGNAME)
        {
            if (strcmp(tokens[p].str + 1, "eip") == 0)
                return cpu.eip;
            int i;
            for (i = R_EAX; i < R_EDI; i++)
                if (strcmp(tokens[p].str + 1, regsl[i]) == 0)
                    return reg_l(i);
        }
        else if (tokens[p].type == VARIABLE)
            return getVariable(tokens[p].str);
    }
    else if (check_parentheses(p, q))
    {
        return eval(p + 1, q - 1);
    }
    else
    {
        int op = dominant_operator(p, q);
        if (tokens[op].type == NEGATIVE)
            return -eval(op + 1, q);
        if (tokens[op].type == DEREFERENCE)
            return swaddr_read(eval(op + 1, q), 4);
        if (tokens[op].type == NOT)
            return !eval(op + 1, q);
        uint32_t val1 = eval(p, op - 1), val2 = eval(op + 1, q);
        switch (tokens[op].type)
        {
        case '+':
            return val1 + val2;
        case '-':
            return val1 - val2;
        case '*':
            return val1 * val2;
        case '/':
            return val1 / val2;
        case EQ:
            return val1 == val2;
        case NE:
            return val1 != val2;
        case AND:
            return val1 && val2;
        case OR:
            return val1 || val2;
        case BITAND:
            return val1 & val2;
        case BITOR:
            return val1 | val2;
        case BITXOR:
            return val1 ^ val2;
        default:
            assert(0);
        }
    }
    return res;
}

uint32_t expr(char *e, bool *success)
{
    if (!make_token(e))
    {
        *success = false;
        return 0;
    }

    /* TODO: Insert codes to evaluate the expression. */
    int i;
    for (i = 0; i < nr_token; i++)
    {
        if (tokens[i].type == '-' && (i == 0 || getPriority(tokens[i - 1].type) < 10 || tokens[i - 1].type == '('))
            tokens[i].type = NEGATIVE;
        if (tokens[i].type == '*' && (i == 0 || getPriority(tokens[i - 1].type) < 10 || tokens[i - 1].type == '('))
            tokens[i].type = DEREFERENCE;
    }
    return eval(0, nr_token - 1);
}
