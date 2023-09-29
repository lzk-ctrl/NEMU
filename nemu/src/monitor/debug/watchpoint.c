#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

WP* new_wp(char *e){
    if(free_==NULL) {
        printf("full watchpoints!\n");
        return NULL;
    }
    WP *p=free_;
    bool suc=true;
    p->exp_value=expr(e, &suc);
    if(suc!=true) {
        printf("Do not set the watchpoint.\n");
        return NULL;
    }
    strcpy(p->token,e);
    free_=p->next;
    p->next=NULL;
    if(head==NULL) head=p;
    else {
        WP* temp=head;
        while(temp->next) temp=temp->next;
        temp->next=p;
    }  
    return p;
}

bool free_wp(int num) {
    if(num<0||num>=NR_WP||head==NULL) {
        printf("Not find this watchpoint!\n");
        return 0;
    }
    WP* temp=head;
    WP* r=head;
    if(num==r->NO) {
        head=temp->next;
        r->next=free_;
        strcpy(r->token," ");
        r->exp_value=0;
        free_=r;
        return 1;
    }
    else {
        while(temp->next) {
            r=temp->next;
            if(num==r->NO) {
                temp->next=r->next;
                r->next=free_;
                strcpy(r->token," ");
                r->exp_value=0;
                free_=r;
                return 1;
            }
            temp=temp->next;
        }
    }
    printf("Not find this watchpoint!\n");
    return 0;
}

bool check_wp(int cpu_eip) {                                                                      
    bool is_change=false;
    if(head) {
	    WP* temp=head;
		while(temp) {
            bool suc=true;
            int tem_val;
            tem_val=expr(temp->token, &suc);
            if(suc!=true) {
                printf("the expression of watchpoint %d change.\n",temp->NO);
                return 1;
            }
            else {
                if(tem_val!=temp->exp_value) {
                    printf("Hint watchpoint %d at address 0x%08x, expr = %s\n",temp->NO,cpu_eip, temp->token);
                    printf("old value = 0x%08x\n",temp->exp_value);
                    printf("new value = 0x%08x\n",tem_val);
                    temp->exp_value=tem_val;
                    is_change=true;
                }
            }
            temp=temp->next;
        }
    }
    return is_change;
}

void show_wp() {
    if(!head) {
        printf("No watchpoint!\n");
        return;
    }
    WP *temp=head;
    while(temp) {
        printf("watchpoint %d: expr %s   value = 0x%08x\n",temp->NO,temp->token,temp->exp_value);
        temp=temp->next;
    }
    return;
}

/* TODO: Implement the functionality of watchpoint */


