#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "cpu/reg.h"
#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool()
{
	int i;
	for (i = 0; i < NR_WP; i++)
	{
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
		wp_pool[i].val = 0;
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}
/* TODO: Implement the functionality of watchpoint */

void print_wp()
{
	WP *p;
	p = head;
	while (p != NULL)
	{
		printf("watchpoint%d:\"%s\" default as %d\n", p->NO, p->EXPR, p->val);
		p = p->next;
	}
}
WP *new_wp(char *e, bool *success)
{
	WP *p, *q;
	if (free_ == NULL)
	{
		return NULL;
	}
	if (head == NULL)
	{
		p = free_;
		free_ = free_->next;
		head = p;
		head->next = NULL;
		head->val = expr(e, success);
		strcpy(head->EXPR, e);
	}
	else
	{
		p = head;
		while (p != NULL)
		{
			q = p;
			p = p->next;
		}
		p = free_;
		free_ = free_->next;
		p->next = NULL;
		p->val = expr(e, success);
		strcpy(p->EXPR, e);
		q->next = p;
	}
	return p;
}

WP *find_wp(char *EXPR)
{
	WP *p = head;
	int n;
	sscanf(EXPR, "%d", &n);
	while (p != NULL)
	{
		if (n == p->NO)
			return p;
		p = p->next;
	}
	return p;
}
void free_wp(WP *wp)
{
	WP *p, *q;
	if (head->NO == wp->NO)
	{
		p = head;
		head = head->next;
	}
	else
	{
		p = head;
		while (p != NULL)
		{
			q = p;
			p = p->next;
			if (p->NO == wp->NO)
			{
				q->next = p->next;
				break;
			}
		}
	}
	if (free_ == NULL)
	{
		free_ = p;
		free_->next = NULL;
	}
	else
	{
		q = free_;
		free_ = p;
		free_->next = q;
	}
}

bool check_wp()
{
	bool ret = false;
	bool success;
	WP *p;
	p = head;
	int n;
	while (p != NULL)
	{
		n = expr(p->EXPR, &success);
		if (n != p->val)
		{
			printf("Hint watchpoint %d at address %x\n", p->NO, cpu.eip);
			ret = true;
		}
		p = p->next;
	}
	return ret;
}