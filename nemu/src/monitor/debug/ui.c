#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include "memory/cache.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

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

static int cmd_si(char *args){
        uint32_t bushu=0;
        if(args == NULL) bushu=1;
        else sscanf(args, "%d",&bushu);
	Log("%d",bushu);
        cpu_exec(bushu);
        return 0;
}


static int cmd_info(char *args){
        if(args[0] == 'r'){
            int i;
            for(i=0;i<8;i++){
               printf("%s \t0x%08x \t%d\n",regsl[i],reg_l(i),reg_l(i));
            }
            printf("eip \t0x%08x \t%d\n",cpu.eip,cpu.eip);
        }

	if(args[0] == 'w'){
		print_wp();
	}
        return 0;
}

static int cmd_x(char *args){
        if (args == NULL)
	{
		printf("Invalid parameter.\n");
		return 0;
	}
	uint32_t n, adr, i;
	int flag = sscanf(args, "%u%x", &n, &adr);
	if (flag != 2)
	{
		printf("Invalid parameter.\n");
		return 0;
	}

	for (i = 0; i < n; i++)
	{
		if (i % 4 == 0)
		{
			if (i != 0)
				printf("\n");
			printf("0x%08x: ", adr + 4u * i);
		}
		printf("0x%08x ", swaddr_read(adr + 4u * i, 4,R_DS));
	}
	printf("\n");
	return 0;
}

static int cmd_p(char *args){
        uint32_t num;
        bool success=true;
        num = expr (args,&success);
        if (success)
            printf("0x%08x(%d)\n",num,num);
        else printf("Bad expression.\n");
        return 0;
}

static int cmd_d(char *args){
	char *arg=strtok(args," ");
	int num;
	sscanf(arg,"%d",&num);
	free_wp(num);
	return 0; 
}

static int cmd_w(char *args){
	
	bool success=true;
	int res=expr(args,&success);
	if(success)
	{
		WP *n_wp=new_wp();
		strcpy(n_wp->exp,args);
		n_wp->value=res;
		printf("watchpoints NO.%d: %s have been set\n" ,n_wp->NO,n_wp->exp);
	}
	else
	{
		printf("invalid expression.\n");
	}
	return 0;
}

char* findFunctionName(uint32_t eip);
static int cmd_bt(char *args){

	uint32_t cnt = 0;
	uint32_t arg[4];
	uint32_t ebp=cpu.ebp;
	uint32_t eip=cpu.eip;

	while(ebp){

		arg[0]=swaddr_read(ebp+8,4,R_SS);
		arg[1]=swaddr_read(ebp+12,4,R_SS);
		arg[2]=swaddr_read(ebp+16,4,R_SS);
		arg[3]=swaddr_read(ebp+20,4,R_SS);

		printf("#%d 0x%08x in %s (0x%08x 0x%08x 0x%08x 0x%08x)\n",cnt,eip,findFunctionName(eip),
		arg[0],arg[1],arg[2],arg[3]);
		eip=swaddr_read(ebp+4,4,R_SS);
		ebp=swaddr_read(ebp,4,R_SS);
		cnt++;
	}
	return 0;

}

/*static int cmd_bt(char *args)
{
	//zhe shi xuan zuo yi dan wo bu que ding shi fou zheng que jiu mei bao 
	uint32_t cnt = 0;
	uint32_t ebp=cpu.ebp;
	uint32_t eip=cpu.eip;
	while(ebp)
	{
		printf("#%d 0x%08x in %s (0x%08x 0x%08x 0x%08x 0x%08x)\n",cnt,eip,findFunctionName(eip),
		swaddr_read(ebp+8,4),swaddr_read(ebp+12,4),swaddr_read(ebp+16,4),swaddr_read(ebp+20,4));
		eip=swaddr_read(ebp+4,4);
		ebp=swaddr_read(ebp,4);
		cnt++;
	}
	return 0;

}
*/
static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
        {"si","Single Step",cmd_si },
        {"info","Print register or Print monitering points ",cmd_info },
        {"x","Scanning memory",cmd_x },
        {"p","Zhi xing ji suan",cmd_p },
	{"d","Delete monitoring points",cmd_d },
	{"w","Add monitoring points",cmd_w },
	{"bt"," ",cmd_bt},


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
