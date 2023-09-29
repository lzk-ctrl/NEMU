#include <nemu.h>
#include <memory/tlb.h>

hwaddr_t page_translate(lnaddr_t addr)
{
    if (cpu.cr0.protect_enable == 1 && cpu.cr0.paging == 1)
    {
        uint32_t dir = addr >> 22;
        uint32_t page = (addr >> 12) & 0x3ff;
        uint32_t offset = addr & 0xfff;
        //read TLB
        int i = read_tlb(addr);
        if (i != -1)
        {
            return (tlb[i].page_num << 12) + offset;
        }
        // get dir position
        uint32_t dir_begin = cpu.cr3.page_directory_base;
        uint32_t dir_pos = (dir_begin << 12) + (dir << 2);
        Page_Descriptor first_content;
        first_content.val = hwaddr_read(dir_pos, 4);
        Assert(first_content.p == 1, "Dir Cannot Be Used!");
        // get page position
        uint32_t page_begin = first_content.addr;
        uint32_t page_pos = (page_begin << 12) + (page << 2);
        Page_Descriptor second_content;
        second_content.val = hwaddr_read(page_pos, 4);
        Assert(second_content.p == 1, "Page Cannot Be Used!");
        // get hwaddr
        uint32_t addr_begin = second_content.addr;
        hwaddr_t hwaddr = (addr_begin << 12) + offset;
        write_tlb(addr, hwaddr);
        return hwaddr;
    }
    else
    {
        return addr;
    }
}

hwaddr_t page_translate_monitor(lnaddr_t addr, int *flag)
{
    if (cpu.cr0.protect_enable == 1 && cpu.cr0.paging == 1)
    {
        uint32_t dir = addr >> 22;
        uint32_t page = (addr >> 12) & 0x3ff;
        uint32_t offset = addr & 0xfff;
        // get dir position
        uint32_t dir_begin = cpu.cr3.page_directory_base;
        uint32_t dir_pos = (dir_begin << 12) + (dir << 2);
        Page_Descriptor first_content;
        first_content.val = hwaddr_read(dir_pos, 4);
        if (first_content.p == 0)
        {
            *flag = 1;
            return 0;
        }
        // get page position
        uint32_t page_begin = first_content.addr;
        uint32_t page_pos = (page_begin << 12) + (page << 2);
        Page_Descriptor second_content;
        second_content.val = hwaddr_read(page_pos, 4);
        if (second_content.p == 0)
        {
            *flag = 2;
            return 0;
        }
        // get hwaddr
        uint32_t addr_begin = second_content.addr;
        hwaddr_t hwaddr = (addr_begin << 12) + offset;
        return hwaddr;
    }
    else
    {
        return addr;
    }
}