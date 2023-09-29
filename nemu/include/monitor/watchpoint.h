#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	uint32_t value;
	char str[32];
	struct watchpoint *next;

	/* TODO: Add more members if necessary */


} WP;

WP *new_wp();
void free_wp(WP *);
void delete_wp(int i);
void info_wp();
bool check_wp();
#endif
