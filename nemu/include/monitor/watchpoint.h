#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	char exp[32];
	uint32_t value;
	/* TODO: Add more members if necessary */


} WP;

WP* new_wp();
void free_wp(WP* wp);
int check_change();
int delete_wp(int i);
void print_wp();

#endif
