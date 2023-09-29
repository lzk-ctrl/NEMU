#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
        char token[32];
        uint32_t exp_value;
	/* TODO: Add more members if necessary */


} WP;

WP* new_wp(char *);
bool free_wp(int); 
bool check_wp(int);
void show_wp();


#endif
