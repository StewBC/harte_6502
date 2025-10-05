#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "6502.h"

// Configure RAM
uint8_t ram_init(RAM *ram, uint16_t num_ram_banks) {
    int ram_bank;
    if (!(ram->ram_banks = (RAM_BANK *)malloc(sizeof(RAM_BANK) * num_ram_banks))) {
        return (ram->num_ram_banks = 0);
    }
    ram->num_ram_banks = num_ram_banks;
    for (ram_bank = 0; ram_bank < num_ram_banks; ram_bank++) {
        ram->ram_banks[ram_bank].address = 0;
        ram->ram_banks[ram_bank].length = 0;
        ram->ram_banks[ram_bank].memory = NULL;
    }
    return 1;
}

void ram_add(RAM *ram, uint8_t ram_bank_num, uint32_t address, uint32_t length, uint8_t *memory) {
    assert(ram_bank_num < ram->num_ram_banks);
    RAM_BANK *r = &ram->ram_banks[ram_bank_num];
    r->address = address;
    r->length = length;
    r->memory = memory;
}

// Configure ROMS
uint8_t roms_init(ROMS *roms, uint16_t num_roms) {
    int rom;
    if (!(roms->rom = (ROM *)malloc(sizeof(ROM) * num_roms))) {
        return (roms->num_roms = 0);
    }
    roms->num_roms = num_roms;
    for (rom = 0; rom < num_roms; rom++) {
        roms->rom[rom].address = 0;
        roms->rom[rom].length = 0;
        roms->rom[rom].memory = NULL;
    }
    return 1;
}

void rom_add(ROMS *roms, uint8_t rom_num, uint32_t address, uint32_t length, uint8_t *memory) {
    assert(rom_num < roms->num_roms);
    ROM *r = &roms->rom[rom_num];
    r->address = address;
    r->length = length;
    r->memory = memory;
}

// Configure PAGES
uint8_t pages_init(PAGES *pages, uint16_t num_pages) {
    int page;
    if (!(pages->pages = (PAGE *)malloc(sizeof(PAGE) * num_pages))) {
        return (pages->num_pages = 0);
    }
    pages->num_pages = num_pages;
    for (page = 0; page < num_pages; page++) {
        pages->pages[page].memory = NULL;
    }
    return 1;
}

void pages_map(PAGES *pages, uint32_t start_page, uint32_t num_pages, uint8_t *memory) {
    assert(start_page + num_pages <= pages->num_pages);
    while (num_pages) {
        pages->pages[start_page++].memory = memory;
        memory += PAGE_SIZE;
        num_pages--;
    }
}

// Init the 6502
void cpu_init(CPU *cpu) {
    cpu->pc = 0xfffc;
    cpu->sp = 0x100;
    cpu->A = cpu->X = cpu->Y = 0;
    cpu->flags = 0;
    cpu->page_fault = 0;
    cpu->instruction = 0x4C;                                        // JMP oper
    cpu->cycles = 1;
    cpu->address_16 = cpu->scratch_16 = 0;
}
