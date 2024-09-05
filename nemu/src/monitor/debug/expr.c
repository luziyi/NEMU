#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <sys/types.h>

enum
{
    NOTYPE = 256,
    NUM = 1,
    REGISTER = 2,
    HEX = 3,
    EQ = 4,
    NOTEQ = 5,
    OR = 6,
    AND = 7,
    POINT = 8,
    NEG = 9,
    ID=10
};

uint32_t getValue(char* str,bool* success);


static struct rule
{
    char *regex;
    int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", NOTYPE}, // spaces

    {"\\+", '+'}, // plus
    {"\\-", '-'},
    {"\\*", '*'},
    {"\\/", '/'},

    {"\\$[a-z]+", REGISTER},
    {"0[xX][0-9a-fA-F]+", HEX},
    {"[0-9]+", NUM},

    {"==", EQ},    // equal
    {"!=", NOTEQ}, // not equal
    {"\\|\\|", OR},
    {"&&", AND},
    {"!", '!'},

    {"\\(", '('},
    {"\\)", ')'},

    {"[a-zA-Z_]{1,31}", ID},

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

bool check_parentheses(int p, int q)
{
    if (tokens[p].type == '(' && tokens[q].type == ')')
    {
        int balance = 0;
        int i;
        for (i = p; i <= q; i++)
        {
            if (tokens[i].type == '(')
                balance++;
            if (tokens[i].type == ')')
                balance--;
            if (balance < 0)
                return false;
        }
        return balance == 0;
    }
    return false;
}

int dominant_operator(int p, int q)
{
    int step = 0;
    int i;
    int op = -1;
    int pri = 0;

    for (i = p; i <= q; i++)
    {
        if (tokens[i].type == '(')
        {
            step++;
        }
        if (tokens[i].type == ')')
        {
            step--;
        }

        if (step == 0)
        {
            if (tokens[i].type == OR)
            {
                if (pri < 51)
                {
                    op = i;
                    pri = 51;
                }
            }
            else if (tokens[i].type == AND)
            {
                if (pri < 50)
                {
                    op = i;
                    pri = 50;
                }
            }
            else if (tokens[i].type == EQ || tokens[i].type == NOTEQ)
            {
                if (pri < 49)
                {
                    op = i;
                    pri = 49;
                }
            }
            else if (tokens[i].type == '+' || tokens[i].type == '-')
            {
                if (pri < 48)
                {
                    op = i;
                    pri = 48;
                }
            }
            else if (tokens[i].type == '*' || tokens[i].type == '/')
            {
                if (pri < 46)
                {
                    op = i;
                    pri = 46;
                }
            }
        }
    }

    return op;
}

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

                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position,
                substr_len, substr_len, substr_start);
                position += substr_len;

                /* TODO: Now a new token is recognized with rules[i]. Add codes
                 * to record the token in the array `tokens'. For certain types
                 * of tokens, some extra actions should be performed.
                 */

                int j;
                for (j = 0; j < 32; j++)
                { //清空
                    tokens[nr_token].str[j] = '\0';
                }

                switch (rules[i].token_type)
                {
                case 256:
                    break;
                case 1:
                    tokens[nr_token].type = NUM;
                    strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
                    nr_token++;
                    break;
                case 2:
                    tokens[nr_token].type = REGISTER;
                    strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
                    nr_token++;
                    break;
                case 3:
                    tokens[nr_token].type = HEX;
                    strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
                    nr_token++;
                    break;
                case 4:
                    tokens[nr_token].type = EQ;
                    strcpy(tokens[nr_token].str, "==");
                    nr_token++;
                    break;
                case 5:
                    tokens[nr_token].type = NOTEQ;
                    strcpy(tokens[nr_token].str, "!=");
                    nr_token++;
                    break;
                case 6:
                    tokens[nr_token].type = OR;
                    strcpy(tokens[nr_token].str, "||");
                    nr_token++;
                    break;
                case 7:
                    tokens[nr_token].type = AND;
                    strcpy(tokens[nr_token].str, "&&");
                    nr_token++;
                    break;
                case '+':
                    tokens[nr_token].type = '+';
                    nr_token++;
                    break;
                case '-':
                    tokens[nr_token].type = '-';
                    nr_token++;
                    break;
                case '*':
                    tokens[nr_token].type = '*';
                    nr_token++;
                    break;
                case '/':
                    tokens[nr_token].type = '/';
                    nr_token++;
                    break;
                case '!':
                    tokens[nr_token].type = '!';
                    nr_token++;
                    break;
                case '(':
                    tokens[nr_token].type = '(';
                    nr_token++;
                    break;
                case ')':
                    tokens[nr_token].type = ')';
                    nr_token++;
                    break;
                case 10:
                    tokens[nr_token].type= ID;
                    strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
                    nr_token++;
                    break;
                default:
                    assert(0);
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

uint32_t eval(int p, int q)
{
    printf("p = %d, q = %d\n", p, q);
    int result = 0;
    int op;
    int val1, val2;
    if (p > q)
    {
        assert(0);
    }
    else if (p == q)
    {
        if (tokens[p].type == NUM)
        {
            sscanf(tokens[p].str, "%d", &result);
            return result;
        }
        else if (tokens[p].type == HEX)
        {
            // printf("tokens[p].str = %s\n", tokens[p].str);
            int i = 2;
            while (tokens[p].str[i] != 0)
            {
                result *= 16;
                result += tokens[p].str[i] < 58 ? tokens[p].str[i] - '0' : tokens[p].str[i] - 'a' + 10;
                i++;
            }
            return result;
        }
        else if (tokens[p].type == REGISTER)
        {
            if (!strcmp(tokens[p].str, "$eax"))
            {
                return cpu.eax;
            }
            else if (!strcmp(tokens[p].str, "$ecx"))
            {
                return cpu.ecx;
            }
            else if (!strcmp(tokens[p].str, "$edx"))
            {
                return cpu.edx;
            }
            else if (!strcmp(tokens[p].str, "$ebx"))
            {
                return cpu.ebx;
            }
            else if (!strcmp(tokens[p].str, "$esp"))
            {
                return cpu.esp;
            }
            else if (!strcmp(tokens[p].str, "$ebp"))
            {
                return cpu.ebp;
            }
            else if (!strcmp(tokens[p].str, "$esi"))
            {
                return cpu.esi;
            }
            else if (!strcmp(tokens[p].str, "$edi"))
            {
                return cpu.edi;
            }
            else if (!strcmp(tokens[p].str, "$eip"))
            {
                return cpu.eip;
            }
            else
            {
                return 0;
            }
        }
        else if(tokens[p].type==ID){
            bool success;
            uint32_t val;
            val = getValue(tokens[p].str,&success);
            if(!success){return 0;}
            return val;
        }
        else
        {
            assert(0);
        }
    }
    else if (check_parentheses(p, q) == true)
    {
        if(check_parentheses(p, q)) printf("true");
        return eval(p + 1, q - 1);
    }
    else
    {
        // printf("\tp = %d, q = %d\n", p, q);
        op = dominant_operator(p, q);
        // printf("op = %d\n", op);
        if (op == -2)
        {
            assert(0);
        }
        else if (op == -1)
        {
            if (tokens[p].type == POINT)
            {
                if (!strcmp(tokens[p + 2].str, "$eax"))
                {
                    result = swaddr_read(cpu.eax, 4);
                    return result;
                }
                else if (!strcmp(tokens[p + 2].str, "$ecx"))
                {
                    result = swaddr_read(cpu.ecx, 4);
                    return result;
                }
                else if (!strcmp(tokens[p + 2].str, "$edx"))
                {
                    result = swaddr_read(cpu.edx, 4);
                    return result;
                }
                else if (!strcmp(tokens[p + 2].str, "$ebx"))
                {
                    result = swaddr_read(cpu.ebx, 4);
                    return result;
                }
                else if (!strcmp(tokens[p + 2].str, "$esp"))
                {
                    result = swaddr_read(cpu.esp, 4);
                    return result;
                }
                else if (!strcmp(tokens[p + 2].str, "$ebp"))
                {
                    result = swaddr_read(cpu.ebp, 4);
                    return result;
                }
                else if (!strcmp(tokens[p + 2].str, "$esi"))
                {
                    result = swaddr_read(cpu.esi, 4);
                    return result;
                }
                else if (!strcmp(tokens[p + 2].str, "$edi"))
                {
                    result = swaddr_read(cpu.edi, 4);
                    return result;
                }
                else if (!strcmp(tokens[p + 2].str, "$eip"))
                {
                    result = swaddr_read(cpu.eip, 4);
                    return result;
                }
            }
            else if (tokens[p].type == NEG)
            {
                sscanf(tokens[q].str, "%d", &result);
                return -result;
            }
        }
        else if (tokens[p].type == '!')
        {
            sscanf(tokens[q].str, "%d", &result);
            return !result;
        }

        val1 = eval(p, op - 1);
        val2 = eval(op + 1, q);
        // printf("val1 = %d, val2 = %d\n", val1, val2);
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
        case OR:
            return val1 || val2;
        case AND:
            return val1 && val2;
        case EQ:
            if (val1 == val2)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        case NOTEQ:
            if (val1 != val2)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        default:
            assert(0);
        }
    }
    return 0;
}

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
        if (tokens[i].type == '*' &&
            (i == 0 || (tokens[i - 1].type != ID && tokens[i - 1].type != NUM && tokens[i - 1].type != HEX && tokens[i - 1].type != ')')))
        {
            tokens[i].type = POINT;
        }
        if (tokens[i].type == '-' &&
            (i == 0 || (tokens[i - 1].type != ID && tokens[i - 1].type != NUM && tokens[i - 1].type != HEX && tokens[i - 1].type != ')')))
        {
            tokens[i].type = NEG;
        }
    }
    return eval(0, nr_token - 1);
}
