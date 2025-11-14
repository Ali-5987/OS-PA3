#include <stdint.h>
#include <stdio.h>

#include <driver/timer.h>
#include <interrupts.h>
#include <utils.h>
uint32_t ticks =0;
uint32_t freq=0;
void init_system_timer(uint32_t frequency)
{   freq = frequency;
    int divisor = 1193182/frequency;
    outb(0x34,PIT_COMMAND_PORT);
    outb((divisor) & 0xFF, PIT_CHANNEL0_DATA_PORT);
    outb((divisor>>8) & 0xFF, PIT_CHANNEL0_DATA_PORT);
    register_interrupt_handler(32,_timer_interrupt_handler);
}
void _timer_interrupt_handler(interrupt_context_t*)
{
    ticks++;
}
uint32_t get_system_tick_count()
{
    return ticks;
}
void sleep(uint32_t ms) {
    uint32_t initial_ticks = ticks;
    uint32_t final_ticks = initial_ticks+(ms*freq)/1000;
    while(1)
    {
        if (final_ticks<= ticks)
        break;
    }
}
