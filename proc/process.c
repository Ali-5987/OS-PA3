#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <../include/proc/process.h>
#include <../include/mm/vmm.h>
//#include <../include/mm/kheap.h>
#define LOG_MOD_NAME 	"PRC"
#define LOG_MOD_ENABLE  1

#include <log.h>
static int current_pid=1;
static int current_tid=1;
void process_create(process_t* process, const char* name, int32_t priority)
{   process->state =1;
    process->Pid = current_pid;
    current_pid++;
    process->dir = vmm_create_address_space();
    memcpy(process->dir,vmm_get_kerneldir(),sizeof(pagedir_t));
    process->priority = priority;
    strncpy(process->name, name,31);
    process->threads = NULL;
}
 void process_destroy(process_t* process)
 {  if (process->state == STATE_TERMINATED)
    return;
    process->state = STATE_TERMINATED;
    thread_t* this_thread = process->threads;
    while (!this_thread)
    {
        thread_t* prev = this_thread;
        thread_destroy(prev);
        this_thread = this_thread->next;
    }
    if (process->dir)
    {
        vmm_free_region(process->dir, (void*)0x0, 0xC0000000);
        kfree(process->dir);
    }
    kfree(process);
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
    return new_proc->Pid;
 }
 int32_t process_fork(){}
 process_t* process_find_by_pid(uint32_t pid){}
 void process_exit(process_t* process, int32_t status){}
thread_t* _get_main_thread(process_t* process)
{   if (!process)
    return NULL;
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
    thisthread->priority =5;
    parent_process->threads = thisthread;
    uint32_t* stack_top = (uint32_t*)((uint32_t)thisthread->kernel + 4*1024);
    *(--stack_top) = (uint32_t)arg;
    *(--stack_top) = (uint32_t)thread_exit;

    interrupt_context_t* context = (interrupt_context_t*)((uint32_t)stack_top - sizeof(interrupt_context_t));
    context->eip = (uint32_t)entry;

    thisthread->trap_frame = context;
    context->cs = 0x1B;
    context->ds = 0x23;
    context->ss = 0x23;     
    context->eflags = 0x202;
    return thisthread;
}
int32_t thread_destroy(thread_t* thread)
{
    
}
void scheduler_init(void){}
void scheduler_tick(interrupt_context_t* context){}
void scheduler_switch(thread_t* next_thread){}
void scheduler_post(thread_t* thread){}
process_t* get_current_proc(void){}
thread_t* get_current_thread(void){}
void thread_exit()
{
    return;
}