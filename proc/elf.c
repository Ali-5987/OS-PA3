#include <../include/proc/elf.h>
#include <../include/utils.h>
#include <../include/mm/kheap.h>
#include <../include/fs/vfs.h>
bool elf_check_hdr(elf_header_t* hdr)
{
    if (hdr->e_ident[0] != 0x7F ||hdr->e_ident[1] != 'E' || hdr->e_ident[2] != 'L' || hdr->e_ident[3] != 'F')
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
    uint32_t start_addr = PAGE_ALIGN_UP(phdr->p_vaddr);
    uint32_t end_addr = PAGE_ALIGN_DWN(phdr->p_vaddr + phdr->p_memsz);
    uint32_t size = end_addr - start_addr;
    uint32_t num_pages = (end_addr - start_addr) / VMM_PAGE_SIZE;
    uint32_t allgined_vaddr = PAGE_ALIGN_UP(phdr->p_vaddr);
    if (!vmm_alloc_region(dir,allgined_vaddr,size, PDE_USER | PDE_PRESENT | PDE_WRITABLE))
    return -1;
    file->f_offset = phdr->p_offset;
    vfs_read(file, (void*)phdr->p_vaddr, phdr->p_filesz);
    if (phdr->p_memsz > phdr->p_filesz)
    {
        memset((void*) (phdr->p_vaddr + phdr->p_filesz), 0,phdr->p_memsz - phdr->p_filesz);
    }
    return 0;
}
int32_t elf_load(const char* path, pagedir_t* dir, void** entry)
{   if (!path || !dir || !entry)
    return -1;
    file_t* thisfile = vfs_open(path,0);
    if (!thisfile)
    return -1;
    elf_header_t e_hdr;
    if (vfs_read(thisfile,&e_hdr,sizeof(elf_header_t)) != sizeof(elf_header_t))
    {
        //vfs_close(thisfile);
        return -1;
    }
    if (!elf_check_hdr(&e_hdr))
   { //vfs_close(thisfile);
    return -1;}
    for (uint32_t i=0; i<e_hdr.e_phnum;i++)
    {    
       uint32_t offset_start =  e_hdr.e_phoff + (i* e_hdr.e_phentsize);
       thisfile->f_offset = offset_start;
        elf_phdr_t phdr;
        vfs_read(thisfile,(void*) &phdr,sizeof (elf_phdr_t));
            if (phdr.p_type == ELF_PT_LOAD)
            {
                uint32_t result = elf_load_seg(thisfile,dir,&phdr);
                if (result ==-1)
                {
                    //vfs_close(thisfile);
                    return -1;
                }
            }
   }
    //vfs_close(thisfile);
    *entry = (void*)e_hdr.e_entry;
   return 0;
};
