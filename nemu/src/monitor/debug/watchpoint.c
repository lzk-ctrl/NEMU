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
	free_ = &wp_pool[0];
}
void free_wp(WP *wp){
	WP *f;
	f=head;
	if(head==NULL)Assert(0,"No monitoring point to free");
	if(wp->NO==head->NO)head=head->next;
	else{
		while(f->next!=NULL&&f->next->NO!=wp->NO)f=f->next;
		if(f->next==NULL)Assert(0,"no this monitoring point");
		if(f->next->NO==wp->NO)f->next=f->next->next;
		else Assert(0,"something wrong");
	}
	/*wp->next=NULL;
	wp->value=0;
	wp->str[0]='\0';*/
	
}
WP* new_wp(){
	WP *f,*p;//
	f=free_;
	if(f==NULL)Assert(0,"Too many monitoring points");
	free_=free_->next;
	f->next=NULL;
	p=head;
	if(p==NULL){
		p=f;
		head=f;	
	}
	else{
		while(p->next!=NULL)p=p->next;
		p->next=f;	
	}
	return f;
	
}
bool check_wp(){
	WP *f;
	bool success;
	bool key=true;
	f=head;
	while(f!=NULL)
	{
		uint32_t num=expr(f->str,&success);
		if(!success)Assert(0,"make_token false");
		if(f->value!=num){
			key=false;
			printf("Hint watchpoint %d at address  0x%08x, ",f->NO,cpu.eip);
			printf("expr = %s\n",f->str);
			printf("Old value = %08x\n",f->value);
			printf("New value = %08x\n",num);
			f->value=num;
		}
		f=f->next;
	}
	return key;
}
void info_wp(){
	WP *f;
	f=head;
	while(f!=NULL){
		printf("watchpoint %d : %s = %d\n",f->NO,f->str,f->value);
		f=f->next;		
	}
}
void delete_wp(int i){
	WP *p;
	p=&wp_pool[i];
	free_wp(p);
	}
/* TODO: Implement the functionality of watchpoint */


