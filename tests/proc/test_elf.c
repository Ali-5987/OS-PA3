// #include <proc/elf.h>
// #include <fs/vfs.h>
// #include <mm/vmm.h>
// #include <string.h>
// #include <stddef.h>

// #include <testmain.h>

// //-----------------------------------------------------------------------------
// // TEST 1: ELF Header Validation - Valid File
// //-----------------------------------------------------------------------------
// void test_elf_check_valid_header(void)
// {
//     // Open a known-good ELF file

// 	char name[12];
// 	strncpy(name, "/fd0/HELLO", 12);
//     file_t* file = vfs_open(name, 0);
//     ASSERT_NOT_NULL(file, "could not open HELLO file");
    
//     // Read ELF header
//     elf_header_t header;
//     int32_t bytes_read = vfs_read(file, &header, sizeof(elf_header_t));
//     ASSERT_TRUE(bytes_read == sizeof(elf_header_t), "could not read ELF header");
    
//     send_msg("PASSED: test_elf_check_valid_header");
// }

// //-----------------------------------------------------------------------------
// // TEST 2: ELF Load - Load Complete Program
// //-----------------------------------------------------------------------------
// void test_elf_load_program(void)
// {
//     // Create a page directory for loading
//     pagedir_t* test_dir = vmm_get_current_pagedir();
//     ASSERT_NOT_NULL(test_dir, "could not create address space");
    
//     void* entry_point = NULL;
    
//     // Load the HELLO program
// 	char name[12];
// 	strncpy(name, "/fd0/HELLO", 12);
//     int32_t result = elf_load(name, test_dir, &entry_point);
//     ASSERT_EQ(result, 0, "elf_load failed for HELLO");
//     ASSERT_NOT_NULL(entry_point, "entry point not set after load");
    
//     // Entry point should be a valid user space address
//     ASSERT_TRUE((uint32_t)entry_point >= 0x100000, "entry point incorrect");
    
//     char msg[64];
//     strcpy(msg, "PASSED: test_elf_load_program (entry @ 0x");
//     char num[16];
//     utoa((uint32_t)entry_point, num);
//     strcat(msg, num);
//     strcat(msg, ")");
//     send_msg(msg);
    
// }

// //-----------------------------------------------------------------------------
// // TEST 3: ELF Load - Nonexistent File
// //-----------------------------------------------------------------------------
// void test_elf_load_nonexistent(void)
// {
//     pagedir_t* dir = vmm_get_current_pagedir();
//     ASSERT_NOT_NULL(dir, "could not create address space");
    
//     void* entry = NULL;
    
//     // Try to load a file that doesn't exist
//     int32_t result = elf_load("/fd0/DOESNOTEXIST", dir, &entry);
    
//     // Should fail (non-zero return)
//     ASSERT_TRUE(result != 0, "elf_load succeeded for nonexistent file");
    
//     send_msg("PASSED: test_elf_load_nonexistent");
// }
