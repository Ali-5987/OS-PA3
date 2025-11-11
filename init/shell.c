#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <init/syscall.h>
#include <init/tty.h>
#include <driver/timer.h>
#include <mm/kheap.h>
#include <mm/vmm.h>
#include <proc/elf.h>
#include <fs/vfs.h>
#include <mem.h>

bool run_cmd(char* cmd)
{

	// tokenize the command
	char* args[4] = {0};
	int argc = 0;

	char* token = strtok(cmd, " ");
	while (token && argc < 4) {
		args[argc++] = token;
		token = strtok(NULL, " ");
	}

	cmd = args[0]; // the first token is the command

	//! exit command
	if (strcmp (cmd, "exit") == 0) {
		return true;
	}

	//! clear screen
	else if (strcmp (cmd, "clear") == 0) {
		terminal_clear_log ();
	}

	//! help
	else if (strcmp (cmd, "help") == 0) {

		printk ("\n");
		printk ("Leenix v0.1 shell\n");
		printk ("with a basic Command Line Interface (CLI)\n\n");
		printk ("Supported commands:\n");
		printk (" - clear: clears the display\n");
		printk (" - kinfo: print kernel memory info\n");
		printk (" - mdmp <address> <size>: dump memory contents at \
					address for size bytes\n");
		printk (" - sd: dump current stack contents\n");
		printk (" - heap: dump heap info\n");
		printk (" - ptwalk <vstart> <vend>: walk the PTs from vstart to vend\n");
		printk (" - break: trigger int3 breakpoint\n");
		printk (" - help: displays this message\n");
		printk (" - exit: quits and halts the system\n");
	}

	
	else if (strcmp (cmd, "heap") == 0) {

		heap_t* heap = get_kernel_heap();
		kheap_stats(heap);

	}

	else if (strcmp (cmd, "mdmp") == 0) {
		
		// dump memory contents
		if (argc < 3) {
			printk("Usage: mdmp <size> <address>\n");
			return false;
		}

		printk ("\n");
		uint32_t count = strtol(args[1], NULL, 0); // get the count of bytes to dump
		uint8_t* addr = (uint8_t*) strtol(args[2], NULL, 0); // get the address to dump from

		for (uint32_t i = 0; i < count; i += 16) {
			printk("0x%08x: ", addr + i);
			for (int j = 0; j < 16; j++) {
				if (i + j < count)
					printk("%02x ", addr[i + j]);
				else
					printk("   ");
			}
			printk(" |");
			for (int j = 0; j < 16 && (i + j) < count; j++) {
				char c = addr[i + j];
				printk("%c", (c >= 32 && c <= 126) ? c : '.');
			}
			printk("|\n");
		}

	}

	else if (strcmp (cmd, "ptwalk") == 0) {

		pagedir_t* pdbr   = vmm_get_current_pagedir();
		uint32_t   vstart = args[1] ? strtol(args[1], NULL, 0) : 0x0;      // start address
		uint32_t   vend   = args[2] ? strtol(args[2], NULL, 0) : 0x10000;  // end address

		printk("Page table walk from 0x%08x to 0x%08x\n", vstart, vend);

		for (uint32_t addr = vstart; addr <= vend; addr += 0x1000) {
			uint32_t pdi = (addr >> 22) & 0x3FF;
			uint32_t pti = (addr >> 12) & 0x3FF;

			pde_t pde = pdbr->table[pdi];
			if ( !(PDE_IS_PRESENT(pde)) )  {
				printk("0x%08x: PDE not present\n", addr);
				continue;
			}

			uint32_t* pt = (uint32_t*)(pde & 0xFFFFF000);
			pagetable_t* ptable = (pagetable_t*) PHYS_TO_VIRT(pt);
			pte_t pte = ptable->table[pti];

			printk("0x%08x -> 0x%08x [%s | %s | %s]\n",
				addr,
				pte & 0xFFFFF000,
				(pte & 1) ? "PRESENT" : "NOT_PRESENT",
				(pte & (1 << 1)) ? "RW" : "RO",
				(pte & (1 << 2)) ? "USER" : "KERN"
			);
		}
	}

	//! empty command
	else if (strcmp (cmd, "") == 0) {
		// do nothing
	}
	//! trigger a breakpoint
	else if (strcmp (cmd, "break") == 0) {
		asm volatile ("int3;");
	}

	//! print kernel info
	else if (strcmp (cmd, "kinfo") == 0) {
		extern char kernel_start;
		extern char kernel_end;
		printk("Kernel loaded from %p to %p\n", &kernel_start, &kernel_end);
		printk("Kernel size: %u bytes\n", (uint32_t)(&kernel_end - &kernel_start));
	}

	//! stack dump
	else if (strcmp (cmd, "sd") == 0) {

		uint32_t* esp;
		asm volatile ("mov %%esp, %0" : "=r"(esp));

		printk("\n");
		printk("Stack dump (ESP = 0x%08x):", (uint32_t)esp);
		printk("\n");
		printk("-----------------------------------------\n");
		printk("    Address       Value       \%esp\n");
		printk("-----------------------------------------\n");

		for (int i = -5; i < 5; ++i) {

			uint32_t* addr = esp + i;
			uint32_t val = *addr;

			if (i == 0) {
				printk("-> 0x%08x: 0x%08x   <-- ESP", (uint32_t)addr, val);
				printk("\n");
			} else {
				printk("   0x%08x: 0x%08x\n", (uint32_t)addr, val);
			}

		}
		printk("-----------------------------------------\n\n");

	}

	else if (strcmp (cmd, "echo") == 0) {
		char str[1024] = "";  // make sure it's large enough

		for (int i = 1; i< argc; i++) {
			strcat(str, args[i]);   // append argument
			strcat(str, " ");       // append a space
		}
		str[strlen(str) - 1] = '\0';
		printk ("%s\n", str);
	}

	else if (strcmp (cmd,"repeat") == 0){
		if (argc > 1){

			long n = strtol(args[1],NULL,10);
			char str[1024] = "";  // make sure it's large enough

			for (int i = 2; i< argc; i++) {
				strcat(str, args[i]);   // append argument
				strcat(str, " ");       // append a space
			}
			str[strlen(str) - 1] = '\0';
			for (long i = 0; i < n;i++){
				printk ("%s ", str);
			}
			printk ("%s\n","");
		}

	}

	else if (strcmp (cmd, "color") == 0) {
		terminal_settext_color(10);
	}

	else if (strcmp (cmd, "bgcolor") == 0) {
		terminal_setbg_color(8);
	}

	//! invalid command
	else {
		printk ("sh: error: Unkown command\n");
	}

	return false;

}

void shell()
{
	printk ("\n");
	// enable_interrupts();
	char cmd_buf[128]; // allocate memory for command buffer
	while (1) {
		printk (" # ");
		getlinek (cmd_buf, 128); // read command from terminal
		if (run_cmd(cmd_buf)) {
			printk ("exiting shell...\n");
			return; // exit the shell if the command is "exit"
		}
	}
}
