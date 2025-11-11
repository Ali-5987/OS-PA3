#include <driver/timer.h>
#include <testmain.h>
#include <stdint.h>
#include <string.h>

//-----------------------------------------------------------------------------
// TEST 1: Tick Count Incrementing
//-----------------------------------------------------------------------------
void test_tick_count_incrementing(void)
{
    // Get initial tick count
    uint32_t tick1 = get_system_tick_count();
    
    // Busy wait for a bit to let timer tick
    for (volatile int i = 0; i < 9000000; i++) {}
    
    // Get tick count again
    uint32_t tick2 = get_system_tick_count();
    
    // Tick count should have increased
    ASSERT_TRUE(tick2 > tick1, "tick count did not increment");
    
    // Wait again
    for (volatile int i = 0; i < 9000000; i++) {}
    
    uint32_t tick3 = get_system_tick_count();
    
    // Should continue incrementing
    ASSERT_TRUE(tick3 > tick2, "tick count stopped incrementing");
    
    char msg[64];
    strcpy(msg, "PASSED: test_tick_count_incrementing (ticks: ");
    char num[16];
    utoa(tick1, num);
    strcat(msg, num);
    strcat(msg, " -> ");
    utoa(tick2, num);
    strcat(msg, num);
    strcat(msg, " -> ");
    utoa(tick3, num);
    strcat(msg, num);
    strcat(msg, ")");
    send_msg(msg);
}

//-----------------------------------------------------------------------------
// TEST 2: Sleep Duration Accuracy
//-----------------------------------------------------------------------------
void test_sleep_duration(void)
{
    // Get tick count before sleep
    uint32_t tick_before = get_system_tick_count();
    
    // Sleep for 100ms
    sleep(100);
    
    // Get tick count after sleep
    uint32_t tick_after = get_system_tick_count();
    
    // Calculate elapsed ticks
    uint32_t elapsed_ticks = tick_after - tick_before;
    
    // Verify we slept (ticks should have advanced)
    ASSERT_TRUE(elapsed_ticks > 0, "no ticks elapsed during sleep");
    
    // For a 100ms sleep at typical frequencies (e.g., 100Hz = 10ms per tick)
    // we expect roughly 10 ticks (±some tolerance for scheduling)
    // This is a loose check - just verify sleep actually suspended execution
    ASSERT_TRUE(elapsed_ticks >= 5, "sleep duration too short");
    ASSERT_TRUE(elapsed_ticks <= 200, "sleep duration too long");
    
    char msg[64];
    strcpy(msg, "PASSED: test_sleep_duration (100ms = ");
    char num[16];
    utoa(elapsed_ticks, num);
    strcat(msg, num);
    strcat(msg, " ticks)");
    send_msg(msg);
}

//-----------------------------------------------------------------------------
// TEST 3: Multiple Sleeps
//-----------------------------------------------------------------------------
void test_multiple_sleeps(void)
{
    uint32_t tick_start = get_system_tick_count();
    
    // Sleep three times for 50ms each
    sleep(50);
    uint32_t tick1 = get_system_tick_count();
    
    sleep(50);
    uint32_t tick2 = get_system_tick_count();
    
    sleep(50);
    uint32_t tick3 = get_system_tick_count();
    
    // Verify each sleep advanced the tick count
    ASSERT_TRUE(tick1 > tick_start, "first sleep did not advance ticks");
    ASSERT_TRUE(tick2 > tick1, "second sleep did not advance ticks");
    ASSERT_TRUE(tick3 > tick2, "third sleep did not advance ticks");
    
    // Total elapsed should be roughly equivalent to 150ms
    uint32_t total_elapsed = tick3 - tick_start;
    ASSERT_TRUE(total_elapsed > 0, "no total elapsed time");
    
    char msg[128];
    strcpy(msg, "PASSED: test_multiple_sleeps (3x50ms = ");
    char num[16];
    utoa(total_elapsed, num);
    strcat(msg, num);
    strcat(msg, " ticks, intervals: ");
    utoa(tick1 - tick_start, num);
    strcat(msg, num);
    strcat(msg, ",");
    utoa(tick2 - tick1, num);
    strcat(msg, num);
    strcat(msg, ",");
    utoa(tick3 - tick2, num);
    strcat(msg, num);
    strcat(msg, ")");
    send_msg(msg);
}