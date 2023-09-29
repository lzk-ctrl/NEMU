#include "common.h"
#include "memory/cache.h"
#include "burst.h"
#include <time.h>
#include <stdlib.h>

void cache_read_replace(hwaddr_t, void*);
void cache_write_replace(hwaddr_t, void*, uint8_t*);
void dram_write(hwaddr_t, size_t, uint32_t);

void init_cache() {
	int i;
	for(i=0;i<L1_cache_S*L1_cache_E;i++) {
		cache_L1[i].valid=false;
	}
	for(i=0;i<L2_cache_S*L2_cache_E;i++) { 
		cache_L2[i].valid=false;
		cache_L2[i].dirty=false;
	}
	return;
}

int read_L1_cache(hwaddr_t aim) {
	uint32_t tag=aim>>(L1_cache_s_bits+cache_b_bits);
	uint32_t group=(aim>>cache_b_bits)&(L1_cache_S-1);
	//uint32_t es_bias=(aim>>L1_cache_b_bits)<<L1_cache_b_bits;
	int i;
	//printf("aim: %08x    tag: %u   group: %u\n",aim,tag,group);
	for(i=group*L1_cache_E;i<L1_cache_E+group*L1_cache_E;i++) {
		if(cache_L1[i].valid==0) continue;
		if(cache_L1[i].tag==tag) 
			return i;
	}
	for(i=group*L1_cache_E;i<L1_cache_E+group*L1_cache_E;i++) {
		if(cache_L1[i].valid==0) break;
	}
	if(i==L1_cache_E+group*L1_cache_E) {
		srand(time(0));
		i=rand()%L1_cache_E+group*L1_cache_E;
	}
	int temp=read_L2_cache(aim);
	memcpy(cache_L1[i].data,cache_L2[temp].data,cache_B);
	//uint32_t j;
	//for(j=0;j<L1_cache_B/BURST_LEN;j++) {
	//	cache_read_replace(es_bias+BURST_LEN*j,L1_cache[group][i].data+BURST_LEN*j);
	//}
	//printf("NO HIT group: %u   i: %u   tag: %u\n",group,i,tag);
	cache_L1[i].valid=true;
	cache_L1[i].tag=tag;
	return i;
}

int read_L2_cache(hwaddr_t aim) {
	uint32_t tag=aim>>(L2_cache_s_bits+cache_b_bits);
	uint32_t group=(aim>>cache_b_bits)&(L2_cache_S-1);
	uint32_t es_bias=(aim>>cache_b_bits)<<cache_b_bits;
	int i;
	//printf("aim: %08x    tag: %u   group: %u\n",aim,tag,group);
	for(i=group*L2_cache_E;i<L2_cache_E+group*L2_cache_E;i++) {
		if(cache_L2[i].valid==0) continue;
		if(cache_L2[i].tag==tag) 
			return i;
	}
	for(i=group*L2_cache_E;i<L2_cache_E+group*L2_cache_E;i++) {
		if(cache_L2[i].valid==0) break;
	}
	if(i==L2_cache_E+group*L2_cache_E) {
		srand(time(0));
		i=rand()%L2_cache_E+group*L2_cache_E;
	}
	if(cache_L2[i].valid==1 && cache_L2[i].dirty==1) {
		uint8_t temp[2*BURST_LEN];
		uint32_t es_bias_change=(cache_L2[i].tag<<(L2_cache_s_bits+cache_b_bits)) | (group<<cache_b_bits);
		memset(temp,1,sizeof(temp));
		int h;
		for(h=0;h<cache_B/BURST_LEN;h++) {
			cache_write_replace(es_bias_change+BURST_LEN*h,cache_L2[i].data+BURST_LEN*h,temp);
		}
	}
	int j;
	for(j=0;j<cache_B/BURST_LEN;j++) {
		cache_read_replace(es_bias+BURST_LEN*j,cache_L2[i].data+BURST_LEN*j);
	}
	//printf("NO HIT group: %u   i: %u   tag: %u\n",group,i,tag);
	cache_L2[i].valid=true;
	cache_L2[i].dirty=false;
	cache_L2[i].tag=tag;
	return i;
}

void write_L1_cache(hwaddr_t aim,size_t len,uint32_t data) {
	uint32_t tag=aim>>(L1_cache_s_bits+cache_b_bits);
	uint32_t group=(aim>>cache_b_bits)&(L1_cache_S-1);
	uint32_t bias=aim&(cache_B-1);
	//printf("aim: %08x    tag: %u   group: %u\n",aim,tag,group);
	int i;
	for(i=group*L1_cache_E;i<L1_cache_E+group*L1_cache_E;i++) {
		if(cache_L1[i].valid==0) continue;
		if(cache_L1[i].tag==tag) {
			dram_write(aim,len,data);
			write_L2_cache(aim,len,data);
			if(bias+len<=cache_B){
				memcpy(cache_L1[i].data+bias,&data,len);
			}
			else {
				memcpy(cache_L1[i].data+bias,&data,cache_B-bias);
				write_L1_cache(aim+cache_B-bias,len-(cache_B-bias),data>>(cache_B-bias));
			}
			return;	
		}
	}
	write_L2_cache(aim,len,data);
	return;
}

void write_L2_cache(hwaddr_t aim,size_t len,uint32_t data) {
	uint32_t tag=aim>>(L2_cache_s_bits+cache_b_bits);
	uint32_t group=(aim>>cache_b_bits)&(L2_cache_S-1);
	uint32_t bias=aim&(cache_B-1);
	int i;
	for(i=group*L2_cache_E;i<L2_cache_E+group*L2_cache_E;i++) {
		if(cache_L2[i].valid==0) continue;
		if(cache_L2[i].tag==tag) {
			cache_L2[i].dirty=1;
			if(bias+len<=cache_B)
				memcpy(cache_L2[i].data+bias,&data,len);
			else {
				memcpy(cache_L2[i].data+bias,&data,cache_B-bias);
				write_L2_cache(aim+cache_B-bias,len-(cache_B-bias),data>>(cache_B-bias));
			}	
			return;
		}
	}
	int ans=read_L2_cache(aim);
	cache_L2[ans].dirty=1;
	memcpy(cache_L2[ans].data+bias,&data,len);
	return;
}

