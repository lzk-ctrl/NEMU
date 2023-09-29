#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include "memory.h"
#include <elf.h>
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

static int cmd_si(char *args) {
        int num;
        char *arg = strtok(NULL, " ");
        if(arg!=NULL) {
                sscanf(arg,"%d",&num);
                if(num<=0){
                        printf("ERROR!\n");
                        return 0;
                }
                cpu_exec(num);
        }
        else cpu_exec(1);
        return 0;
}
    
static int cmd_info(char *args) {  
        int i=0;
        char *arg = strtok(NULL, " ");
        if(*arg=='r'){
            while(i<8){
                printf("%s        0x%08x        %d\n",regsl[i],cpu.gpr[i]._32,cpu.gpr[i]._32);
                i++;
            }
            printf("eip        0x%08x        %d\n",cpu.eip,cpu.eip);
        }
        if(*arg=='w'){
            show_wp();
        }
        return 1;
}

static int cmd_x(char *args){
        int i,j;
        char *arg1 = strtok(NULL, " ");
        char *arg2 = strtok(NULL, " ");
        if(!arg1||!arg2) {
                printf("ERROR!\n");
                return 0;
        }
        sscanf(arg1,"%d",&i);
        bool suc=true;
        j=expr(arg2,&suc);
        if(i<=0||suc!=true) {
                printf("Unable to read the address.\n");
                return 0;
        }
        int flag=0;
        printf("0x%08x:  ",j);
        while(flag<i) {
                if(flag%4==0&&flag!=0) printf("\n0x%08x:  ",j+flag*4);
                printf("0x%08x  ",swaddr_read(j+flag*4,4,R_DS));
                flag++;
        }
        printf("\n");
        return 0;
}

static int cmd_p(char *args) {
    uint32_t result;
    bool suc=true;
    result=expr(args, &suc);
    if(suc==true) {
        printf("0x%08x (%d)\n",result,result);
	    return 1;
    }
    else return 0;
}

static int cmd_w(char *args) {
    WP* newwp;
    newwp=new_wp(args);
    if(newwp) printf("Set watchpoint %d: %s  value = 0x%08x\n",newwp->NO,newwp->token,newwp->exp_value);   
	return 1;
}

static int cmd_d(char *args) {
    int num;
    bool suc;
    sscanf(args,"%d",&num);
    suc=free_wp(num);
    if(suc) printf("Delete watchpoint %d\n",num);
	return 1;
}

void get_function(swaddr_t,char [], bool*);

void print_chain(swaddr_t now_addr) {
	int cnt=0;
	bool success=true;
	char func_name[32]={};
	swaddr_t edp_a=reg_l(R_EBP);
	swaddr_t ret_a=now_addr;
	while(edp_a!=0) {
		get_function(ret_a,func_name,&success);
		if(success) {
	//		if(cnt==1) printf("the current address 0x%08x is in function %s.\n",now_addr,func_name);
			printf("#%d 0x%08x in %s ",cnt,ret_a,func_name);
			printf("(0x%08x 0x%08x 0x%08x 0x%08x)\n",
           swaddr_read(edp_a+8,4,R_SS),swaddr_read(edp_a+12,4,R_SS),swaddr_read(edp_a+16,4,R_SS),swaddr_read(edp_a+20,4,R_SS));
		}
		else break;
		cnt++;
	    ret_a=swaddr_read(edp_a+4,4,R_SS);
		edp_a=swaddr_read(edp_a,4,R_SS);
	//	printf("edp_a: 0x%08x  ret_a: 0x%08x\n",edp_a,ret_a);
	}
	return;
}

static int cmd_bt(char* args) {
	swaddr_t now_addr=cpu.eip;
	print_chain(now_addr);
	return 1;
					
}

static int cmd_page(char* args) {
	uint32_t addr;
	sscanf(args,"%x",&addr);
	printf("0x%08x\n",page_translate(addr));
	return 1;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
   	{ "si", "Repeat the execution of the program for N", cmd_si },
  	{ "info", "Print the status of the registers or watch point", cmd_info},
   	{ "x", "Print the value at the given address", cmd_x},
	{ "p", "Evaluate the value of the expression", cmd_p},
	{ "w", "Set the watch point", cmd_w},
	{ "d", "Delete the watch point", cmd_d},
	{ "bt", "Print the chain of stack", cmd_bt},
	{ "page", "Translate the given address into page mode", cmd_page}
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
