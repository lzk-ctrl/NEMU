#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;

	char exp[64];

        int value;
} WP;

bool test_change();
int print_wp();
void free_wp();
WP* new_wp();
#endif
