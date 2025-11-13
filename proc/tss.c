#include <../include/proc/tss.h>
static tss_t current_tss;
tss_t* global_tss = &current_tss;
void 	tss_flush (uint16_t selector)
{
    asm volatile ("mov %0,%%ax\n"
                 "ltr %%ax" : : "r" (selector) : "ax");
}
void 	tss_update_esp0 (uint32_t esp0)
{
    global_tss->esp0 = esp0;
}
tss_t* 	tss_get_global (void)
{
    return global_tss;
}