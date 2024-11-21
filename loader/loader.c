#include "loader.h"

Elf32_Ehdr *ehdr = NULL; // ELF header
Elf32_Phdr *phdr = NULL; // Program header
int fd = -1;             // File descriptor

/*
 * Release memory and other cleanups
 */
void loader_cleanup() {
    free(ehdr);
    free(phdr);
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char **exe) {
    fd = open(exe[0], O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        exit(1);
    }

    // Allocate memory for ELF header
    ehdr = malloc(sizeof(Elf32_Ehdr));
    if (!ehdr) {
        perror("Error allocating memory for ELF header");
        loader_cleanup();
        exit(1);
    }

    // Read ELF header
    if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("Error reading ELF header");
        loader_cleanup();
        exit(1);
    }

    // Allocate memory for Program headers
    phdr = malloc(sizeof(Elf32_Phdr) * ehdr->e_phnum);
    if (!phdr) {
        perror("Error allocating memory for Program headers");
        loader_cleanup();
        exit(1);
    }

    // Seek to the Program header table and read it
    if (lseek(fd, ehdr->e_phoff, SEEK_SET) == -1) {
        perror("Error seeking to Program headers");
        loader_cleanup();
        exit(1);
    }
    if (read(fd, phdr, sizeof(Elf32_Phdr) * ehdr->e_phnum) != sizeof(Elf32_Phdr) * ehdr->e_phnum) {
        perror("Error reading Program headers");
        loader_cleanup();
        exit(1);
    }

    // Iterate through the PHDR table to find PT_LOAD segments
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            // Allocate memory for the segment
            void *segmem = mmap((void *)phdr[i].p_vaddr, phdr[i].p_memsz,
                                PROT_READ | PROT_WRITE | PROT_EXEC,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (segmem == MAP_FAILED) {
                perror("Error allocating memory with mmap");
                loader_cleanup();
                exit(1);
            }

            // Seek to the segment offset in the file
            if (lseek(fd, phdr[i].p_offset, SEEK_SET) == -1) {
                perror("Error seeking to segment offset");
                loader_cleanup();
                exit(1);
            }

            // Read the segment content into memory
            if (read(fd, segmem, phdr[i].p_filesz) != phdr[i].p_filesz) {
                perror("Error reading segment content");
                loader_cleanup();
                exit(1);
            }

            // Zero out any remaining memory if p_memsz > p_filesz
            if (phdr[i].p_memsz > phdr[i].p_filesz) {
                memset(segmem + phdr[i].p_filesz, 0, phdr[i].p_memsz - phdr[i].p_filesz);
            }
        }
    }

    // Navigate to the entry point address (e_entry)
    int (*_start)(void) = (int (*)(void))(ehdr->e_entry);

    // Call the entry point function
    int result = _start();
    printf("User _start return value = %d\n", result);
}

