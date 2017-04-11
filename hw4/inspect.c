#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <elf.h>
#include <string.h> 

/* Given the in-memory ELF header pointer as `ehdr` and a section
   header pointer as `shdr`, returns a pointer to the memory that
   contains the in-memory content of the section */
#define AT_SEC(ehdr, shdr) ((void *)(ehdr) + (shdr)->sh_offset)

static void check_for_shared_object(Elf64_Ehdr *ehdr);
static void fail(char *reason, int err_code);

void getVars(void* func, Elf64_Ehdr* ehdr);

/*
int strcmp(const char* a, const char* b){
  int i = 0;
  printf("Entered function!");
  
  while(a[i] != 0 && b[0] != 0){
    if(a[i] != b[1])
      return 1;
    printf("%d\n", i);
    i++;
  }

  if((a[i] != 0 && b[i] == 0) || (a[i] == 0 && b[i] != 0))
    return 1;
  else
    return 0;
}
*/

// Returns a pointer to the section header of the given section name.
Elf64_Shdr* getSectionByName(Elf64_Ehdr* ehdr, char* name){
  Elf64_Shdr *shdrs = (void*)ehdr + ehdr->e_shoff; // Gets the position of section header information.

  // Get the location of the string table, which will contain the human-readable section names.
  char* strs = (void*)ehdr+shdrs[ehdr->e_shstrndx].sh_offset;
  // printf("getsecbyname");
  // Loop through all of the section headers
  int i;
  for(i = 0; i < ehdr->e_shnum; i++){
  
    if(strcmp(strs + shdrs[i].sh_name, name) == 0){ // If this section is the one we're looking for.
      return &shdrs[i]; // Return the location of the sh_header
    }
    
  }

  return NULL;
}

void parseSectionNames(Elf64_Ehdr* ehdr){
  Elf64_Shdr *shdrs = (void*)ehdr + ehdr->e_shoff; // Gets the position of section header information.

  // Get the location of the string table, which will contain the human-readable section names.
  char* strs = (void*)ehdr+shdrs[ehdr->e_shstrndx].sh_offset;
  
  // Loop through all of the section headers
  int i;
  for(i = 0; i < ehdr->e_shnum; i++){
    // shdrs[i].sh_name contains the offset into the str table where the section name can be found.
    printf("%s\n", strs + shdrs[i].sh_name);
  }
}

/*
 * Returns the number of functions in the file. Loads the names of those functions into funcNames.
 */
int parseFunctions(Elf64_Ehdr* ehdr, char*** funcNames){
  // Get the location of the dynsym section
  Elf64_Shdr *dynsym_shdr = getSectionByName(ehdr, ".dynsym");
  
  Elf64_Sym *syms = AT_SEC(ehdr, dynsym_shdr);
  
  // Get the location of the string table, which will contain the function names
  char *strs = AT_SEC(ehdr, getSectionByName(ehdr, ".dynstr"));
  
  int i = 0, count = dynsym_shdr->sh_size / sizeof(Elf64_Sym);

  char** funcs = (char**)calloc(count, sizeof(char*));
  
  int numFuncs = 0;

  // Need to determine where the .text section starts
  Elf64_Shdr* text_hdr = getSectionByName(ehdr, ".text");
  void* text = AT_SEC(ehdr, text_hdr); // pointer to the start of the text section
  
  // Walk through the symbol table
  for (i = 0; i < count; i++) {
    if(ELF64_ST_TYPE(syms[i].st_info) == STT_FUNC){ // Check to see if the symbol is a function
      char* name = strs + syms[i].st_name;

      // Ignore compiler-generated functions
      if(strcmp(name, "__cxa_finalize") && strcmp(name, "_init") && strcmp(name, "_fini")){
	funcs[numFuncs] = name;
	
	long offset = syms[i].st_value - text_hdr->sh_addr; // Offset into elf file
	void* func = text + offset; // The location of this function's machine code

	printf("%s\n", name);
	getVars(func, ehdr);
	
	numFuncs++;
      }
    }
  }

  *funcNames = funcs;
  return numFuncs;
}

/*
 * Determines what, if any, global variables are used by a function.
 */
void getVars(void* func, Elf64_Ehdr* ehdr){
  char* currByte = (char*)func;
  short mov = 0; // State variable for parsing movq instructions

  Elf64_Shdr* reladyn_hdr = getSectionByName(ehdr, ".rela.dyn");

  int count = reladyn_hdr->sh_size / sizeof(Elf64_Rela);
  //printf("%d\n", count);
  
  // Contains r_offset, r_info, and r_append fields.
  Elf64_Rela* reladyn = AT_SEC(ehdr, reladyn_hdr);

  Elf64_Shdr* dyn_hdr = getSectionByName(ehdr, ".dynsym");
  Elf64_Sym* sym = AT_SEC(ehdr, dyn_hdr);
  
  // Get the location of the string table, which will contain the function names
  char *strs = AT_SEC(ehdr, getSectionByName(ehdr, ".dynstr"));
  
  while(1){
    //printf("%x\n", *currByte);
    
    if(*currByte == (char)0x48){ // mov
      currByte++;
      
      if(*currByte == (char)0x8b){
	currByte++; // Now will be the reg flag

	int top = *currByte >> 6;
	int bottom = *currByte & (char)7;

	if(top == 0 && bottom == 5){ // If this is a PC-relative mov
	  //printf("PC relative!\n");

	  // Collect the next four bytes, which will be the PC-relative offset
	  unsigned int pcOff = 0;

	  /*
	  for(int i = 0; i < 4; i++){
	    currByte++;
	    pcOff = pcOff | (int)(*currByte);
	    printf("%x\t%x\n", *currByte, pcOff);
	    if(i != 3)
	      pcOff <<= 8;
	    //printf("%x\t%x\n", *currByte, pcOff);
	  }
	  */
	  int i;
	  for(i = 0; i < 4; i++){
	    currByte++;
	    unsigned int temp = (int)(*currByte);
	    temp <<= 24;
	    pcOff = pcOff | temp;
	    //printf("%x\t%x\n", *currByte, pcOff);
	    if(i != 3)
	      pcOff >>= 8;
	    //printf("%x\t%x\n", *currByte, pcOff);
	  }
	  
	  //printf("%x\n", pcOff);

	  // Need to use this info to compute actual offset
	  pcOff += (long)currByte - (long)ehdr + 1;
	  //printf("%x\n", pcOff);
	  
	  //printf("Offset\n");
	  for(i = 0; i < count; i++){
	    if(reladyn[i].r_offset == pcOff){
	      //printf("Found it!!\n");
	    
	      //printf("%x\n", ELF64_R_SYM(reladyn[i].r_info)); // print the index into the symbol table

	      Elf64_Sym entry = sym[ELF64_R_SYM(reladyn[i].r_info)]; // The entry for this var
	      char* name = strs + entry.st_name;
	      printf("  %s\n", name);
	    }
	  }
	}
      }
    }
    else if(*currByte == (char)0xc3){ // ret
      break;
    }

    currByte++;
  }
}

// Print the names of all glbl vars found in the file
void printGlbVars(Elf64_Ehdr* ehdr){
  // Global vars are given in .bss, .data, .rodata
  Elf64_Shdr* bss_hdr = getSectionByName(ehdr, ".bss");
  Elf64_Shdr* data_hdr = getSectionByName(ehdr, ".data");
  Elf64_Shdr* rodata_hdr = getSectionByName(ehdr, ".rodata");

  Elf64_Shdr *sym_shdr = getSectionByName(ehdr, ".symtab");

  Elf64_Sym *syms = AT_SEC(ehdr, sym_shdr);

  char *strs = AT_SEC(ehdr, getSectionByName(ehdr, ".strtab"));

  int i = 0, count = sym_shdr->sh_size / sizeof(Elf64_Sym);

  for(i = 0; i < count; i++){
    if(ELF64_ST_TYPE(syms[i].st_info) == STT_OBJECT)
      printf("%s\n", strs + syms[i].st_name);
  }
}

int main(int argc, char **argv) {
  int fd;
  size_t len;
  void *p; 
  Elf64_Ehdr *ehdr; // pointer to the start of the ELF header

  if (argc != 2)
    fail("expected one file on the command line", 0);

  /* Open the shared-library file */
  fd = open(argv[1], O_RDONLY);
  if (fd == -1)
    fail("could not open file", errno);

  /* Find out how big the file is: */
  len = lseek(fd, 0, SEEK_END);

  /* Map the whole file into memory: */
  p = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
  if (p == (void*)-1)
    fail("mmap failed", errno);

  /* Since the ELF file starts with an ELF header, the in-memory image
     can be cast to a `Elf64_Ehdr *` to inspect it: */
  ehdr = (Elf64_Ehdr *)p;  // 

  /* Check that we have the right kind of file: */
  check_for_shared_object(ehdr);

  /* Add a call to your work here */
  char** funcNames;
  int count = parseFunctions(ehdr, &funcNames);
  //for(int i = 0; i < count; i++)
  //printf("%s\n", funcNames[i]);

  //printGlbVars(ehdr);
  
  return 0;
}

static void check_for_shared_object(Elf64_Ehdr *ehdr) {
  if ((ehdr->e_ident[EI_MAG0] != ELFMAG0)
      || (ehdr->e_ident[EI_MAG1] != ELFMAG1)
      || (ehdr->e_ident[EI_MAG2] != ELFMAG2)
      || (ehdr->e_ident[EI_MAG3] != ELFMAG3))
    fail("not an ELF file", 0);

  if (ehdr->e_ident[EI_CLASS] != ELFCLASS64)
    fail("not a 64-bit ELF file", 0);
  
  if (ehdr->e_type != ET_DYN)
    fail("not a shared-object file", 0);
}

static void fail(char *reason, int err_code) {
  fprintf(stderr, "%s (%d)\n", reason, err_code);
  exit(1);
}
