#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <elf.h>

void cpu_exec(uint32_t);

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

static int cmd_si(char *args) {
	
	if (!args) 
	{
		cpu_exec(1);
		return 0;
	}
	else
	{
	cpu_exec(atoi(args));
	return 0;
	}
}


static int cmd_info(char *args){
	if (args[0]=='r')
	{
	printf("eax 0x%08x %8d\n",cpu.eax,cpu.eax);
	printf("ecx 0x%08x %8d\n",cpu.ecx,cpu.ecx);
	printf("edx 0x%08x %8d\n",cpu.edx,cpu.edx);
	printf("ebx 0x%08x %8d\n",cpu.ebx,cpu.ebx);
	printf("esp 0x%08x %8d\n",cpu.esp,cpu.esp);
	printf("ebp 0x%08x %8d\n",cpu.ebp,cpu.ebp);
	printf("esi 0x%08x %8d\n",cpu.esi,cpu.esi);
	printf("edi 0x%08x %8d\n",cpu.edi,cpu.edi);
	/*for(int i=0;i<8;i++)
	printf("%s 0x%x %d\n",regsl[i],gpr[i]._32,gpr[i]._32);*/	
	}
	if (args[0]=='w')
	{
		info_wp();
	}
	return 0;
}

static int cmd_x(char *args){
	char *N=strtok(NULL, " ");
	char *EXPR = strtok(NULL, " ");
	int address;
	int len;
	sscanf(EXPR,"%x",&address);
	sscanf(N,"%d", &len);
	int x=len/4;
	int i;
	for(i=0;i<x;i++)
	{
		printf("0x%08x: ",address);
		int z=4;
		while(z--)
		{
			printf("0x%08x ",swaddr_read(address,4));
			address+=4;
		}
		printf("\n");
	}
	printf("0x%08x: ",address);
	int y=len%4;
	while(y--)
	{
		printf("0x%08x ",swaddr_read(address,4));
		address+=4;
	}
	printf("\n");
	return 0;
}

static int cmd_w(char *args){
		WP *f;
		bool success;
		f=new_wp();
		printf("watchpoint %d: %s\n",f->NO,args);
		f->value=expr(args,&success);
		strcpy(f->str,args);
		if(!success)Assert(0,"make_token false");
		printf("value : %d\n",f->value);
		return 0;
	}
static int cmd_d(char *args){
	int i;
	sscanf(args,"%d",&i);
	delete_wp(i);
	return 0;
}
static int cmd_p(char *args){
	uint32_t num;
	bool success;
	num=expr(args,&success);
	if(success)printf("0x%x\t%d\n",num,num);
	else Assert(0,"Wrong");
	return 0;
}
void GetFunctionAddr(swaddr_t cur_addr,char* name);
typedef struct {
	swaddr_t prev_ebp;
	swaddr_t ret_addr;
	uint32_t args[4];
}PartOfStackFrame;

static int cmd_bt(char* args){
	PartOfStackFrame EBP;
	char name[32];
	EBP.ret_addr = cpu.eip;
	swaddr_t addr = cpu.ebp;
	//printf("%x\n",addr);
	int i = 0;
	while(addr){
		printf("#%d\t",i ++);
		GetFunctionAddr(EBP.ret_addr,name);
		printf("%s\t",name);
		EBP.prev_ebp = swaddr_read(addr,4);
		EBP.ret_addr = swaddr_read(addr + 4, 4);
		int j;
		for(j = 0;j < 4 ;j ++){
			EBP.args[j] = swaddr_read(addr + 8 + 4 * j, 4);
		}
		printf("0x%x\t",EBP.ret_addr);
		for(j = 0; j < 4; j ++){
			printf("0x%x",EBP.args[j]);
			if(j < 3) printf(",");
			else printf("\n");
		}
		addr = EBP.prev_ebp;

	}
	
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
	{ "si", "Execute N times according to the parameter N", cmd_si},
	{ "info", "Parameter r prints register status, parameter w prints monitoring point information", cmd_info},
	{ "x","Use the given address as the starting memory address to output N consecutive 4 bytes", cmd_x},
	{ "p","Find the value of the expression EXPR", cmd_p},
	{ "w","Pause the program when the value of the expression expr changes", cmd_w},
	{ "d","Delete the designated serial number monitoring point", cmd_d},
	{ "bt","Print stack frame chain",cmd_bt},
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
