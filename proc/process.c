#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <../include/proc/process.h>
#include <../include/mm/vmm.h>
#include <../include/mm/kheap.h>
#include <../include/utils.h>
//#include <../include/mm/kheap.h>
#define LOG_MOD_NAME 	"PRC"
#define LOG_MOD_ENABLE  1

#include <log.h>
static process_t* current_proc = NULL;
static thread_t* current_thread = NULL;
static int current_pid=1;
static int current_tid=1;
static thread_t* queue[PROCESS_PRI_MAX+1];
void process_create(process_t* process, const char* name, int32_t priority)
{   process->state =1;
    process->Pid = current_pid;
    current_pid++;
    process->dir = vmm_create_address_space();
    // pagedir_t* kernel_dir = vmm_get_kerneldir();
    // process->dir->table[0] = kernel_dir->table[0];
    // for (int i = 768; i < 1024; i++)
    // {
    //     process->dir->table[i] = kernel_dir->table[i];
    // }
   // memcpy(process->dir,vmm_get_kerneldir(),sizeof(pagedir_t));
    process->priority = priority;
    strncpy(process->name, name,31);
    process->name[31] = '\0';
    process->threads = NULL;
}
 void process_destroy(process_t* process)
 {  if (process->state == STATE_TERMINATED)
    return;
    process->state = STATE_TERMINATED;
    thread_t* this_thread = process->threads;
    while (this_thread)
    {
        thread_t* prev = this_thread;
        this_thread = this_thread->next;
        if (prev)
        thread_destroy(prev);
    }
    if (process->dir)
    {
        vmm_free_region(process->dir, (void*)0x0, 0xC0000000);
        free(process->dir);
    }
    free(process);
 }
 int32_t process_spawn(const char* filename)
 {
    process_t* new_proc = malloc(sizeof(process_t));
    process_create(new_proc,filename,PROCESS_PRI_DEFAULT);
    void* entry;
    if (elf_load(filename,new_proc->dir,&entry) !=0)
    {process_destroy(new_proc);
        return -1;}
    new_proc->threads = thread_create(new_proc,entry,NULL);
    scheduler_post(new_proc->threads);
    return new_proc->Pid;
 }
 int32_t process_fork(){}
 process_t* process_find_by_pid(uint32_t pid){}
 void process_exit(process_t* process, int32_t status){}
thread_t* _get_main_thread(process_t* process)
{   if (!process)
        return NULL;
    if (!process->threads) {
        process->threads = thread_create(process, thread_exit, NULL);
    }
    
    return process->threads;
}
thread_t* thread_create(process_t* parent_process, void* entry, void* arg)
{
    thread_t* thisthread = malloc(sizeof(thread_t));
    thisthread->tid = current_tid;
    current_tid++;
    thisthread->kernel = malloc(4*1024);
    if (!thisthread->kernel)
    { 
        free(thisthread);
        return NULL;
    }
    thisthread->parent = parent_process;
    thisthread->state = 1;
    thisthread->next = parent_process->threads;
    thisthread->priority = PROCESS_PRI_DEFAULT;
    thisthread->time_slice = SLICE;
    thisthread->next_ready = NULL;
    parent_process->threads = thisthread;
    uint32_t* stack_top = (uint32_t*)((uint32_t)thisthread->kernel + 4*1024);
    *(--stack_top) = (uint32_t)arg;
    *(--stack_top) = (uint32_t)thread_exit;

    interrupt_context_t* context = (interrupt_context_t*)((uint32_t)stack_top - sizeof(interrupt_context_t));
    memset(context, 0, sizeof(interrupt_context_t));
    context->eip = (uint32_t)entry;

    thisthread->trap_frame = context;
    context->cs = 0x1B;
    context->ds = 0x23;
    context->ss = 0x23;         
    context->eflags = 0x202;
    return thisthread;
}
int32_t thread_destroy(thread_t* thread)
{   cli();
    if (!thread)
    {sti(); return -1;}
    if (thread->state == STATE_RUNNING)
   {sti(); return -1;}
    if (!thread->parent || !thread->parent->threads)
   {sti(); return -1;}
    thread_t* head = queue[thread->priority];
    if (head == thread)
    queue[thread->priority] = thread->next_ready;
    else
    {   thread_t *prev = head; 
        while (head && head != thread)
    {   prev = head;
        head= head->next_ready;
    }
    if (head)
    prev->next_ready = head->next_ready;}
    thread_t* current = thread->parent->threads;
    if (current == thread)
    thread->parent->threads = current->next;
    else
{    while (current != NULL && current->next != thread)
    {
            current = current->next;
    }
    if (current != NULL)
    {
        current->next = thread->next;
    }}

    free(thread->kernel);
    free(thread);
    sti();
    return 0;
}
void scheduler_init(void)
{
    process_t* kernel_process = malloc(sizeof(process_t));
    thread_t* kernel_thread = malloc(sizeof(thread_t));
    kernel_process->dir = vmm_get_kerneldir();
    kernel_process->Pid =current_pid;
    current_pid++;
    kernel_thread->parent = kernel_process;
    kernel_thread->state = STATE_RUNNING;
    kernel_process->threads = kernel_thread;
    kernel_process->state = STATE_RUNNING;
    kernel_thread->tid = current_tid;
    current_tid++;
    kernel_thread->priority =5;
    kernel_thread->time_slice = SLICE;
    kernel_thread->kernel = NULL;
    kernel_thread->trap_frame = NULL;
    for (int i=0;i<PROCESS_PRI_MAX;i++)
    queue[i] = NULL;
    current_proc = kernel_process;
    current_thread = kernel_thread;
}
void scheduler_tick(interrupt_context_t* context)
{
    current_thread->trap_frame = context;
    //current_thread->time_slice--;
    if (--current_thread->time_slice>0)
    return;
    current_thread->time_slice = SLICE;
    scheduler_post(current_thread);
    thread_t* next_thread = NULL;
    for (int i = PROCESS_PRI_MAX - 1; i >= 0; i--) 
    {
        if (queue[i] != NULL) 
        {
            next_thread = queue[i];
            queue[i] = next_thread->next_ready; 
            next_thread->next_ready = NULL;
            break;
        }
    }
    if (next_thread == NULL)
    return;
    cli();
    scheduler_switch(next_thread);
}
void scheduler_switch(thread_t* next_thread)
{
    if (!next_thread || next_thread == current_thread )
    return;
    if (next_thread->parent != current_thread->parent)
    vmm_switch_pagedir(next_thread->parent->dir);
    current_thread = next_thread;
    current_proc = next_thread->parent;
    next_thread->state = STATE_RUNNING;
    if (next_thread->kernel != NULL)
    tss_update_esp0((uint32_t)next_thread->kernel + 4 * 1024);
asm volatile (
        "movl %0, %%esp \n\t"   
        "popl %%ds       \n\t"   
        "popa            \n\t"   
        "addl $8, %%esp  \n\t"   
        "iret            \n\t"   
        : 
        : "r" (next_thread->trap_frame) 
        : "memory" 
    );
}
void scheduler_post(thread_t* thread)
{   cli();
    if (!thread || thread->state == STATE_RUNNING)
    {sti();
        return;}
    thread->state = STATE_READY;
    thread_t* head = queue[thread->priority];
    if (!head)
    queue[thread->priority] = thread;
    else
    {while (head->next_ready)
    {
        head= head->next_ready;
    }
    head->next_ready = thread;}
    sti();
}
process_t* get_current_proc(void)
{
    return current_proc;
}
thread_t* get_current_thread(void)
{
    return current_thread;
}
void thread_exit()
{   cli();
    current_thread->state = STATE_TERMINATED;
    thread_t* next_thread = NULL;
    for (int i = PROCESS_PRI_MAX - 1; i >= 0; i--) 
    {
        if (queue[i] != NULL) 
        {
            next_thread = queue[i];
            queue[i] = next_thread->next_ready; 
            next_thread->next_ready = NULL;
            break;
        }
    }

    if (next_thread == NULL)
    {
        while(1) {
        }
    }
    scheduler_switch(next_thread);
}