#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"

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

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char* exp){
	assert(free_ != NULL);
	WP* temp = free_;
	free_ = free_->next;
	temp->next = NULL;
	bool success = false;
	strcpy(temp->exp, exp);
	temp->value = expr(temp->exp, &success);
	assert(!success);
	if(head == NULL){
		head = temp;
	}
	else{
		WP* p = head;
		while(p->next){
			p = p->next;
		}
		p->next = temp;
	}
	return temp;
}

void free_wp(WP* wp){
	if(wp == NULL){
		printf("EMPTY!\n");
	}
	if(wp == head){
		head = head->next;
	}
	else{
		WP* p = head;
		while(p->next != wp){
			p = p->next;
		}
		if(p == NULL){
			return;
		}
		p->next = wp->next;
	}
	wp->next = free_;
	free_ = wp;
}

int check_change(){
	WP* wp = head;
	while(wp != NULL){
		bool success = false;
		uint32_t newvalue = expr(wp->exp, &success);
		if(wp->value != newvalue){
			printf("Hint watchpoint %d at address 0x%08x, expr = %s\n", wp->NO, cpu.eip, wp->exp);
			printf("old value = 0x%06x\n", wp->value);
			printf("new value = 0x%06x\n", newvalue);
			return 0;
		}
		wp = wp->next;
	}
	return 1;
}

int delete_wp(int i){
	WP* wp = head;
	while(wp->NO != i && wp != NULL){
		wp = wp->next;
	}
	if(wp == NULL){
		return 0;
	}
	else{
		free_wp(wp);
		return 1;
	}
}

void print_wp(){
	WP* wp = head;
	if(head == NULL){
		printf("No watchpoint!\n");
		return;
	}
	while(wp != NULL){
		printf("Watchpoint %d:%s\tThe value is 0x%08x\n", wp->NO, wp->exp, wp->value);
		wp = wp->next;
	}
}





