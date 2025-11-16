#ifndef PROCESS_H
#define PROCESS_H
 
 #include <fs/vfs.h>
#include <mm/vmm.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "interrupts.h"
#include "elf.h"
#define PROCESS_PRI_MIN 0
#define PROCESS_PRI_MAX 10
#define PROCESS_PRI_DEFAULT 5
#define STATE_TERMINATED -1
#define STATE_READY 1
#define STATE_BLOCKED 0
#define STATE_RUNNING 2
 typedef struct process process_t;
typedef struct thread thread_t;

 typedef struct process{
    uint32_t Pid;
    char name[32];
    thread_t* threads;
    uint32_t priority;
    pagedir_t* dir;
    uint32_t state;
} process_t;
 
typedef struct thread
{
    uint32_t tid;
    uint32_t priority;
    uint32_t state;
    process_t* parent;
    uint32_t* kernel;
    interrupt_context_t* trap_frame;
    struct thread_t* next;
} thread_t;

 
 void process_create(process_t* process, const char* name, int32_t priority);
 void process_destroy(process_t* process);
 int32_t process_spawn(const char* filename);
 int32_t process_fork();
 process_t* process_find_by_pid(uint32_t pid);
 void process_exit(process_t* process, int32_t status);
thread_t* _get_main_thread(process_t* process);
thread_t* thread_create(process_t* parent_process, void* entry, void* arg);
int32_t thread_destroy(thread_t* thread);
void scheduler_init(void);
void scheduler_tick(interrupt_context_t* context);
void scheduler_switch(thread_t* next_thread);
void scheduler_post(thread_t* thread);
process_t* get_current_proc(void);
thread_t* get_current_thread(void);
void thread_exit();
#endif // PROCESS_H