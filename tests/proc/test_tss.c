#include <proc/tss.h>
#include <testmain.h>
#include <stdint.h>
#include <string.h>

//-----------------------------------------------------------------------------
// TEST 1: TSS Global Structure Access
//-----------------------------------------------------------------------------
void test_tss_global_access(void)
{
    // Get pointer to global TSS
    tss_t* tss = tss_get_global();
    
    ASSERT_NOT_NULL(tss, "global TSS is null");
    
    // Verify we can read from it (should not crash)
    // TSS is a valid memory location
    volatile uint32_t test_read = tss->esp0;
    (void)test_read;
    // Get it again to verify consistency
    tss_t* tss2 = tss_get_global();
    ASSERT_NOT_NULL(tss2, "second global TSS call returned null");
    
    // Should return the same pointer (global/singleton)
    ASSERT_TRUE(tss == tss2, "global TSS pointer inconsistent");
    
    char msg[64];
    strcpy(msg, "PASSED: test_tss_global_access (tss @ 0x");
    char num[16];
    utoa((uint32_t)tss, num);
    strcat(msg, num);
    strcat(msg, ")");
    send_msg(msg);
}

//-----------------------------------------------------------------------------
// TEST 2: TSS ESP0 Update
//-----------------------------------------------------------------------------
void test_tss_esp0_update(void)
{
    // Get global TSS
    tss_t* tss = tss_get_global();
    ASSERT_NOT_NULL(tss, "global TSS is null");
    
    // Save original esp0 value
    uint32_t original_esp0 = tss->esp0;
    
    // Update esp0 to a new value
    uint32_t test_esp0_1 = 0xDEADBEEF;
    tss_update_esp0(test_esp0_1);
    
    // Verify the update
    ASSERT_EQ(tss->esp0, test_esp0_1, "first esp0 update failed");
    
    // Update again with different value
    uint32_t test_esp0_2 = 0xCAFEBABE;
    tss_update_esp0(test_esp0_2);
    
    // Verify second update
    ASSERT_EQ(tss->esp0, test_esp0_2, "second esp0 update failed");
    
    // Update back to a reasonable value
    uint32_t test_esp0_3 = 0xC0000000;
    tss_update_esp0(test_esp0_3);
    ASSERT_EQ(tss->esp0, test_esp0_3, "third esp0 update failed");
    
    char msg[128];
    strcpy(msg, "PASSED: test_tss_esp0_update (");
    char num[16];
    utoa(original_esp0, num);
    strcat(msg, num);
    strcat(msg, " -> 0x");
    utoa(test_esp0_1, num);
    strcat(msg, num);
    strcat(msg, " -> 0x");
    utoa(test_esp0_2, num);
    strcat(msg, num);
    strcat(msg, " -> 0x");
    utoa(test_esp0_3, num);
    strcat(msg, num);
    strcat(msg, ")");
    send_msg(msg);
}