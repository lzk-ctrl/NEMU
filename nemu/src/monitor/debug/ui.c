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

static int cmd_help(char *args);

static int cmd_si(char *args)
{
	int step = 0;
	char *pi = args;
	if (pi == NULL)
	{
		cpu_exec(1);
		return 0;
	}
	sscanf(args, "%d", &step);
	if (step <= 0)
	{
		printf("ERROR!\n");
		return 0;
	}
	cpu_exec(step);
	return 0;
}

static int cmd_info(char *args)
{
	int i;
	if (strcmp(args, "r") == 0)
	{
		for (i = 0; i < 8; i++)
		{
			printf("%s\t 0x%08x\t %d\n", regsl[i], cpu.gpr[i]._32, cpu.gpr[i]._32);
		}
	}
	if (strcmp(args, "w") == 0)
	{
		print_wp();
	}
	return 0;
}

static int cmd_x(char *args)
{
	int i, N;
	char *num = strtok(NULL, " ");
	char *EXPR = strtok(NULL, "\n");
	bool success = false;
	swaddr_t addr;
	sscanf(num, "%d", &N);
	addr = expr(EXPR, &success);
	for (i = 1; i <= N; i++)
	{
		if (i % 4 == 1)
			printf("0x%08x:", addr);
		printf(" %08x", swaddr_read(addr, 4, R_DS));
		addr += 4;
		if (i % 4 == 0)
			printf("\n");
	}
	if (N / 4 != 0 || N < 4)
		printf("\n");
	return 0;
}

static int cmd_p(char *args)
{
	bool success;
	int res;
	res = expr(args, &success);
	printf("(0x%x) %d\n", res, res);
	// else printf("failed!\n");
	return 0;
}

static int cmd_w(char *args)
{
	bool success = false;
	new_wp(args, &success);
	return 0;
}

static int cmd_d(char *args)
{
	WP *p;
	p = find_wp(args);
	free_wp(p);
	return 0;
}

typedef struct
{
	swaddr_t prev_ebp;
	swaddr_t ret_addr;
	uint32_t args[4];
} PartOfStackFrame;

/*static int cmd_bt(char *args){
	return 0;
	PartOfStackFrame p;
	p.prev_ebp = regl(R_EBP);
	while(swaddr_read(p.prev_ebp, 4) != 0){
		//printf("%x\t%x\t%")
	}
	return 0;
}
*/
static struct
{
	char *name;
	char *description;
	int (*handler)(char *);
} cmd_table[] = {
	{"help", "Display informations about all supported commands", cmd_help},
	{"c", "Continue the execution of the program", cmd_c},
	{"q", "Exit NEMU", cmd_q},
	{"si", "Single move", cmd_si},
	{"info", "Print the vaule of reg", cmd_info},
	{"x", "Scan memory", cmd_x},
	{"p", "Evaluate the expression", cmd_p},
	{"w", "Add a checkpoint", cmd_w},
	{"d", "Delete a checkpoint", cmd_d}
	//{ "bt", "Print stack frame chain", cmd_bt}
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
