// GB Enhanced+ Copyright Daniel Baxter 2015
// Licensed under the GPLv2
// See LICENSE.txt for full license text

// File : arm9.cpp
// Date : November 05, 2015
// Description : ARM946E-S emulator
//
// Emulates an ARM946E-S CPU in software
// This is the primary CPU of the DS (NDS9 - Video)

#include "arm9.h"

/****** CPU Constructor ******/
NTR_ARM9::NTR_ARM9()
{
	reset();
}

/****** CPU Destructor ******/
NTR_ARM9::~NTR_ARM9()
{
	std::cout<<"CPU::ARM9 - Shutdown\n";
}

/****** CPU Reset ******/
void NTR_ARM9::reset()
{
	reg.r0 = reg.r1 = reg.r2 = reg.r3 = reg.r4 = reg.r5 = reg.r6 = reg.r7 = reg.r8 = reg.r9 = reg.r10 = reg.r11 = reg.r12 = reg.r14 = 0;

	//Set default values for some registers if not booting from the NDS firmware
	if(!config::use_bios || !config::use_firmware)
	{
		reg.r13 = reg.r13_fiq = reg.r13_abt = reg.r13_und = 0x3002F7C;
		reg.r13_svc = 0x3003FC0;
		reg.r13_irq = 0x3003F80;
		reg.cpsr = 0x5F;
		reg.r15 = 0;
		current_cpu_mode = SYS;
	}

	//Otherwise, init registers as zero (except PC), CPSR in SVC mode with IRQ and FIQ bits set
	else
	{
		reg.r13 = reg.r13_fiq = reg.r13_abt = reg.r13_und = 0;
		reg.r13_svc = 0;
		reg.r13_irq = 0;
		reg.cpsr = 0xD3;
		reg.r15 = 0xFFFF0000;
		current_cpu_mode = SVC;
	}

	reg.r8_fiq = reg.r9_fiq = reg.r10_fiq = reg.r11_fiq = reg.r12_fiq = reg.r14_fiq = reg.spsr_fiq = 0;
	reg.r14_svc = reg.spsr_svc = 0;
	reg.r14_abt = reg.spsr_abt = 0;
	reg.r14_irq = reg.spsr_irq = 0;
	reg.r14_und = reg.spsr_und = 0;

	lbl_addr = 0;
	first_branch = false;

	running = false;
	in_interrupt = false;

	idle_state = 0;
	last_idle_state = 0;

	thumb_long_branch = false;
	last_instr_branch = false;
	swi_waitbyloop_count = 0;

	arm_mode = ARM;

	controllers.timer.clear();
	controllers.timer.resize(4);

	for(int x = 0; x < 4; x++)
	{
		controllers.timer[x].cnt = 0;
		controllers.timer[x].counter = 0;
		controllers.timer[x].reload_value = 0;
		controllers.timer[x].prescalar = 1;
		controllers.timer[x].clock = 0;
		controllers.timer[x].count_up = false;
		controllers.timer[x].enable = false;
	}

	//Setup CPU access timing
	setup_cpu_timing();

	debug_message = 0xFF;
	debug_code = 0;
	debug_cycles = 0;
	debug_addr = 0;

	sync_cycles = 0;
	system_cycles = 0;
	re_sync = false;

	flush_pipeline();
	mem = NULL;

	co_proc.reset();

	std::cout<<"CPU::ARM9 - Initialized\n";
}

/****** Sets up CPU access timing ******/
void NTR_ARM9::setup_cpu_timing()
{
	//Populate default timings before setting specific timings
	for(u32 x = 0; x < 16; x++)
	{
		for(u32 y = 0; y < 8; y++)
		{
			cpu_timing[x][y] = 2;
		}
	}

	//Main Memory
	cpu_timing[2][CODE_N16] = 4;
	cpu_timing[2][CODE_S16] = 4;
	cpu_timing[2][CODE_N32] = 9;
	cpu_timing[2][CODE_S32] = 9;
	cpu_timing[2][DATA_N16] = 9;
	cpu_timing[2][DATA_S16] = 1;
	cpu_timing[2][DATA_N32] = 10;
	cpu_timing[2][DATA_S32] = 2;

	//WRAM, BIOS, I/O, OAM
	cpu_timing[3][CODE_N16] = 2;
	cpu_timing[3][CODE_S16] = 2;
	cpu_timing[3][CODE_N32] = 4;
	cpu_timing[3][CODE_S32] = 4;
	cpu_timing[3][DATA_N16] = 4;
	cpu_timing[3][DATA_S16] = 1;
	cpu_timing[3][DATA_N32] = 4;
	cpu_timing[3][DATA_S32] = 1;

	cpu_timing[4][CODE_N16] = 2;
	cpu_timing[4][CODE_S16] = 2;
	cpu_timing[4][CODE_N32] = 4;
	cpu_timing[4][CODE_S32] = 4;
	cpu_timing[4][DATA_N16] = 4;
	cpu_timing[4][DATA_S16] = 1;
	cpu_timing[4][DATA_N32] = 4;
	cpu_timing[4][DATA_S32] = 1;

	cpu_timing[7][CODE_N16] = 2;
	cpu_timing[7][CODE_S16] = 2;
	cpu_timing[7][CODE_N32] = 4;
	cpu_timing[7][CODE_S32] = 4;
	cpu_timing[7][DATA_N16] = 4;
	cpu_timing[7][DATA_S16] = 1;
	cpu_timing[7][DATA_N32] = 4;
	cpu_timing[7][DATA_S32] = 1;

	//VRAM, Palettes
	cpu_timing[5][CODE_N16] = 2;
	cpu_timing[5][CODE_S16] = 2;
	cpu_timing[5][CODE_N32] = 5;
	cpu_timing[5][CODE_S32] = 5;
	cpu_timing[5][DATA_N16] = 4;
	cpu_timing[5][DATA_S16] = 1;
	cpu_timing[5][DATA_N32] = 5;
	cpu_timing[5][DATA_S32] = 2;

	//VRAM, Palettes
	cpu_timing[6][CODE_N16] = 2;
	cpu_timing[6][CODE_S16] = 2;
	cpu_timing[6][CODE_N32] = 5;
	cpu_timing[6][CODE_S32] = 5;
	cpu_timing[6][DATA_N16] = 4;
	cpu_timing[6][DATA_S16] = 1;
	cpu_timing[6][DATA_N32] = 5;
	cpu_timing[6][DATA_S32] = 2;
}

/****** CPU register getter - Returns value from the CURRENT pipeline stage ******/
u32 NTR_ARM9::get_reg(u8 g_reg) const
{
	switch(g_reg)
	{
		case 0: return reg.r0; break;
		case 1: return reg.r1; break;
		case 2: return reg.r2; break;
		case 3: return reg.r3; break;
		case 4: return reg.r4; break;
		case 5: return reg.r5; break;
		case 6: return reg.r6; break;
		case 7: return reg.r7; break;
		
		case 8: 
			switch(current_cpu_mode)
			{
				case FIQ: return reg.r8_fiq; break;
				default: return reg.r8; break;
			}
			break;

		case 9: 
			switch(current_cpu_mode)
			{
				case FIQ: return reg.r9_fiq; break;
				default: return reg.r9; break;
			}
			break;

		case 10: 
			switch(current_cpu_mode)
			{
				case FIQ: return reg.r10_fiq; break;
				default: return reg.r10; break;
			}
			break;

		case 11: 
			switch(current_cpu_mode)
			{
				case FIQ: return reg.r11_fiq; break;
				default: return reg.r11; break;
			}
			break;

		case 12: 
			switch(current_cpu_mode)
			{
				case FIQ: return reg.r12_fiq; break;
				default: return reg.r12; break;
			}
			break;

		case 13: 
			switch(current_cpu_mode)
			{
				case USR:
				case SYS: return reg.r13; break;
				case FIQ: return reg.r13_fiq; break;
				case SVC: return reg.r13_svc; break;
				case ABT: return reg.r13_abt; break;
				case IRQ: return reg.r13_irq; break;
				case UND: return reg.r13_und; break;
			}
			break;

		case 14: 
			switch(current_cpu_mode)
			{
				case USR:
				case SYS: return reg.r14; break;
				case FIQ: return reg.r14_fiq; break;
				case SVC: return reg.r14_svc; break;
				case ABT: return reg.r14_abt; break;
				case IRQ: return reg.r14_irq; break;
				case UND: return reg.r14_und; break;
			}
			break;

		case 15: return reg.r15; break;

		//This should not happen
		default:
			std::cout<<"CPU::Error - ARM9 tried to access invalid general purpose register: " << (int)g_reg << "\n"; break;
	}

	return 0;
}

/****** CPU register setter ******/
void NTR_ARM9::set_reg(u8 s_reg, u32 value)
{
	switch(s_reg)
	{
		case 0: reg.r0 = value; break;
		case 1: reg.r1 = value; break;
		case 2: reg.r2 = value; break;
		case 3: reg.r3 = value; break;
		case 4: reg.r4 = value; break;
		case 5: reg.r5 = value; break;
		case 6: reg.r6 = value; break;
		case 7: reg.r7 = value; break;
		
		case 8: 
			switch(current_cpu_mode)
			{
				case FIQ: reg.r8_fiq = value; break;
				default: reg.r8 = value; break;
			}
			break;

		case 9: 
			switch(current_cpu_mode)
			{
				case FIQ: reg.r9_fiq = value; break;
				default: reg.r9 = value; break;
			}
			break;

		case 10: 
			switch(current_cpu_mode)
			{
				case FIQ: reg.r10_fiq = value; break;
				default: reg.r10 = value; break;
			}
			break;

		case 11: 
			switch(current_cpu_mode)
			{
				case FIQ: reg.r11_fiq = value; break;
				default: reg.r11 = value; break;
			}
			break;

		case 12: 
			switch(current_cpu_mode)
			{
				case FIQ: reg.r12_fiq = value; break;
				default: reg.r12 = value; break;
			}
			break;

		case 13: 
			switch(current_cpu_mode)
			{
				case USR:
				case SYS: reg.r13 = value; break;
				case FIQ: reg.r13_fiq = value; break;
				case SVC: reg.r13_svc = value; break;
				case ABT: reg.r13_abt = value; break;
				case IRQ: reg.r13_irq = value; break;
				case UND: reg.r13_und = value; break;
			}
			break;

		case 14: 
			switch(current_cpu_mode)
			{
				case USR:
				case SYS: reg.r14 = value; break;
				case FIQ: reg.r14_fiq = value; break;
				case SVC: reg.r14_svc = value; break;
				case ABT: reg.r14_abt = value; break;
				case IRQ: reg.r14_irq = value; break;
				case UND: reg.r14_und = value; break;
			}
			break;

		case 15: reg.r15 = value; break;

		//This should not happen
		default:
			std::cout<<"CPU::Error - ARM9 tried to access invalid general purpose register: " << (int)s_reg << "\n"; break;
	}
}

/****** Saved Program Status Register getter ******/
u32 NTR_ARM9::get_spsr() const
{
	switch(current_cpu_mode)
	{
		case USR:
		case SYS: return reg.cpsr; break;
		case FIQ: return reg.spsr_fiq; break;
		case SVC: return reg.spsr_svc; break;
		case ABT: return reg.spsr_abt; break;
		case IRQ: return reg.spsr_irq; break;
		case UND: return reg.spsr_und; break;
		default: std::cout<<"CPU::Error - ARM9 tried to access invalid SPSR in mode 0x" << std::hex << (int)current_cpu_mode << "\n"; break;
	}

	return 0;
}

/****** Saved Program Status Register setter ******/
void NTR_ARM9::set_spsr(u32 value)
{
	switch(current_cpu_mode)
	{
		case USR:
		case SYS: break;
		case FIQ: reg.spsr_fiq = value; break;
		case SVC: reg.spsr_svc = value; break;
		case ABT: reg.spsr_abt = value; break;
		case IRQ: reg.spsr_irq = value; break;
		case UND: reg.spsr_und = value; break;
		default: std::cout<<"CPU::Error - ARM9 tried to access invalid SPSR in mode 0x" << std::hex << (int)current_cpu_mode << "\n"; break;
	}
}

/****** Fetch ARM instruction ******/
void NTR_ARM9::fetch()
{
	mem->fetch_request = true;

	//Fetch THUMB instructions
	if(arm_mode == THUMB)
	{
		//Read 16-bit THUMB instruction
		instruction_pipeline[pipeline_pointer] = mem->read_u16(reg.r15);

		//Set the operation to perform as UNDEFINED until decoded
		instruction_operation[pipeline_pointer] = UNDEFINED;
	}

	//Fetch ARM instructions
	else if(arm_mode == ARM)
	{
		//Read 32-bit ARM instruction
		instruction_pipeline[pipeline_pointer] = mem->read_u32(reg.r15);

		//Set the operation to perform as UNDEFINED until decoded
		instruction_operation[pipeline_pointer] = UNDEFINED;
	}

	mem->fetch_request = false;
}

/****** Decode ARM instruction ******/
void NTR_ARM9::decode()
{
	u8 pipeline_id = (pipeline_pointer + 2) % 3;

	if(instruction_operation[pipeline_id] == PIPELINE_FILL) { return; }

	//Decode THUMB instructions
	if(arm_mode == THUMB)
	{
		u16 current_instruction = instruction_pipeline[pipeline_id];
		
		if(((current_instruction >> 13) == 0) && (((current_instruction >> 11) & 0x7) != 0x3))
		{
			//THUMB_1
			instruction_operation[pipeline_id] = THUMB_1;
		}

		else if(((current_instruction >> 11) & 0x1F) == 0x3)
		{
			//THUMB_2
			instruction_operation[pipeline_id] = THUMB_2;
		}

		else if((current_instruction >> 13) == 0x1)
		{
			//THUMB_3
			instruction_operation[pipeline_id] = THUMB_3;
		}

		else if(((current_instruction >> 10) & 0x3F) == 0x10)
		{
			//THUMB_4
			instruction_operation[pipeline_id] = THUMB_4;
		}

		else if(((current_instruction >> 10) & 0x3F) == 0x11)
		{
			//THUMB_5
			instruction_operation[pipeline_id] = THUMB_5;
		}

		else if((current_instruction >> 11) == 0x9)
		{
			//THUMB_6
			instruction_operation[pipeline_id] = THUMB_6;
		}

		else if((current_instruction >> 12) == 0x5)
		{
			if(current_instruction & 0x200)
			{
				//THUMB_8
				instruction_operation[pipeline_id] = THUMB_8;
			}

			else
			{
				//THUMB_7
				instruction_operation[pipeline_id] = THUMB_7;
			}
		}

		else if(((current_instruction >> 13) & 0x7) == 0x3)
		{
			//THUMB_9
			instruction_operation[pipeline_id] = THUMB_9;
		}

		else if((current_instruction >> 12) == 0x8)
		{
			//THUMB_10
			instruction_operation[pipeline_id] = THUMB_10;
		}

		else if((current_instruction >> 12) == 0x9)
		{
			//THUMB_11
			instruction_operation[pipeline_id] = THUMB_11;
		}

		else if((current_instruction >> 12) == 0xA)
		{
			//THUMB_12
			instruction_operation[pipeline_id] = THUMB_12;
		}

		else if((current_instruction >> 8) == 0xB0)
		{
			//THUMB_13
			instruction_operation[pipeline_id] = THUMB_13;
		}

		else if((current_instruction >> 12) == 0xB)
		{
			//THUMB_14
			instruction_operation[pipeline_id] = THUMB_14;
		}

		else if((current_instruction >> 12) == 0xC)
		{
			//THUMB_15
			instruction_operation[pipeline_id] = THUMB_15;
		}

		else if((current_instruction >> 12) == 13)
		{
			//THUMB_16
			instruction_operation[pipeline_id] = THUMB_16;
		}

		else if((current_instruction >> 11) == 0x1C)
		{
			//THUMB_18
			instruction_operation[pipeline_id] = THUMB_18;
		}

		else if((current_instruction >> 11) >= 0x1E)
		{
			//THUMB_19
			instruction_operation[pipeline_id] = THUMB_19;
		}

		else if((current_instruction & 0xF800) == 0xE800)
		{
			//THUMB_19 BLX
			instruction_operation[pipeline_id] = THUMB_19;
		}
	}

	//Decode ARM instructions
	if(arm_mode == ARM)
	{
		u32 current_instruction = instruction_pipeline[pipeline_id];

		if(((current_instruction >> 8) & 0xFFFFF) == 0x12FFF)
		{
			//ARM_3
			instruction_operation[pipeline_id] = ARM_3;
		}

		else if(((current_instruction >> 25) & 0x7) == 0x5)
		{
			//ARM_4
			instruction_operation[pipeline_id] = ARM_4;
		}		

		else if((current_instruction & 0xD900000) == 0x1000000) 
		{

			if((((current_instruction >> 16) & 0xFFF) == 0x16F) && (((current_instruction >> 4) & 0xFF) == 0xF1))
			{
				//ARM CLZ
				instruction_operation[pipeline_id] = ARM_CLZ;
			}

			else if((((current_instruction >> 24) & 0xF) == 0x1) && (((current_instruction >> 4) & 0xFF) == 0x5))
			{
				//ARM QADD-QSUB
				instruction_operation[pipeline_id] = ARM_QADD_QSUB;
			}

			else if((current_instruction & 0x80) && (current_instruction & 0x10) && ((current_instruction & 0x2000000) == 0))
			{
				if(((current_instruction >> 5) & 0x3) == 0) 
				{ 
					instruction_operation[pipeline_id] = ARM_12;
				}

				else 
				{
					instruction_operation[pipeline_id] = ARM_10;
				}
			}

			else 
			{
				//ARM7
				if((current_instruction & 0x80) && ((current_instruction & 0x2000000) == 0))
				{
					instruction_operation[pipeline_id] = ARM_7;
				}

				//ARM_6
				else
				{
					instruction_operation[pipeline_id] = ARM_6;
				}
			}
		}

		else if(((current_instruction >> 26) & 0x3) == 0x0)
		{
			if((current_instruction & 0x80) && ((current_instruction & 0x10) == 0))
			{
				//ARM.5
				if(current_instruction & 0x2000000)
				{
					instruction_operation[pipeline_id] = ARM_5;
				}

				//ARM.5
				else if((current_instruction & 0x100000) && (((current_instruction >> 23) & 0x3) == 0x2))
				{
					instruction_operation[pipeline_id] = ARM_5;
				}

				//ARM.5
				else if(((current_instruction >> 23) & 0x3) != 0x2)
				{
					instruction_operation[pipeline_id] = ARM_5;
				}

				//ARM.7
				else
				{
					instruction_operation[pipeline_id] = ARM_7;
				}
			}

			else if((current_instruction & 0x80) && (current_instruction & 0x10))
			{
				if(((current_instruction >> 4) & 0xF) == 0x9)
				{
					//ARM.5
					if(current_instruction & 0x2000000)
					{
						instruction_operation[pipeline_id] = ARM_5;
					}

					//ARM.12
					else if(((current_instruction >> 23) & 0x3) == 0x2)
					{
						instruction_operation[pipeline_id] = ARM_12;
					}

					//ARM.7
					else
					{
						instruction_operation[pipeline_id] = ARM_7;
					}
				}

				//ARM.5
				else if(current_instruction & 0x2000000)
				{
					instruction_operation[pipeline_id] = ARM_5;
				}

				//ARM.10
				else
				{
					instruction_operation[pipeline_id] = ARM_10;
				}
			}

			//ARM.5
			else
			{
				instruction_operation[pipeline_id] = ARM_5;
			}
		}

		else if(((current_instruction >> 26) & 0x3) == 0x1)
		{
			//ARM_9
			instruction_operation[pipeline_id] = ARM_9;
		}

		else if(((current_instruction >> 25) & 0x7) == 0x4)
		{
			//ARM_11
			instruction_operation[pipeline_id] = ARM_11;
		}

		else if(((current_instruction >> 24) & 0xF) == 0xF)
		{
			//ARM_13
			instruction_operation[pipeline_id] = ARM_13;
		}

		else if(((current_instruction >> 24) & 0xF) == 0xE)
		{
			//ARM Coprocessor Register Transfer
			if(current_instruction & 0x10) { instruction_operation[pipeline_id] = ARM_COP_REG_TRANSFER; }
			
			//ARM Coprocessor Data Operation
			else { instruction_operation[pipeline_id] = ARM_COP_DATA_OP; }
		}

		else if(((current_instruction >> 25) & 0x7) == 6)
		{
			//ARM Coprocessor Data Transfer
			instruction_operation[pipeline_id] = ARM_COP_DATA_TRANSFER;
		}
	}
}

/****** Execute ARM instruction ******/
void NTR_ARM9::execute()
{
	u8 pipeline_id = (pipeline_pointer + 1) % 3;

	if(instruction_operation[pipeline_id] == PIPELINE_FILL) 
	{
		debug_message = 0xFF; 
		return; 
	}

	//Execute THUMB instruction
	if(arm_mode == THUMB)
	{
		debug_addr = (reg.r15 - 4);

		switch(instruction_operation[pipeline_id])
		{
			case THUMB_1:
				move_shifted_register(instruction_pipeline[pipeline_id]);
				debug_message = 0x0; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_2:
				add_sub_immediate(instruction_pipeline[pipeline_id]);
				debug_message = 0x1; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_3:
				mcas_immediate(instruction_pipeline[pipeline_id]);
				debug_message = 0x2; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_4:
				alu_ops(instruction_pipeline[pipeline_id]);
				debug_message = 0x3; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_5:
				hireg_bx(instruction_pipeline[pipeline_id]);
				debug_message = 0x4; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_6:
				load_pc_relative(instruction_pipeline[pipeline_id]);
				debug_message = 0x5; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_7:
				load_store_reg_offset(instruction_pipeline[pipeline_id]);
				debug_message = 0x6; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_8:
				load_store_sign_ex(instruction_pipeline[pipeline_id]);
				debug_message = 0x7; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_9:
	 			load_store_imm_offset(instruction_pipeline[pipeline_id]);
				debug_message = 0x8; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_10:
				load_store_halfword(instruction_pipeline[pipeline_id]);
				debug_message = 0x9; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_11:
				load_store_sp_relative(instruction_pipeline[pipeline_id]);
				debug_message = 0xA; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_12:
				get_relative_address(instruction_pipeline[pipeline_id]);
				debug_message = 0xB; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_13:
				add_offset_sp(instruction_pipeline[pipeline_id]);
				debug_message = 0xC; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_14:
				push_pop(instruction_pipeline[pipeline_id]);
				debug_message = 0xD; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_15:
				multiple_load_store(instruction_pipeline[pipeline_id]);
				debug_message = 0xE; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_16:
				conditional_branch(instruction_pipeline[pipeline_id]);
				debug_message = 0xF; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_18:
				unconditional_branch(instruction_pipeline[pipeline_id]);
				debug_message = 0x11; debug_code = instruction_pipeline[pipeline_id];
				break;

			case THUMB_19:
				long_branch_link(instruction_pipeline[pipeline_id]);
				debug_message = 0x12; debug_code = instruction_pipeline[pipeline_id];
				break;

			default:
				debug_message = 0x13; debug_code = instruction_pipeline[pipeline_id];
				std::cout<<"CPU::NTR_ARM9::Error - Unknown THUMB instruction -> 0x" << std::hex << debug_code << "\n";
				if(!config::ignore_illegal_opcodes) { running = false; }
				break;
		}
	}

	//Execute ARM instruction
	else if(arm_mode == ARM)
	{
		debug_addr = (reg.r15 - 8);

		//Conditionally execute ARM instruction
		if(check_condition(instruction_pipeline[pipeline_id]))
		{

			switch(instruction_operation[pipeline_id])
			{
				case ARM_3:
					branch_exchange(instruction_pipeline[pipeline_id]);
					debug_message = 0x14; debug_code = instruction_pipeline[pipeline_id];
					break;

				case ARM_4:
					branch_link(instruction_pipeline[pipeline_id]);
					debug_message = 0x15; debug_code = instruction_pipeline[pipeline_id];
					break;

				case ARM_5:
					data_processing(instruction_pipeline[pipeline_id]);
					debug_message = 0x16; debug_code = instruction_pipeline[pipeline_id];
					break;

				case ARM_6:
					psr_transfer(instruction_pipeline[pipeline_id]);
					debug_message = 0x17; debug_code = instruction_pipeline[pipeline_id];
					break;

				case ARM_7:
					multiply(instruction_pipeline[pipeline_id]);
					debug_message = 0x18; debug_code = instruction_pipeline[pipeline_id];
					break;

				case ARM_9:
					single_data_transfer(instruction_pipeline[pipeline_id]);
					debug_message = 0x19; debug_code = instruction_pipeline[pipeline_id];
					break;

				case ARM_10:
					halfword_signed_transfer(instruction_pipeline[pipeline_id]);
					debug_message = 0x1A; debug_code = instruction_pipeline[pipeline_id];
					break;

				case ARM_11:
					block_data_transfer(instruction_pipeline[pipeline_id]);
					debug_message = 0x1B; debug_code = instruction_pipeline[pipeline_id];
					break;

				case ARM_12:
					single_data_swap(instruction_pipeline[pipeline_id]);
					debug_message = 0x1C; debug_code = instruction_pipeline[pipeline_id];
					break;

				case ARM_13:
					software_interrupt_breakpoint(instruction_pipeline[pipeline_id]);
					debug_message = 0x1D; debug_code = instruction_pipeline[pipeline_id];
					break;

				case ARM_COP_REG_TRANSFER:
					coprocessor_register_transfer(instruction_pipeline[pipeline_id]);
					debug_message = 0x1E; debug_code = instruction_pipeline[pipeline_id];
					break;

				case ARM_COP_DATA_TRANSFER:
					std::cout<<"CPU::ARM9::Warning - LDC/STC unimplemented\n";
					debug_message = 0x1F; debug_code = instruction_pipeline[pipeline_id];
					running = false;
					break;

				case ARM_COP_DATA_OP:
					std::cout<<"CPU::ARM9::Warning - CDP unimplemented\n";
					debug_message = 0x20; debug_code = instruction_pipeline[pipeline_id];
					running = false;
					break;

				case ARM_CLZ:
					count_leading_zeroes(instruction_pipeline[pipeline_id]);
					debug_message = 0x21; debug_code = instruction_pipeline[pipeline_id];
					break;

				case ARM_QADD_QSUB:
					sticky_math(instruction_pipeline[pipeline_id]);
					debug_message = 0x22; debug_code = instruction_pipeline[pipeline_id];
					break;

				default:
					debug_message = 0x23; debug_code = instruction_pipeline[pipeline_id];
					std::cout<<"CPU::ARM9::Error - Unknown ARM instruction -> 0x" << std::hex << debug_code << "\n";
					if(!config::ignore_illegal_opcodes) { running = false; }
					break;
			}
		}

		//Skip ARM instruction
		else 
		{ 
			debug_message = 0x24; 
			debug_code = instruction_pipeline[pipeline_id];

			//Clock CPU and controllers - 1S
			clock(reg.r15, CODE_S32); 
		}
	}

}	

/****** Flush the pipeline - Called when branching or resetting ******/
void NTR_ARM9::flush_pipeline()
{
	needs_flush = false;
	pipeline_pointer = 0;
	instruction_pipeline[0] = instruction_pipeline[1] = instruction_pipeline[2];
	instruction_operation[0] = instruction_operation[1] = instruction_operation[2] = PIPELINE_FILL;
}

/****** Updates the PC after each fetch-decode-execute ******/
void NTR_ARM9::update_pc()
{
	reg.r15 += (arm_mode == ARM) ? 4 : 2;
}

/****** Check conditional code ******/
bool NTR_ARM9::check_condition(u32 current_arm_instruction) const
{
	switch(current_arm_instruction >> 28)
	{
		//EQ
		case 0x0:
			if(reg.cpsr & CPSR_Z_FLAG) { return true; }
			else { return false; }
			break;

		//NE
		case 0x1:
			if(reg.cpsr & CPSR_Z_FLAG) { return false; }
			else { return true; }
			break;

		//CS
		case 0x2:
			if(reg.cpsr & CPSR_C_FLAG) { return true; }
			else { return false; }
			break;

		//CC
		case 0x3:
			if(reg.cpsr & CPSR_C_FLAG) { return false; }
			else { return true; }
			break;

		//MI
		case 0x4:
			if(reg.cpsr & CPSR_N_FLAG) { return true; }
			else { return false; }
			break;

		//PL
		case 0x5:
			if(reg.cpsr & CPSR_N_FLAG) { return false; }
			else { return true; }
			break;

		//VS
		case 0x6:
			if(reg.cpsr & CPSR_V_FLAG) { return true; }
			else { return false; }
			break;

		//VC
		case 0x7:
			if(reg.cpsr & CPSR_V_FLAG) { return false; }
			else { return true; }
			break;

		//HI
		case 0x8:
			if((reg.cpsr & CPSR_C_FLAG) && ((reg.cpsr & CPSR_Z_FLAG) == 0)) { return true; }
			else { return false; }
			break;

		//LS
		case 0x9:
			if((reg.cpsr & CPSR_Z_FLAG) || ((reg.cpsr & CPSR_C_FLAG) == 0)) { return true; }
			else { return false; }
			break;

		//GE
		case 0xA:
			{
				u8 n = (reg.cpsr & CPSR_N_FLAG) ? 1 : 0;
				u8 v = (reg.cpsr & CPSR_V_FLAG) ? 1 : 0;

				if(n == v) { return true; }
				else { return false; }
			}
			break;

		//LT
		case 0xB:
			{
				u8 n = (reg.cpsr & CPSR_N_FLAG) ? 1 : 0;
				u8 v = (reg.cpsr & CPSR_V_FLAG) ? 1 : 0;

				if(n != v) { return true; }
				else { return false; }
			}
			break;

		//GT
		case 0xC:
			{
				u8 n = (reg.cpsr & CPSR_N_FLAG) ? 1 : 0;
				u8 v = (reg.cpsr & CPSR_V_FLAG) ? 1 : 0;
				u8 z = (reg.cpsr & CPSR_Z_FLAG) ? 1 : 0;

				if((z == 0) && (n == v)) { return true; }
				else { return false; }
			}
			break;

		//LE
		case 0xD:
			{
				u8 n = (reg.cpsr & CPSR_N_FLAG) ? 1 : 0;
				u8 v = (reg.cpsr & CPSR_V_FLAG) ? 1 : 0;
				u8 z = (reg.cpsr & CPSR_Z_FLAG) ? 1 : 0;

				if((z == 1) || (n != v)) { return true; }
				else { return false; }
			}
			break;

		//AL
		case 0xE: return true; break;

		//NV
		default:
			if(instruction_pipeline[((pipeline_pointer + 1) % 3)] != ARM_4)
			{
				//std::cout<<"CPU::ARM9::Warning: ARM instruction uses reserved conditional code NV \n";
			}

			return true;
	}
}

/****** Updates the condition codes in the CPSR register after logical operations ******/
void NTR_ARM9::update_condition_logical(u32 result, u8 shift_out)
{
	//Negative flag
	if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
	else { reg.cpsr &= ~CPSR_N_FLAG; }

	//Zero flag
	if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
	else { reg.cpsr &= ~CPSR_Z_FLAG; }

	//Carry flag
	if(shift_out == 1) { reg.cpsr |= CPSR_C_FLAG; }
	else if(shift_out == 0) { reg.cpsr &= ~CPSR_C_FLAG; }
}

/****** Updates the condition codes in the CPSR register after arithmetic operations ******/
void NTR_ARM9::update_condition_arithmetic(u32 input, u32 operand, u32 result, bool addition)
{
	//Negative flag
	if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
	else { reg.cpsr &= ~CPSR_N_FLAG; }

	//Zero flag
	if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
	else { reg.cpsr &= ~CPSR_Z_FLAG; }

	//Carry flag - Addition
	if((operand > (0xFFFFFFFF - input)) && (addition)) { reg.cpsr |= CPSR_C_FLAG; }

	//Carry flag - Subtraction
	else if((operand <= input) && (!addition)) { reg.cpsr |= CPSR_C_FLAG; }

	else { reg.cpsr &= ~CPSR_C_FLAG; }

	//Overflow flag
	u8 input_msb = (input & 0x80000000) ? 1 : 0;
	u8 operand_msb = (operand & 0x80000000) ? 1 : 0;
	u8 result_msb = (result & 0x80000000) ? 1 : 0;

	if(addition)
	{
		if(input_msb != operand_msb) { reg.cpsr &= ~CPSR_V_FLAG; }
		
		else
		{
			if((result_msb == input_msb) && (result_msb == operand_msb)) { reg.cpsr &= ~CPSR_V_FLAG; }
			else { reg.cpsr |= CPSR_V_FLAG; }
		}
	}

	else
	{
		if(input_msb == operand_msb) { reg.cpsr &= ~CPSR_V_FLAG; }
		
		else
		{
			if(result_msb == operand_msb) { reg.cpsr |= CPSR_V_FLAG; }
			else { reg.cpsr &= ~CPSR_V_FLAG; }
		}
	}
}

/****** Updates the condition code in CPSR for Stick Overflow after QADD or QSUB operations ******/
u8 NTR_ARM9::update_sticky_overflow(u32 input, u32 operand, u32 result, bool addition)
{
	u8 input_msb = (input & 0x80000000) ? 1 : 0;
	u8 operand_msb = (operand & 0x80000000) ? 1 : 0;
	u8 result_msb = (result & 0x80000000) ? 1 : 0;
	u8 saturation_code = 0;

	if(addition)
	{
		if(!input_msb && !operand_msb && result_msb) { reg.cpsr |= CPSR_Q_FLAG; saturation_code = 1; }
		else if(input_msb && operand_msb && !result_msb) { reg.cpsr |= CPSR_Q_FLAG; saturation_code = 2; }
	}

	else
	{
		if(!input_msb && operand_msb && result_msb) { reg.cpsr |= CPSR_Q_FLAG; saturation_code = 1; }
		else if(input_msb && !operand_msb && !result_msb) { reg.cpsr |= CPSR_Q_FLAG; saturation_code = 2; }
	} 

	return saturation_code;
}

/****** Performs 32-bit logical shift left - Returns Carry Out ******/
u8 NTR_ARM9::logical_shift_left(u32& input, u8 offset)
{
	u8 carry_out = 0;

	if(offset > 0)
	{
		//Test for carry
		//Perform LSL #(n-1), if Bit 31 is 1, we know it will carry out
		u32 carry_test = input << (offset - 1);
		carry_out = (carry_test & 0x80000000) ? 1 : 0;

		if(offset >= 32)
		{
			input = 0;
			carry_out = (offset == 32) ? carry_out : 0;
		}

		else { input <<= offset; }
	}

	//LSL #0
	//No shift performed, carry flag not affected, set it to something not 0 or 1 to check!
	else { carry_out = 2; }

	return carry_out;
}

/****** Performs 32-bit logical shift right - Returns Carry Out ******/
u8 NTR_ARM9::logical_shift_right(u32& input, u8 offset)
{
	u8 carry_out = 0;

	if(offset > 0)
	{
		//Test for carry
		//Perform LSR #(n-1), if Bit 0 is 1, we know it will carry out
		u32 carry_test = input >> (offset - 1);
		carry_out = (carry_test & 0x1) ? 1 : 0;

		if(offset >= 32)
		{
			input = 0;
			carry_out = (offset == 32) ? carry_out : 0;
		}

		else { input >>= offset; }
	}

	//LSR #0
	//Same as LSR #32, input becomes zero, carry flag is Bit 31 of input
	else
	{
		carry_out = (input & 0x80000000) ? 1 : 0;
		input = 0;
	}

	return carry_out;
}

/****** Performs 32-bit arithmetic shift right - Returns Carry Out ******/
u8 NTR_ARM9::arithmetic_shift_right(u32& input, u8 offset)
{
	u8 carry_out = 0;

	if(offset > 0)
	{
		u8 high_bit = (input & 0x80000000) ? 1 : 0;		

		//Basically LSR, but bits become Bit 31
		for(int x = 0; x < offset; x++)
		{
			carry_out = (input & 0x1) ? 1 : 0;
			input >>= 1;
			if(high_bit == 1) { input |= 0x80000000; }
		}	
	}

	//ASR #0
	//Same as ASR #32, input becomes 0xFFFFFFFF or 0x0 depending on Bit 31 of input
	//Carry flag set to 0 or 1 depending on Bit 31 of input
	else
	{
		if(input & 0x80000000) { input = 0xFFFFFFFF; carry_out = 1; }
		else { input = 0; carry_out = 0; }
	}

	return carry_out;
}

/****** Performs 32-bit rotate right ******/
u8 NTR_ARM9::rotate_right(u32& input, u8 offset)
{
	u8 carry_out = 0;

	if(offset > 0)
	{
		//Perform ROR shift on immediate
		for(int x = 0; x < offset; x++)
		{
			carry_out = input & 0x1;
			input >>= 1;

			if(carry_out) { input |= 0x80000000; }
		}
	}

	//ROR #0
	//Same as RRX #1, which is similar to ROR #1, except Bit 31 now becomes the old carry flag
	else
	{
		u8 old_carry = (reg.cpsr & CPSR_C_FLAG) ? 1 : 0;
		carry_out = input & 0x1;
		input >>= 1;
		
		if(old_carry) { input |= 0x80000000; }
	}

	return carry_out;
}

/****** Performs 32-bit rotate right - For ARM.5 Data Processing when Bit 25 is 1 ******/
u8 NTR_ARM9::rotate_right_special(u32& input, u8 offset)
{
	u8 carry_out = 2;

	if(offset > 0)
	{
		//Perform ROR shift on immediate
		for(int x = 0; x < (offset * 2); x++)
		{
			carry_out = input & 0x1;
			input >>= 1;

			if(carry_out) { input |= 0x80000000; }
		}
	}

	return carry_out;
}			

/****** Checks address before 32-bit reading/writing for special case scenarios ******/
void NTR_ARM9::mem_check_32(u32 addr, u32& value, bool load_store)
{
	if(load_store) { value = mem->read_u32(addr); }
	else { mem->write_u32(addr, value); }
}

/****** Checks address before 16-bit reading/writing for special case scenarios ******/
void NTR_ARM9::mem_check_16(u32 addr, u32& value, bool load_store)
{
	if(load_store) { value = mem->read_u16(addr); }
	else { mem->write_u16(addr, value); }
}

/****** Checks address before 8-bit reading/writing for special case scenarios ******/
void NTR_ARM9::mem_check_8(u32 addr, u32& value, bool load_store)
{
	if(load_store) { value = mem->read_u8(addr); }
	else { mem->write_u8(addr, value); }
}

/****** Counts cycles for memory accesses  ******/
void NTR_ARM9::clock(u32 access_addr, mem_modes current_mode)
{
	//Determine memory region being accessed
	switch(access_addr >> 24)
	{
		//ITCM - TODO Cache Miss
		case 0x1:
			system_cycles += 2;
			break;

		//Main Memory
		case 0x2:
			switch(current_mode)
			{
				case CODE_N16: system_cycles += 9; break;
				case CODE_S16: system_cycles += 9; break;
				case CODE_N32: system_cycles += 18; break;
				case CODE_S32: system_cycles += 18; break;

				case DATA_N16: system_cycles += 18; break;
				case DATA_S16: system_cycles += 2; break;
				case DATA_N32: system_cycles += 20; break;
				case DATA_S32: system_cycles += 4; break;
			}

			break;

		//WRAM, BIOS, I/O, OAM
		case 0x3:
		case 0x4:
		case 0x7:
			switch(current_mode)
			{
				case CODE_N16: system_cycles += 4; break;
				case CODE_S16: system_cycles += 4; break;
				case CODE_N32: system_cycles += 8; break;
				case CODE_S32: system_cycles += 8; break;

				case DATA_N16: system_cycles += 8; break;
				case DATA_S16: system_cycles += 2; break;
				case DATA_N32: system_cycles += 8; break;
				case DATA_S32: system_cycles += 2; break;
			}

			break;

		//VRAM, Palettes
		case 0x5:
		case 0x6:
			switch(current_mode)
			{
				case CODE_N16: system_cycles += 5; break;
				case CODE_S16: system_cycles += 5; break;
				case CODE_N32: system_cycles += 10; break;
				case CODE_S32: system_cycles += 10; break;

				case DATA_N16: system_cycles += 8; break;
				case DATA_S16: system_cycles += 2; break;
				case DATA_N32: system_cycles += 10; break;
				case DATA_S32: system_cycles += 4; break;
			}

			break;

		default: system_cycles += 2;
	}
}

/****** Counts internal cycles ******/
void NTR_ARM9::clock() { system_cycles++; }

/****** Runs audio and video controllers every clock cycle ******/
void NTR_ARM9::clock_system()
{
	//Convert 66MHz cycles to 33MHz
	system_cycles >>= 1;

	//ARM9 CPU sync cycles
	sync_cycles += system_cycles;

	//Run controllers for each cycle		 
	for(int x = 0; x < system_cycles; x++) { controllers.video.step(); }

	//Run DMA channels
	clock_dma();

	//Run timers
	clock_timers(system_cycles);

	//Reset system cycles
	system_cycles = 2;
}

/****** Runs DMA controllers every clock cycle ******/
void NTR_ARM9::clock_dma()
{
	//Run DMA0 - DMA3 if applicable
	for(u32 x = 0; x < 4; x++)
	{
		if(mem->dma[x].enable) { nds9_dma(x); }
	}
}

/****** Runs Timer controllers every clock cycle ******/
void NTR_ARM9::clock_timers(u8 access_cycles)
{
	u32 due_cycles = 0;
	u32 update_count = 0;
	u32 old_count = 0;

	for(u32 x = 0; x < 4; x++)
	{
		//See if this timer is enabled first
		if(controllers.timer[x].enable)
		{
			//If internal clock goes past 0, increment counter
			if((controllers.timer[x].clock - access_cycles) & 0x80000000)
			{
				old_count = controllers.timer[x].counter;
				due_cycles = ~(controllers.timer[x].clock - access_cycles) + 1;
				update_count = due_cycles / controllers.timer[x].prescalar;
				due_cycles -= (update_count * controllers.timer[x].prescalar);

				if((controllers.timer[x].prescalar == 1)) { controllers.timer[x].clock = 0; }
				else { controllers.timer[x].clock = controllers.timer[x].prescalar - due_cycles; }

				if(!update_count) { update_count = 1; }
				if(!controllers.timer[x].count_up) { controllers.timer[x].counter += update_count; }

				//If counter overflows, reload value, trigger interrupt if necessary
				if((old_count + update_count) > 0xFFFF) 
				{
					controllers.timer[x].counter = controllers.timer[x].reload_value;

					//Increment next timer if in count-up mode
					if((x < 4) && (controllers.timer[x+1].count_up)) { controllers.timer[x+1].counter++; }

					//Interrupt
					if(controllers.timer[x].interrupt) { mem->nds9_if |= (8 << x); }
				}
			}

			else { controllers.timer[x].clock -= access_cycles; }
		}
	}
}

/****** Jumps to or exits an interrupt ******/
void NTR_ARM9::handle_interrupt()
{
	//Exit interrupt
	if((!config::use_bios) && (reg.r15 == 0xFFFF0290))
	{
		//Restore registers from SP
		u32 sp_addr = get_reg(13);
		reg.r0 = mem->read_u32(sp_addr); sp_addr += 4;
		reg.r1 = mem->read_u32(sp_addr); sp_addr += 4;
		reg.r2 = mem->read_u32(sp_addr); sp_addr += 4;
		reg.r3 = mem->read_u32(sp_addr); sp_addr += 4;
		set_reg(12, mem->read_u32(sp_addr)); sp_addr += 4;
		set_reg(14, mem->read_u32(sp_addr)); sp_addr += 4;
		set_reg(13, sp_addr);

		//Set PC to LR - 4;
		reg.r15 = get_reg(14) - 4;

		//Set CPSR from SPSR, turn on IRQ flag
		reg.cpsr = get_spsr();

		//Set the CPU mode accordingly
		switch((reg.cpsr & 0x1F))
		{
			case 0x10: current_cpu_mode = USR; break;
			case 0x11: current_cpu_mode = FIQ; break;
			case 0x12: current_cpu_mode = IRQ; break;
			case 0x13: current_cpu_mode = SVC; break;
			case 0x17: current_cpu_mode = ABT; break;
			case 0x1B: current_cpu_mode = UND; break;
			case 0x1F: current_cpu_mode = SYS; break;
		}

		//Request pipeline flush, signal end of interrupt handling, switch to appropiate ARM/THUMB mode
		flush_pipeline();
		arm_mode = (reg.cpsr & 0x20) ? THUMB : ARM;
	}

	//Jump into an interrupt, check if the master flag is enabled
	if((mem->nds9_ime & 0x1) && ((reg.cpsr & CPSR_IRQ) == 0))
	{
		//Wait until pipeline is finished filling
		//Wait until THUMB.19 is finished executing
		if((debug_message == 0xFF) || (thumb_long_branch)) { return; }

		u32 if_check = mem->nds9_if;
		u32 ie_check = mem->nds9_ie;

		//When there is a match, jump to interrupt vector
		if(ie_check & if_check)
		{
			current_cpu_mode = IRQ;

			if(last_instr_branch) { reg.r15 += 4; }
			else if((last_idle_state == 0) && (arm_mode == ARM)) { reg.r15 -= 4; }

			//Save PC to LR
			set_reg(14, reg.r15);

			//Set PC and SPSR
			reg.r15 = mem->nds9_bios_vector + 0x18;
			set_spsr(reg.cpsr);

			//Request pipeline flush, signal interrupt handling, and go to ARM mode
			flush_pipeline();

			in_interrupt = true;
			arm_mode = ARM;
			last_idle_state = 0;

			//Alter CPSR bits, turn off THUMB and IRQ flags, set mode bits
			reg.cpsr &= ~0x20;
			reg.cpsr |= CPSR_IRQ;
			reg.cpsr &= ~0x1F;
			reg.cpsr |= CPSR_MODE_IRQ;

			//Execute real BIOS code by returning, otherwise use HLE to service the interrupt
			if(config::use_bios) { return; }

			//Save registers to SP
			u32 sp_addr = get_reg(13);
			sp_addr -= 4; mem->write_u32(sp_addr, get_reg(14));
			sp_addr -= 4; mem->write_u32(sp_addr, get_reg(12));
			sp_addr -= 4; mem->write_u32(sp_addr, reg.r3);
			sp_addr -= 4; mem->write_u32(sp_addr, reg.r2);
			sp_addr -= 4; mem->write_u32(sp_addr, reg.r1);
			sp_addr -= 4; mem->write_u32(sp_addr, reg.r0);
			set_reg(13, sp_addr);

			//Set LR to 0xFFFF0290
			set_reg(14, 0xFFFF0290);

			//Set R0 to DTCM + 0x4000
			reg.r0 = mem->dtcm_addr + 0x4000;

			//Set PC to DTCM + 0x3FFC
			reg.r15 = mem->read_u32(mem->dtcm_addr + 0x3FFC);

			//Switch to THUMB mode if necessary
			if(reg.r15 & 0x1) 
			{	 
				arm_mode = THUMB;
				reg.cpsr |= 0x20;
				reg.r15 &= ~0x1;
			}
		}
	}
}

/****** Read CPU data from save state ******/
bool NTR_ARM9::cpu_read(u32 offset, std::string filename)
{
	std::ifstream file(filename.c_str(), std::ios::binary);
	
	if(!file.is_open()) { return false; }

	//Go to offset
	file.seekg(offset);

	//Serialize CPU registers data from file stream
	file.read((char*)&reg, sizeof(reg));

	//Serialize misc CPU data to save state
	file.read((char*)&current_cpu_mode, sizeof(current_cpu_mode));
	file.read((char*)&arm_mode, sizeof(arm_mode));
	file.read((char*)&lbl_addr, sizeof(lbl_addr));
	file.read((char*)&first_branch, sizeof(first_branch));
	file.read((char*)&running, sizeof(running));
	file.read((char*)&needs_flush, sizeof(needs_flush));
	file.read((char*)&in_interrupt, sizeof(in_interrupt));
	file.read((char*)&idle_state, sizeof(idle_state));
	file.read((char*)&last_idle_state, sizeof(last_idle_state));
	file.read((char*)&thumb_long_branch, sizeof(thumb_long_branch));
	file.read((char*)&last_instr_branch, sizeof(last_instr_branch));
	file.read((char*)&swi_waitbyloop_count, sizeof(swi_waitbyloop_count));
	file.read((char*)&instruction_pipeline[0], sizeof(instruction_pipeline[0]));
	file.read((char*)&instruction_pipeline[1], sizeof(instruction_pipeline[1]));
	file.read((char*)&instruction_pipeline[2], sizeof(instruction_pipeline[2]));
	file.read((char*)&instruction_operation[0], sizeof(instruction_operation[0]));
	file.read((char*)&instruction_operation[1], sizeof(instruction_operation[1]));
	file.read((char*)&instruction_operation[2], sizeof(instruction_operation[2]));
	file.read((char*)&pipeline_pointer, sizeof(pipeline_pointer));
	file.read((char*)&debug_message, sizeof(debug_message));
	file.read((char*)&debug_code, sizeof(debug_code));
	file.read((char*)&debug_cycles, sizeof(debug_cycles));
	file.read((char*)&debug_addr, sizeof(debug_addr));
	file.read((char*)&sync_cycles, sizeof(sync_cycles));
	file.read((char*)&system_cycles, sizeof(system_cycles));
	file.read((char*)&re_sync, sizeof(re_sync));

	//Serialize timers to save state
	file.read((char*)&controllers.timer[0], sizeof(controllers.timer[0]));
	file.read((char*)&controllers.timer[1], sizeof(controllers.timer[1]));
	file.read((char*)&controllers.timer[2], sizeof(controllers.timer[2]));
	file.read((char*)&controllers.timer[3], sizeof(controllers.timer[3]));

	//Serialize CP15 registers
	for(u32 x = 0; x < 32; x++) { file.read((char*)&co_proc.regs[x], sizeof(co_proc.regs[x])); }

	file.close();
	return true;
}

/****** Write CPU data to save state ******/
bool NTR_ARM9::cpu_write(std::string filename)
{
	std::ofstream file(filename.c_str(), std::ios::binary | std::ios::trunc);
	
	if(!file.is_open()) { return false; }

	//Serialize CPU registers data to save state
	file.write((char*)&reg, sizeof(reg));

	//Serialize misc CPU data to save state
	file.write((char*)&current_cpu_mode, sizeof(current_cpu_mode));
	file.write((char*)&arm_mode, sizeof(arm_mode));
	file.write((char*)&running, sizeof(running));
	file.write((char*)&lbl_addr, sizeof(lbl_addr));
	file.write((char*)&first_branch, sizeof(first_branch));
	file.write((char*)&needs_flush, sizeof(needs_flush));
	file.write((char*)&in_interrupt, sizeof(in_interrupt));
	file.write((char*)&idle_state, sizeof(idle_state));
	file.write((char*)&last_idle_state, sizeof(last_idle_state));
	file.write((char*)&thumb_long_branch, sizeof(thumb_long_branch));
	file.write((char*)&last_instr_branch, sizeof(last_instr_branch));
	file.write((char*)&swi_waitbyloop_count, sizeof(swi_waitbyloop_count));
	file.write((char*)&instruction_pipeline[0], sizeof(instruction_pipeline[0]));
	file.write((char*)&instruction_pipeline[1], sizeof(instruction_pipeline[1]));
	file.write((char*)&instruction_pipeline[2], sizeof(instruction_pipeline[2]));
	file.write((char*)&instruction_operation[0], sizeof(instruction_operation[0]));
	file.write((char*)&instruction_operation[1], sizeof(instruction_operation[1]));
	file.write((char*)&instruction_operation[2], sizeof(instruction_operation[2]));
	file.write((char*)&pipeline_pointer, sizeof(pipeline_pointer));
	file.write((char*)&debug_message, sizeof(debug_message));
	file.write((char*)&debug_code, sizeof(debug_code));
	file.write((char*)&debug_cycles, sizeof(debug_cycles));
	file.write((char*)&debug_addr, sizeof(debug_addr));
	file.write((char*)&sync_cycles, sizeof(sync_cycles));
	file.write((char*)&system_cycles, sizeof(system_cycles));
	file.write((char*)&re_sync, sizeof(re_sync));

	//Serialize timers to save state
	file.write((char*)&controllers.timer[0], sizeof(controllers.timer[0]));
	file.write((char*)&controllers.timer[1], sizeof(controllers.timer[1]));
	file.write((char*)&controllers.timer[2], sizeof(controllers.timer[2]));
	file.write((char*)&controllers.timer[3], sizeof(controllers.timer[3]));

	//Serialize CP15 registers
	for(u32 x = 0; x < 32; x++) { file.write((char*)&co_proc.regs[x], sizeof(co_proc.regs[x])); }

	file.close();
	return true;
}

/****** Gets the size of CPU data for serialization ******/
u32 NTR_ARM9::size()
{
	u32 cpu_size = 0;

	cpu_size += sizeof(reg);
	cpu_size += sizeof(current_cpu_mode);
	cpu_size += sizeof(arm_mode);
	cpu_size += sizeof(lbl_addr);
	cpu_size += sizeof(first_branch);
	cpu_size += sizeof(running);
	cpu_size += sizeof(needs_flush);
	cpu_size += sizeof(in_interrupt);
	cpu_size += sizeof(idle_state);
	cpu_size += sizeof(last_idle_state);
	cpu_size += sizeof(thumb_long_branch);
	cpu_size += sizeof(last_instr_branch);
	cpu_size += sizeof(swi_waitbyloop_count);
	cpu_size += sizeof(instruction_pipeline[0]);
	cpu_size += sizeof(instruction_pipeline[1]);
	cpu_size += sizeof(instruction_pipeline[2]);
	cpu_size += sizeof(instruction_operation[0]);
	cpu_size += sizeof(instruction_operation[1]);
	cpu_size += sizeof(instruction_operation[2]);
	cpu_size += sizeof(pipeline_pointer);
	cpu_size += sizeof(debug_message);
	cpu_size += sizeof(debug_code);
	cpu_size += sizeof(debug_cycles);
	cpu_size += sizeof(debug_addr);
	cpu_size += sizeof(controllers.timer[0]);
	cpu_size += sizeof(controllers.timer[1]);
	cpu_size += sizeof(controllers.timer[2]);
	cpu_size += sizeof(controllers.timer[3]);
	cpu_size += sizeof(sync_cycles);
	cpu_size += sizeof(system_cycles);
	cpu_size += sizeof(re_sync);

	for(u32 x = 0; x < 32; x++) { cpu_size += sizeof(co_proc.regs[x]); }

	return cpu_size;
}
