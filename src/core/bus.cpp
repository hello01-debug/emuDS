#include "bus.h"

#include <src/core/arm9/arm9.h>

uint8_t* arm9_bios;
uint8_t* arm7_bios;

size_t arm9_bios_size;

uint32_t dtcm_start = 0;
bool ime = false;

bool postflg_arm9 = false;

uint8_t dtcm[0x4000];
uint8_t* arm9_ram;

uint8_t ipcsync; // Used for synchronization primitives between ARM9 and ARM7

void Bus::AddARMBios(std::string file_name, bool is_arm9)
{
    std::ifstream bios(file_name, std::ios::ate | std::ios::binary);

    size_t size = bios.tellg();
    bios.seekg(0, std::ios::beg);

    if (is_arm9)
    {
        arm9_bios = new uint8_t[size];
        arm9_bios_size = size;
        bios.read((char*)arm9_bios, size);
    }
    else
    {
        arm7_bios = new uint8_t[size];
        bios.read((char*)arm7_bios, size);
    }

	arm9_ram = new uint8_t[4*1024*1024];
}

void Bus::Write32(uint32_t addr, uint32_t data)
{
	if (addr >= dtcm_start && addr < dtcm_start + 0x4000)
	{
		*(uint32_t*)&dtcm[addr - dtcm_start] = data;
		return;
	}
	if ((addr & 0xFF000000) == 0x02000000)
	{
		*(uint32_t*)&arm9_ram[addr & 0x3FFFFF] = data;
		return;
	}

    switch (addr)
    {
    case 0x040001A0: // Ignore Gamecard ROM and SPI control
    case 0x040001A4: // Ignore Gamecard bus ROMCTRL
        return;
    }

    printf("[emu/Bus]: Write32 0x%08x to unknown address 0x%08x\n", data, addr);
    exit(1);
}

void Bus::Write16(uint32_t addr, uint16_t data)
{
	if ((addr & 0xFF000000) == 0x02000000)
	{
		printf("Writing to RAM\n");
		*(uint16_t*)&arm9_ram[addr & 0x3FFFFF] = data;
		return;
	}
	
	switch (addr)
	{
	case 0x04000180:
		ipcsync = data & 0xF;
		return;
	case 0x04000204: // Ignore EXMEMCNT
		return;
	}

    printf("[emu/Bus]: Write16 0x%04x to unknown address 0x%08x\n", data, addr);
    exit(1);
}

void Bus::Write8(uint32_t addr, uint8_t data)
{
	switch (addr)
	{
	case 0x04000208:
		ime = data & 1;
		return;
	}

    printf("[emu/Bus]: Write8 0x%02x to unknown address 0x%08x\n", data, addr);
    exit(1);
}

uint32_t Bus::Read32(uint32_t addr)
{
    if (addr >= 0xFFFF0000 && addr < 0xFFFF0000 + arm9_bios_size)
        return *(uint32_t*)&arm9_bios[addr - 0xFFFF0000];
	if (addr >= dtcm_start && addr < dtcm_start + 0x4000)
		return *(uint32_t*)&dtcm[addr - dtcm_start];
	if ((addr & 0xFF000000) == 0x02000000)
	{
		printf("Reading from addr 0x%08x\n", addr);
		return *(uint32_t*)&arm9_ram[addr & 0x3FFFFF];
	}

    printf("[emu/Bus]: Read32 from unknown address 0x%08x\n", addr);
    ARM9::Dump();
	exit(1);
}

uint16_t Bus::Read16(uint32_t addr)
{
	if (addr >= 0xFFFF0000 && addr < 0xFFFF0000 + arm9_bios_size)
        return *(uint16_t*)&arm9_bios[addr - 0xFFFF0000];
	if ((addr & 0xFF000000) == 0x02000000)
		return *(uint16_t*)&arm9_ram[addr & 0x3FFFFF];

    printf("[emu/Bus]: Read16 from unknown address 0x%08x\n", addr);
    exit(1);
}

uint8_t Bus::Read8(uint32_t addr)
{

    switch (addr)
    {
    case 0x04000300:
        return postflg_arm9;
    }

    printf("[emu/Bus]: Read8 from unknown address 0x%08x\n", addr);
    exit(1);
}

void Bus::RemapDTCM(uint32_t addr)
{
	dtcm_start = addr;
}

void Bus::Dump()
{
	std::ofstream out("ram.dump");

	for (int i = 0; i < 4*1024*1024; i++)
	{
		out << arm9_ram[i];
	}

	out.close();
}
