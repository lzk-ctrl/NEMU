#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);
WP* new_wp();
void free_wp(WP *wp);
void show_watchpoint();
bool delete_watchpoint(int no);
/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args)
{
	char *arg = strtok(NULL, " ");
	if(arg==NULL)//单步执行
	{
		cpu_exec(1);
	}
	else
	{
		if(*arg<'0'||*arg>'9')
		{
			printf("输入格式不对");
			return 0;
		}
		int n=atoi(arg);
		cpu_exec(n);
	}
	return 0;
}

static int cmd_info(char *args)
{
	char *arg = strtok(NULL, " ");
	if(arg==NULL)
	{
		printf("Unknown command \n");	
	}
	else if(*arg=='r')
	{
		int i;
		for( i=0;i<8;i++)
		{
			printf("%s\t%#010x\t%d\n",regsl[i],cpu.gpr[i]._32,cpu.gpr[i]._32);
		}
		printf("eip\t%#010x\t%d\n",cpu.eip,cpu.eip);
	}
	else if(*arg=='w')
	{
		show_watchpoint();
	}
	return 0;
}
static int cmd_x(char *args)
{
	int i=0;
	if(args)
	{
		char* num=strtok(NULL," ");
		char* exprs=strtok(NULL,".");
		int n=atoi(num);
		bool success=false;
		int address=expr(exprs,&success);
		for(i=0;i<n;i++)
		{
			printf("%#010x\t",swaddr_read(address + i * 4,4));
			if(i%4==3&&i!=n)	
				printf("\n");
		}
		printf("\n");
		return 0;
	}
	printf("Unknown command\n");
	return	0;
}
static int cmd_p(char* args)
{
	bool success=false;
	int ans=expr(args,&success);
	printf("%d\n",ans);
	return 0;
}
static int cmd_w(char* args)
{
	WP* wp=new_wp();
	if(wp==NULL)
	{
		return 0;
	}
	printf("Set watchpoint #%d",wp->NO);
	strcpy(wp->exprs,args);
	bool success=false;
	wp->old_value=expr(args,&success);
	return 0;
}
static int cmd_d(char* args)
{
	char* arg=strtok(NULL," ");
	int no=atoi(arg);
	if(!delete_watchpoint(no))
		printf("Watchpoint #%d does not exist\n",no);
	return 0;
}
static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "单步执行", cmd_si },
	{ "info", "-r 打印寄存器状态  -w 打印监视点状态", cmd_info},
	{ "x", "-n -expr 求出expr的值，并将其结果作为内存起始地址输出后面连续的n个四字节", cmd_x},
	{"p","-expr 求出表达式 EXPR 的值",cmd_p},
	{"w","-expr 当表达式expr的值发生变化时，暂停程序",cmd_w},
	{"d","-NO 删除序号为NO监视点",cmd_d},
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
