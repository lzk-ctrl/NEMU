#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
WP* new_wp()
{	
	WP* p=head;
	if(!p)	
	{
		head=free_;
		free_=free_->next;
		head->next=NULL;
		return head;
	}
	while(p->next!=NULL)
	{
		p=p->next;
	}
	p->next=free_;
	free_=free_->next;
	p=p->next;
	p->next=NULL;
	return p;
}
void free_wp(WP *wp)
{
	WP* p=head;
	if(wp==head)	head=head->next;
	else
	{
		while(p->next!=wp)
		{
			p=p->next;
		}
		p->next=wp->next;
	}
	wp->next=free_;
	free_=wp;
	return ;
}
bool check_watchpoint()
{
	WP* p=head;
	bool res=true;
	while(p)
	{
		bool success=false;
		printf("%s",p->exprs);
		if(p->old_value!=expr(p->exprs,&success))
		{
			printf("Hint watchpoint %d at address %#010x\texpr %s\n",p->NO,cpu.eip,p->exprs);
			printf("old value = %#010x\tnew value = %#010x\n",p->old_value,expr(p->exprs,&success));
			res=false;
		}
		p=p->next;
	}
	return res;
}
bool delete_watchpoint(int no)
{
	WP* p=head;
	WP* pre=head;
	if(p==NULL)	return false;
	else 
	{
		if(p->NO==no)
		{
			head=head->next;
			p->next=free_;
			free_=p;
			return true;
		}
		else
		{
			p=head->next;
			while(p->NO!=no)
			{
				p=p->next;
				pre=pre->next;
			}
		}
	}
	pre->next=p->next;
	p->next=free_;
	return true;
}
void show_watchpoint()
{
	WP* p;
	p=head;
	while(p!=NULL)
	{
		printf("Watchponit %d\t%s\t%#010x\n",p->NO,p->exprs,p->old_value);
		p=p->next;
	}
}
void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP-1; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP-1].NO=NR_WP-1;
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */


