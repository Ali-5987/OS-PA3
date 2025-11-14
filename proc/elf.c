#include <../include/proc/elf.h>
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
int32_t elf_load_seg(file_t* file, pagedir_t* dir, elf_phdr_t* phdr){};
int32_t elf_load(const char* path, pagedir_t* dir, void** entry){};
