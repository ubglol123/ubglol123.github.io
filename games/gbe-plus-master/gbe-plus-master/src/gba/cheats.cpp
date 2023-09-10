// GB Enhanced+ Copyright Daniel Baxter 2018
// Licensed under the GPLv2
// See LICENSE.txt for full license text

// File : cheats.cpp
// Date : May 06, 2018
// Description : Game Boy Advance cheat code management
//
// Decrypts GSAv1 codes
// Writes to ROM or RAM as needed by each cheat

#include "mmu.h"

/****** Decrypts Gameshark Advance (GSA) cheats ******/
void AGB_MMU::decrypt_gsa(u32 &addr, u32 &val, bool v1)
{
	//Set encryption seeds
	u32 s0 = v1 ? 0x09F4FBBD : 0x7AA9648F;
	u32 s1 = v1 ? 0x9681884A : 0x7FAE6994;
	u32 s2 = v1 ? 0x352027E9 : 0xC0EFAAD5;
	u32 s3 = v1 ? 0xF3DEE5A7 : 0x42712C57;

	//Decrypt cheat code bytes
	for(u32 x = 32; x > 0; x--)
	{
		u32 p1 = ((addr * 16) + s2);
		u32 p2 = (addr + (x * 0x9E3779B9));
		u32 p3 = ((addr / 32) + s3);
		u32 p4 = p1 ^ p2 ^ p3;
		val = val - p4;

		p1 = ((val * 16) + s0);
		p2 = (val + (x * 0x9E3779B9));
		p3 = ((val / 32) + s1);
		p4 = p1 ^ p2 ^ p3;
		addr = addr - p4;
	}
}

/****** Applies cheats when running emulation core ******/
void AGB_MMU::set_cheats()
{
	for(u32 x = 0; x < cheat_bytes.size(); x += 2)
	{
		//Grab two 32-bit values for GSA cheat
		u32 addr = cheat_bytes[x];
		u32 val = cheat_bytes[x + 1];

		//Ignore Master Enable
		if((val & 0xFFFFFF) == 0x1DC0DE) { addr = 0xFFFFFFFF; } 

		process_cheats(addr, val, x);
	}
}

/****** Processes specific cheat code ******/
void AGB_MMU::process_cheats(u32 a, u32 v, u32& index)
{
	u16 c = 0;

	//GSA cheat commands
	switch(a >> 28)
	{
		//8-bit RAM Write
		case 0x0:
			a &= 0xFFFFFFF;
			v &= 0xFF;
			write_u8(a, v);

			break;

		//16-bit RAM
		case 0x1:
			a &= 0xFFFFFFF;
			v &= 0xFFFF;
			write_u16(a, v);

			break;

		//32-bit RAM
		case 0x2:
			a &= 0xFFFFFFF;
			write_u32(a, v);

			break;

		//Write to list
		case 0x3:
			c = (a & 0xFFFF) - 1;

			while(v != 0)
			{			
				if((index + 2) <= cheat_bytes.size())
				{
					index += 2;
						
					write_u32(cheat_bytes[index], v);
					if(cheat_bytes[index + 1]) { write_u32(cheat_bytes[index + 1], v); }
					else { v = 0; }
				}

				else { v = 0; }
			}

			break;

		//ROM Patch
		case 0x6:
			if(gsa_patch_count < 1)
			{
				a &= 0xFFFFFF;
				a <<= 1;
				a += 0x8000000;
				v &= 0xFFFF;
					
				write_u16(a, v);
				gsa_patch_count++;
			}

			break;

		//IF-THEN
		//Change Seeds
		case 0xD:
			if(a == 0xDEADFACE) { }

			else
			{
				if((read_u16(a & 0xFFFFFFF) == (v & 0xFFFF)) && ((index + 2) <= cheat_bytes.size()))
				{
					index += 2;
					a = cheat_bytes[index];
					v = cheat_bytes[index + 1];

					process_cheats(a, v, index);
				}
			}

			break;

		//Hook Routine (Ignore)
		case 0xF:
			break;

		default:
				std::cout<<"MMU::Unhandled GSA command -> 0x" << a << "\n";
	}
}
