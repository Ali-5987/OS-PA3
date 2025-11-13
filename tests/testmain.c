#include <driver/serial.h>

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <testmain.h>
#include <proc/tests.h>

/* Minimal unsigned int → string converter */
void utoa(unsigned val, char *buf) {
    char tmp[16];
    int i = 0, j = 0;

    if (val == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (val > 0 && i < (int)sizeof(tmp)) {
        tmp[i++] = '0' + (val % 10);
        val /= 10;
    }

    while (i > 0) {
        buf[j++] = tmp[--i];
    }
    buf[j] = '\0';
}

/* All commands received from the orchestrator are read in this buffer */

#define CMD_BUF_SIZE	128
static  char 	cmd_buf [CMD_BUF_SIZE];

/* List of all the test name to test function mappings. All commands are sent 
	as a single word, and the corresponding test is ran in the kernel. Support
	for params can be added later by using simple strtok calls. */

static struct test_case test_cases[] = {

	// -- Timer tests 

	// { "test_tick_count_incrementing",		test_tick_count_incrementing },
	// { "test_sleep_duration",			test_sleep_duration },
	// { "test_multiple_sleeps",			test_multiple_sleeps },

	// -- TSS tests --

	{ "test_tss_global_access",			test_tss_global_access },
	{ "test_tss_esp0_update",			test_tss_esp0_update },

	// -- ELF Tests --

	// { "test_elf_check_valid_header",	test_elf_check_valid_header },
	// { "test_elf_load_program",			test_elf_load_program },
	// { "test_elf_load_nonexistent",		test_elf_load_nonexistent },

	// -- Process and threads tests --

	// { "test_thread",						thread_test },
	// { "test_process_create_nonnull",		test_process_create_nonnull },
	// { "test_process_create_different_priorities",	test_process_create_different_priorities },
	// { "test_process_name_handling",		test_process_name_handling },
	// { "test_get_main_thread_nonnull",		test_get_main_thread_nonnull },
	// { "test_scheduler_init_idempotent",		test_scheduler_init_idempotent },
	// { "test_scheduler_get_current_thread",	test_scheduler_get_current_thread },
	// { "test_scheduler_get_current_proc",		test_scheduler_get_current_proc },
	// { "test_thread_creation",			test_thread_creation },
	// { "test_multiple_thread_creation",		test_multiple_thread_creation },
	// { "test_scheduler_post_thread",		test_scheduler_post_thread },
	// { "test_multiple_process_creation",		test_multiple_process_creation },
	// { "test_thread_destroy_safe",			test_thread_destroy_safe },
	// { "test_process_exit_safe",			test_process_exit_safe },
	// { "test_concurrent_scheduler_ops",		test_concurrent_scheduler_ops },
	// { "test_many_threads_creation",		test_many_threads_creation },
	

	{ NULL, NULL } // marks the end of the array

};

/* Responsible for test orchestration inside the kernel. */

void start_tests () {

	/* testing cycle is simple, read command, dispatch to function, repeat */
	while (1) {

		size_t cmd_size = read_command ();
		if (cmd_size == 0) {
			continue; // empty command, ignore
		}

		bool found = false;

		for (size_t i = 0; test_cases[i].name != NULL; i++) {

			if (strcmp (cmd_buf, test_cases[i].name) == 0) {

				found = true;
				test_cases[i].func (); // call the test function
				break;

			}

		}

		if (!found) {
			send_msg ("Unknown command");
		}

	}

}

/* Reads a command from the serial port into cmd_buf and returns
   the number of characters read. The command is terminated by a any of these:
   '\0', '\n', '\r'. */

size_t read_command () {

	size_t start = 0;
	memset (cmd_buf, 0, CMD_BUF_SIZE);

	while (1) {
		char c = serial_getc ();

		if (c == '\0' || c == '\n' || c == '\r') {

			cmd_buf [start] = '\0';
			break;
		
		} else {

			if (start < CMD_BUF_SIZE - 1) {
				cmd_buf [start++] = c;
			}
		
		}	
	}

	return start;
}

/* Send a message string back to the server. A "*" is used to mark the end
	of the message. */

void send_msg (const char *msg) {
	serial_puts (msg);
	serial_putc ('*'); // end of message marker
}
