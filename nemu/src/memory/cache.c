#include "common.h"
#include "memory/cache.h"
#include "burst.h"
#include "stdlib.h"

uint32_t dram_read(hwaddr_t addr, size_t len);
void call_ddr3_read(hwaddr_t, void *);
void call_ddr3_write(hwaddr_t, void *, uint8_t *);

void init_cache(){
    int i;
    for (i = 0; i < CACHE_SIZE / CACHE_BLOCK; i++){
        cache[i].valid = false;
        cache[i].tag = 0;
        memset(cache[i].byte, 0, CACHE_BLOCK);
    }
    cnt = 0;
}

uint32_t cache_read(hwaddr_t addr) {
    uint32_t set = (addr >> 6) & 0x7f; //取出地址中set的部分（右移block位并 & 1111111），set = 7 byte (set = 2^s = 128), offset = 6 byte (block = 2^e = 64)
    bool hit = false;
    int i;
    for (i = set * E; i < (set + 1) * E; i++) { //找到相应set所在
        if (cache[i].tag == (addr >> 13) && cache[i].valid) { //如果tag和地址相符合并且alid == true
            hit = true; 
            cnt += 2;
            break;
        }
    }
    if (!hit) { //找不到
        for (i = set * E; i < (set + 1) * E; i++) { //去到相应set所在
            if (!cache[i].valid) //找到空的地方退出
            break; 
        }
        if (i == (set + 1) * E) { // 到最后仍然没有找到空的地方，执行随机替换算法
            srand(0);
            i = set * E + rand() % E;
        }
        cache[i].valid = true;
        cache[i].tag = addr >> 13;
       int j;
       uint32_t block = (addr >> 6) << 6; // 取出block
         for (j = 0; j < BURST_LEN; j++) {
            call_ddr3_read(block + j * BURST_LEN, cache[i].byte + j * BURST_LEN); 
         }
        cnt += 200;
    }
    return i;
}

void cache_write(hwaddr_t addr, size_t len, uint32_t data) {
    uint32_t set = (addr >> 6) & 0x7f;
    uint32_t offset = addr & (CACHE_BLOCK - 1); 
    int i;
    bool hit = false;
    for (i = set * E; i < (set + 1) * E; i++) {
        if (cache[i].tag == (addr >> 13) && cache[i].valid) {
            hit = true;
            break;
        }
    }
    if (hit) { // 写直通
        memcpy(cache[i].byte + offset, &data, len);
    }     
    else {
        i = cache_read(addr);
         memcpy(cache[i].byte + offset, &data, len);
    }
}
