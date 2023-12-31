#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char *rl_gets()
{
	static char *line_read = NULL;

	if (line_read)
	{
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read)
	{
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args)
{
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args)
{
	return -1;
}

static int cmd_si(char *args)
{
	int n;
	sscanf(args, "%d", &n);
	cpu_exec(n);
	return 0;
}
char regname[8][4] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
static int cmd_info(char *args)
{
	if (strcmp(args, "r") == 0)
	{
		for (int i = 0; i < 4; i++)
			printf("%s\t%#010x\t%d\n", regname[i], cpu.gpr[i]._32, cpu.gpr[i]._32);
	}
	if (strcmp(args, "w") == 0)
	{
		info_wp();
		return 0;
	}
	return 0;
}
static int cmd_x(char *args)
{
	char *arg = strtok(NULL, " ");
	int n;
	sscanf(arg, "%d", &n);
	bool s;
	uint32_t addrr = expr(arg + strlen(arg) + 1, &s);
	for (int i = 0; i < n; i++)
	{
		if (i % 4 == 0)
			printf("%#010x:", addrr);
		printf(" %#010x", swaddr_read(addrr, 4));
		addrr += 4;
		if (i % 4 == 3 || i == n - 1)
			printf("\n");
	}
	return 0;
}

static int cmd_p(char *args)
{
	bool success;

	if (args)
	{
		uint32_t r = expr(args, &success);
		if (success)
		{
			printf("0x%08x\n", r);
		}
		else
		{
			printf("Bad expression\n");
		}
	}
	return 0;
}
static int cmd_w(char *args)
{
	if (args == NULL)
		return puts("Wrong args!!"), 1;
	WP *f;
	bool suc;
	f = new_wp();
	f->val = expr(args, &suc);
	if (!suc)
	{
		return puts("Wrong args!!"), 1;
	}
	strcpy(f->expr, args);
	printf("Watchpoint created: Watchpoint %d: %s = 0x%x\n", f->NO, f->expr, f->val);
	return 0;
}
static int cmd_d(char *args)
{
	if (args == NULL)
		return puts("Wrong args!!"), 1;
	delete_wp(atoi(args));
	return 0;
}

static int cmd_help(char *args);

static struct
{
	char *name;
	char *description;
	int (*handler)(char *);
} cmd_table[] = {
	{"help", "Display informations about all supported commands", cmd_help},
	{"c", "Continue the execution of the program", cmd_c},
	{"q", "Exit NEMU", cmd_q},
	{"si", "single take", cmd_si},
	{"info", "print reg", cmd_info},
	{"x", "print memory", cmd_x},
	{"p", "calculate", cmd_p},
	{"d", "watchpoint", cmd_d},
	{"w","ww",cmd_w}

	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args)
{
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if (arg == NULL)
	{
		/* no argument given */
		for (i = 0; i < NR_CMD; i++)
		{
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else
	{
		for (i = 0; i < NR_CMD; i++)
		{
			if (strcmp(arg, cmd_table[i].name) == 0)
			{
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop()
{
	while (1)
	{
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if (cmd == NULL)
		{
			continue;
		}

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if (args >= str_end)
		{
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for (i = 0; i < NR_CMD; i++)
		{
			if (strcmp(cmd, cmd_table[i].name) == 0)
			{
				if (cmd_table[i].handler(args) < 0)
				{
					return;
				}
				break;
			}
		}

		if (i == NR_CMD)
		{
			printf("Unknown command '%s'\n", cmd);
		}
	}
}