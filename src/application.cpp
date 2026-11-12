/*
    application.cpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#pragma once

#include "application.hpp"

#include <fcntl.h>
#include <link.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>

namespace spiral::application {

void findSection(const char *section_name, uintptr_t *start, uintptr_t *size) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        int err = errno;
        printf("[spiral::application.findSection] failed to open application '%s': %s\n", path, strerror(err));
        return;
    }

    struct stat st;
    fstat(fd, &st);
    void *filemap = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (filemap == MAP_FAILED) {
        int err = errno;
        printf("[spiral::application.findSection] failed to map application: %s\n", strerror(err));
        return;
    }
    close(fd);

    auto *ehdr = reinterpret_cast<Elf64_Ehdr *>(filemap);
    auto *shdrs = reinterpret_cast<Elf64_Shdr *>(
        reinterpret_cast<char *>(filemap) + ehdr->e_shoff);
    const char *shstrtab =
        reinterpret_cast<char *>(filemap) + shdrs[ehdr->e_shstrndx].sh_offset;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        const char *name = shstrtab + shdrs[i].sh_name;
        if (strcmp(name, section_name) == 0) {
            *start = base + shdrs[i].sh_addr;
            *size = shdrs[i].sh_size;
            break;
        }
    }

    munmap(filemap, st.st_size);
}

bool readApplication() {
    dl_iterate_phdr(
        [](dl_phdr_info *info, size_t, void *) -> int {
            base = info->dlpi_addr;
            return 1;
        },
        NULL);

    if (base == 0) {
        puts("[spiral::application.readApplication] failed to get application base. quitting...");
        return false;
    }

    printf("[spiral] application base: 0x%lx\n", application::base);

    if (readlink("/proc/self/exe", path, sizeof(path)) < 10) {
        puts("[spiral::application.readApplication] failed to get application path. quitting...");
        return false;
    }

    printf("[spiral] application path: %s\n", path);

    findSection(".text", &text_section, &text_section_size);
    if (text_section == 0 || text_section_size == 0) {
        puts("[spiral::application.readApplication] failed to get text section. quitting...");
        return false;
    };

    printf("[spiral] .text section: 0x%lx .. 0x%lx\n", application::text_section, application::text_section + application::text_section_size);

    return true;
}

}  // namespace spiral::application
