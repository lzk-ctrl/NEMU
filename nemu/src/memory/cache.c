#include "common.h"
#include "memory/cache.h"
#include <stdlib.h>
#define gr(group,group_index,offset)(this->cache_blocks+(group*this->associate + group_index)*this->block_size+offset)

uint64_t time_m=0;
uint32_t dram_read(hwaddr_t,size_t);
void dram_write(hwaddr_t,size_t,uint32_t);

static uint32_t mask(int len){
	if(len==1)  return 0xff;
	else if(len==2) return 0xffff;
	else return 0xffffffff;
}

uint32_t read_cache(CACHE *this,uint32_t addr,int len){
	uint32_t block_offset = addr % this ->block_size;
	uint32_t block = addr/this->block_size;
	uint32_t group = block % (this->size / this->block_size / this->associate);
	uint32_t tag = block / (this->size / this->block_size / this->associate);
	int i;
	if (block_offset + len > this->block_size){
		uint32_t res=0;
		
		for(i=len-1;i>=0;--i){
			uint32_t ret=read_cache(this,addr+i,1);
			res=(res<<8)|ret;
		}
		return res;
	}

	for(i=0;i < this -> associate;++i){
		if(this->valid[group * this->associate +1] && this->tags[group * this->associate +i] ==tag){
		time_m+=2;

		return (*(uint32_t *)(void *)gr(group,i,block_offset)) & mask(len);
		}
	}

	time_m+=200;
	int j=0;
	i = rand() %this->associate;
	if( this ->valid[group * this->associate +i]){
		uint32_t local_block=0;
		local_block=(this -> tags[group*this -> associate +i]*(this->size / this->block_size / this->associate))|group;
		for(j=0;j< this->block_size ; ++j){
			dram_write(local_block * this->block_size + j,1, *gr(group,i,j));
		}
	}
	
	for(j=0;j < this->block_size; ++j){
		*(uint8_t *)(void *)gr(group,i,j) = dram_read(block * this->block_size + j,i);
	}
	this ->tags[group * this ->associate + i] = tag;
	this ->valid[group * this ->associate + i]=1;
	return *(uint32_t *)(void *)gr(group,i,block_offset) & mask(len);
}


void write_cache(CACHE *this, uint32_t addr,int len,uint32_t data){
	time_m+=200;
	int j = 0;
	uint32_t block_offset=addr % this->block_size;
	uint32_t block = addr / this->block_size;
	uint32_t group = block % (this->size / this->block_size / this ->associate);
	uint32_t tag = block / (this->size / this ->block_size / this->associate);


	int i;
	if(block_offset + len > this ->block_size){
		uint32_t temp = data;
		for(i=0;i<len;++i){
			write_cache(this,addr+i,1,temp & 0xff);
			temp >>=8;
		}
		return;
	}

	for(i=0; i < this->associate; ++i){
		if(this->valid[group * this->associate+i] && this->tags[group * this->associate + i] ==tag){
			int k=0;
			for(k = 0;k < len;++k){
				*(uint8_t *)(void *)gr(group,i,block_offset + k) = (uint8_t)(data & 0xff);
				data >>= 8;
			}

			for(j=0;j <this->block_size ; ++j){
				dram_write(block * this->block_size + j,1 ,*gr(group,i,j));
			}

		return;
		dram_write((uint64_t)(void *) gr(group,i,block_offset), len,data);
		}
	}

	i = rand() % this->associate;
	if (this->valid[group * this->associate + i]){
		uint32_t local_block=0;
        	local_block=(this->tags[group*this->associate+i]*(this->size / this->block_size / this->associate))|group;
		for (j = 0; j < this->block_size; ++j){
			dram_write(local_block * this->block_size + j, 1, *gr(group, i, j));
        	}
	}



	for (j = 0; j < this->block_size; ++j){

		*(uint8_t *)(void *)gr(group, i, j) = dram_read(block * this->block_size + j, 1);
	}
	this->tags[group * this->associate + i] = tag;
	this->valid[group * this->associate + i] = 1;
	int k = 0;
	for (k = 0; k < len; ++k){
		*(uint8_t *)(void *)gr(group, i, block_offset + k) = (uint8_t)(data & 0xff);
		data >>= 8;
	}
	for (j = 0; j < this->block_size; ++j){

		dram_write(block * this->block_size + j, 1, *gr(group, i, j));
	}
	return;
}




void init_cache(){

	int bs = 64, sz = 64 * 1024, ass = 8;
	L1_cache.cache_blocks = malloc(sz);
	L1_cache.tags = malloc((sz / bs) * 4);
	L1_cache.valid = malloc(sz / bs);
	memset(L1_cache.valid, 0, sz / bs);
	L1_cache.block_size = bs;
	L1_cache.size = sz;
	L1_cache.associate = ass;
	L1_cache.next = NULL;
}







































