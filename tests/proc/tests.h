#ifndef _PROC_TESTS_H
#define _PROC_TESTS_H


/* Tests list */

// -- Timer tests

extern void test_tick_count_incrementing(void);
extern void test_sleep_duration(void);
extern void test_multiple_sleeps(void);


// -- TSS tests --

extern void test_tss_global_access(void);
extern void test_tss_esp0_update(void);

// -- ELF Tests

extern void test_elf_check_valid_header(void);
extern void test_elf_load_program(void);
extern void test_elf_load_nonexistent(void);

// -- Process and threads tests --

extern void test_process_create_nonnull (void);
extern void test_process_create_different_priorities (void);
extern void test_process_name_handling(void);
extern void test_get_main_thread_nonnull(void);

extern void test_scheduler_init_idempotent(void);
extern void test_scheduler_get_current_thread(void);
extern void test_scheduler_get_current_proc(void);

extern void test_thread_creation(void);
extern void test_multiple_thread_creation(void);
extern void test_scheduler_post_thread(void);
extern void test_multiple_process_creation(void);
extern void test_thread_destroy_safe(void);
extern void test_process_exit_safe(void);
extern void test_concurrent_scheduler_ops(void);
extern void test_many_threads_creation(void);

extern void thread_test (void);

#endif