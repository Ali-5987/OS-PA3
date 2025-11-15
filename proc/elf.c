#include <../include/proc/elf.h>
#include <../include/utils.h>
bool elf_check_hdr(elf_header_t* hdr)
{
    if (hdr->e_ident[0] != "0x7F" ||hdr->e_ident[1] != "E" || hdr->e_ident[2] != "L" || hdr->e_ident[2] != "F")
   return false;
   if (hdr->e_type != ELF_TYPE_EXEC)
   return false;
    if (hdr->e_ident[4] != ELF_CLASS_32) 
    return false;
    if (hdr->e_ident[5] != ELF_DATA_LSB) 
        return false;

    return true;
}
int32_t elf_load_seg(file_t* file, pagedir_t* dir, elf_phdr_t* phdr)
{
    if (!file || !dir || !phdr)
    return -1;
    if (phdr->p_type != ELF_PT_LOAD)
    return 0;
    uint32_t start_addr = PAGE_ALIGN_UP(phdr->p_vaddr);
    uint32_t end_addr = PAGE_ALIGN_DWN(phdr->p_vaddr + phdr->p_memsz);
    uint32_t num_pages = (end_addr - start_addr) / VMM_PAGE_SIZE;
    uint32_t allgined_vaddr = PAGE_ALIGN_UP(phdr->p_vaddr);
   // uint32_t size = phdr->p_memsz + (allgined_vaddr - phdr->p_vaddr);
    vmm_alloc_region(dir,allgined_vaddr,num_pages, PDE_USER | PDE_PRESENT | PDE_WRITABLE);
    vfs_seek(file, phdr->p_offset);
    vfs_read(file, (void*)phdr->p_vaddr, phdr->p_filesz);

    if (phdr->p_memsz > phdr->p_filesz)
    {
        memset((void*) (phdr->p_vaddr + phdr->p_filesz), 0,phdr->p_memsz - phdr->p_filesz);
    }
    return 0;
}
int32_t elf_load(const char* path, pagedir_t* dir, void** entry)
{

};
