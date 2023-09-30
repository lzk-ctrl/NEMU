#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint
{
	int NO;
	struct watchpoint *next;
	char EXPR[32];
	int val;
	/* TODO: Add more members if necessary */

} WP;
WP *new_wp(char *, bool *);
WP *find_wp(char *);
void free_wp(WP *);
bool check_wp();
void print_wp();
#endif
