#include <nemu.h>

lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t id)
{
    if (cpu.cr0.protect_enable == 0)
    {
        return addr;
    }
    else
    {
        return cpu.sreg[id].base + addr;
    }
}