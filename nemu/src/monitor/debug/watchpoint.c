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

WP* new_wp(char *exp){
	assert(free_ != NULL);
        WP *ans = free_;
	WP *anss= head;
	free_ = free_ -> next;
        ans -> next = NULL;
	
	if(anss==NULL)
	head=ans;
	else{
		
		while(anss->next!=NULL)
			anss=anss->next;
		anss->next=ans;
		
	}
        return ans;
}

/*int free_wp(WP *wp){
	if(wp==NULL)
		printf("the input have something wrong");
	if(wp==head)
		head=head->next;
	else{
		WP *p =head;
		while (p->next!=wp)
			p=p->next;
		
	}
        wp -> next = free_;
        free_ = wp;
	
        return 1;
}
*/

void free_wp(int k){
	WP *p=head;
	if(head==NULL){
		printf("The list of monitoring points is empty\n");
		
	}
	else if(p->NO==k){
		head=head->next;
		p->value=0;
		p->next=free_;
		free_=p;
		printf("You have delete the NO.%d watchpoint\n",k);
		return;
	}
	else{
		WP *q=head;
		p=p->next;
		while(p!=NULL){
			if(p->NO==k){
				q->next=p->next;
				p->value=0;
				p->next=free_;
				free_=p;
				printf("You have delete the NO.%d watchpoint\n",k);
				return;
			}
			else{
				p=p->next;
				q=q->next;
			}
		}
	}
	return;
}

int print_wp(){
	WP *p=head;
	if (p==NULL){
		printf("There is no watchpoint here\n");
		
	}
	else{
		while(p!=NULL){
			printf("NO.%d  \t%s  \t0x%08x\n",p->NO,p->exp,p->value);
			p=p->next;
		}
		
	}
	return 0;
}



bool test_change(uint32_t last_eip){
	WP *p;
	p=head;
	bool key= true;
	bool success=1;
	while (p!=NULL){
		int ans_expr = 1;
		ans_expr=expr(p->exp,&success);

		//if(!success)  assert(1);
		printf("ans_expr==%x \t,culc=%x, ,p->expr==%s, p->val==%x \n",ans_expr,expr(p->exp,&success),p->exp,p->value);
		if(ans_expr != p->value){
			key =false;
			printf("Hint watchpoint %d at address 0x%08x\n",p->NO,last_eip);
			printf("expr = %s\n",p->exp);
			printf("Old value: \t%d\nNew value: \t%d\n",p->value,ans_expr);

			p->value=ans_expr;
		}
		p=p->next;
	}
	return key;
}



/* TODO: Implement the functionality of watchpoint */


