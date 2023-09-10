// GB Enhanced+ Copyright Daniel Baxter 2015
// Licensed under the GPLv2
// See LICENSE.txt for full license text

// File : arm9_thumb_instr.cpp
// Date : November 05, 2015
// Description : ARM9 THUMB instructions
//
// Emulates an ARM9 THUMB instructions with equivalent C++

#include "arm9.h"

/****** THUMB.1 - Move Shifted Register ******/
void NTR_ARM9::move_shifted_register(u16 current_thumb_instruction)
{
	//Grab destination register - Bits 0-2
	u8 dest_reg = (current_thumb_instruction & 0x7);

	//Grab source register - Bits 3-5
	u8 src_reg = ((current_thumb_instruction >> 3) & 0x7);

	//Grab shift offset - Bits 6-10
	u8 offset = ((current_thumb_instruction >> 6) & 0x1F);

	//Grab shift opcode - Bits 11-12
	u8 op = ((current_thumb_instruction >> 11) & 0x3);

	u32 result = get_reg(src_reg);
	u8 shift_out = 0;

	//Shift the register
	switch(op)
	{
		//LSL
		case 0x0:
			shift_out = logical_shift_left(result, offset);
			break;

		//LSR
		case 0x1:
			shift_out = logical_shift_right(result, offset);
			break;

		//ASR
		case 0x2:
			shift_out = arithmetic_shift_right(result, offset);
			break;

		default: std::cout<<"CPU::ARM9::Warning: This should not happen in THUMB.1 ... \n"; break;
	}

	set_reg(dest_reg, result);

	//Zero flag
	if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
	else { reg.cpsr &= ~CPSR_Z_FLAG; }

	//Negative flag
	if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
	else { reg.cpsr &= ~CPSR_N_FLAG; }

	//Carry flag
	if(shift_out == 1) { reg.cpsr |= CPSR_C_FLAG; }
	else if(shift_out == 0) { reg.cpsr &= ~CPSR_C_FLAG; }

	//Clock CPU and controllers - 1S
	clock(reg.r15, CODE_S16);
} 

/****** THUMB.2 - Add-Sub Immediate ******/
void NTR_ARM9::add_sub_immediate(u16 current_thumb_instruction)
{
	//Grab destination register - Bits 0-2
	u8 dest_reg = (current_thumb_instruction & 0x7);

	//Grab source register - Bits 3-5
	u8 src_reg = ((current_thumb_instruction >> 3) & 0x7);

	//Grab the opcode - Bits 9-10
	u8 op = ((current_thumb_instruction >> 9) & 0x3);

	u32 input = get_reg(src_reg);
	u32 result = 0;
	u32 operand = 0;
	u8 imm_reg = ((current_thumb_instruction >> 6) & 0x7);

	//Perform addition or subtraction
	switch(op)
	{
		//Add with register as operand
		case 0x0:
			operand = get_reg(imm_reg);
			result = input + operand;
			break;

		//Subtract with register as operand
		case 0x1:
			operand = get_reg(imm_reg);
			result = input - operand;
			break;

		//Add with 3-bit immediate as operand
		case 0x2:
			operand = imm_reg;
			result = input + operand;
			break;

		//Subtract with 3-bit immediate as operand
		case 0x3:
			operand = imm_reg;
			result = input - operand;
			break;
	}

	set_reg(dest_reg, result);

	//Update condition codes
	if(op & 0x1){ update_condition_arithmetic(input, operand, result, false); }
	else { update_condition_arithmetic(input, operand, result, true); }

	//Clock CPU and controllers - 1S
	clock(reg.r15, CODE_S16);
}

/****** THUMB.3 Move-Compare-Add-Subtract Immediate ******/
void NTR_ARM9::mcas_immediate(u16 current_thumb_instruction)
{
	//Grab destination register - Bits 8-10
	u8 dest_reg = ((current_thumb_instruction >> 8) & 0x7);

	//Grab opcode - Bits 11-12
	u8 op = ((current_thumb_instruction >> 11) & 0x3);

	u32 input = get_reg(dest_reg); //Looks weird but the source is also the destination in this instruction
	u32 result = 0;
	
	//Operand is 8-bit immediate
	u32 operand = (current_thumb_instruction & 0xFF);
	
	//Perform Move-Compare-Add-Subtract
	switch(op)
	{
		//MOV
		case 0x0:
			result = operand;

			//Zero flag
			if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
			else { reg.cpsr &= ~CPSR_Z_FLAG; }

			//Negative flag
			if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
			else { reg.cpsr &= ~CPSR_N_FLAG; }
			
			break;

		//CMP, SUB
		case 0x1:
		case 0x3:
			result = (input - operand);
			update_condition_arithmetic(input, operand, result, false);
			break;

		//ADD
		case 0x2:
			result = (input + operand);
			update_condition_arithmetic(input, operand, result, true);
			break;
	}

	//Do not update the destination register if CMP is the operation!
	if(op != 1) { set_reg(dest_reg, result); }

	//Clock CPU and controllers - 1S
	clock(reg.r15, CODE_S16);
}
			
/****** THUMB.4 ALU Operations ******/
void NTR_ARM9::alu_ops(u16 current_thumb_instruction)
{
	//Grab destination register - Bits 0-2
	u8 dest_reg = (current_thumb_instruction & 0x7);

	//Grab source register - Bits 3-5
	u8 src_reg = ((current_thumb_instruction >> 3) & 0x7);

	//Grab opcode - Bits 6-9
	u8 op = ((current_thumb_instruction >> 6) & 0xF);

	u32 input = get_reg(dest_reg); //Still looks weird, but same as in THUMB.3
	u32 result = 0;
	u32 operand = get_reg(src_reg);
	u8 shift_out = 0;
	u8 carry_out = (reg.cpsr & CPSR_C_FLAG) ? 1 : 0;

	//Perform ALU operations
	switch(op)
	{
		//AND
		case 0x0:
			result = (input & operand);
			
			//Zero flag
			if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
			else { reg.cpsr &= ~CPSR_Z_FLAG; }

			//Negative flag
			if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
			else { reg.cpsr &= ~CPSR_N_FLAG; }

			set_reg(dest_reg, result);

			//Clock CPU and controllers - 1S
			clock(reg.r15, CODE_S16);

			break;

		//XOR
		case 0x1:
			result = (input ^ operand);
		
			//Zero flag
			if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
			else { reg.cpsr &= ~CPSR_Z_FLAG; }

			//Negative flag
			if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
			else { reg.cpsr &= ~CPSR_N_FLAG; }

			set_reg(dest_reg, result);

			//Clock CPU and controllers - 1S
			clock(reg.r15, CODE_S16);

			break;

		//LSL
		case 0x2:
			operand &= 0xFF;
			if(operand != 0) { shift_out = logical_shift_left(input, operand); }
			result = input;

			//Zero flag
			if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
			else { reg.cpsr &= ~CPSR_Z_FLAG; }

			//Negative flag
			if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
			else { reg.cpsr &= ~CPSR_N_FLAG; }		

			//Carry flag
			if(operand != 0)
			{
				if(shift_out == 1) { reg.cpsr |= CPSR_C_FLAG; }
				else if(shift_out == 0) { reg.cpsr &= ~CPSR_C_FLAG; }
			}

			//Clock CPU and controllers - 1I
			clock();

			set_reg(dest_reg, result);

			//Clock CPU and controllers - 1S
			clock((reg.r15 + 2), CODE_S16);

			break;

		//LSR
		case 0x3:
			operand &= 0xFF;
			if(operand != 0) { shift_out = logical_shift_right(input, operand); }
			result = input;

			//Zero flag
			if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
			else { reg.cpsr &= ~CPSR_Z_FLAG; }

			//Negative flag
			if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
			else { reg.cpsr &= ~CPSR_N_FLAG; }		

			//Carry flag
			if(operand != 0)
			{
				if(shift_out == 1) { reg.cpsr |= CPSR_C_FLAG; }
				else if(shift_out == 0) { reg.cpsr &= ~CPSR_C_FLAG; }
			}

			//Clock CPU and controllers - 1I
			clock();

			set_reg(dest_reg, result);

			//Clock CPU and controllers - 1S
			clock((reg.r15 + 2), CODE_S16);

			break;

		//ASR
		case 0x4:
			operand &= 0xFF;
			if(operand != 0) { shift_out = arithmetic_shift_right(input, operand); }
			result = input;

			//Zero flag
			if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
			else { reg.cpsr &= ~CPSR_Z_FLAG; }

			//Negative flag
			if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
			else { reg.cpsr &= ~CPSR_N_FLAG; }		

			//Carry flag
			if(operand != 0)
			{
				if(shift_out == 1) { reg.cpsr |= CPSR_C_FLAG; }
				else if(shift_out == 0) { reg.cpsr &= ~CPSR_C_FLAG; }
			}

			//Clock CPU and controllers - 1I
			clock();

			set_reg(dest_reg, result);

			//Clock CPU and controllers - 1S
			clock((reg.r15 + 2), CODE_S16);

			break;

		//ADC
		case 0x5:
			result = (input + operand + carry_out);
			update_condition_arithmetic(input, operand, result, true);

			set_reg(dest_reg, result);

			//Clock CPU and controllers - 1S
			clock(reg.r15, CODE_S16);

			break;

		//SBC
		case 0x6:
			//Invert (NOT) carry
			if(carry_out) { carry_out = 0; }
			else { carry_out = 1; }

			result = (input - operand - carry_out);
			update_condition_arithmetic(input, operand, result, false);

			set_reg(dest_reg, result);

			//Clock CPU and controllers - 1S
			clock(reg.r15, CODE_S16);

			break;

		//ROR
		case 0x7:
			operand &= 0xFF;
			if(operand != 0) { shift_out = rotate_right(input, operand); }
			result = input;

			//Zero flag
			if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
			else { reg.cpsr &= ~CPSR_Z_FLAG; }

			//Negative flag
			if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
			else { reg.cpsr &= ~CPSR_N_FLAG; }		

			//Carry flag
			if(operand != 0)
			{
				if(shift_out == 1) { reg.cpsr |= CPSR_C_FLAG; }
				else if(shift_out == 0) { reg.cpsr &= ~CPSR_C_FLAG; }
			}

			//Clock CPU and controllers - 1I
			clock();

			set_reg(dest_reg, result);

			//Clock CPU and controllers - 1S
			clock(reg.r15, CODE_S16);

			break;

		//TST
		case 0x8:
			result = (input & operand);

			//Zero flag
			if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
			else { reg.cpsr &= ~CPSR_Z_FLAG; }

			//Negative flag
			if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
			else { reg.cpsr &= ~CPSR_N_FLAG; }

			//Clock CPU and controllers - 1S
			clock(reg.r15, CODE_S16);

			break;

		//NEG
		case 0x9:
			input = 0;
			result = (input - operand);
			update_condition_arithmetic(input, operand, result, false);

			set_reg(dest_reg, result);

			//Clock CPU and controllers - 1S
			clock(reg.r15, CODE_S16);

			break;

		//CMP
		case 0xA:
			result = (input - operand);
			update_condition_arithmetic(input, operand, result, false);

			//Clock CPU and controllers - 1S
			clock(reg.r15, CODE_S16);

			break;

		//CMN
		case 0xB:
			result = (input + operand);
			update_condition_arithmetic(input, operand, result, true);

			//Clock CPU and controllers - 1S
			clock(reg.r15, CODE_S16);

			break;

		//ORR
		case 0xC:
			result = (input | operand);
			
			//Zero flag
			if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
			else { reg.cpsr &= ~CPSR_Z_FLAG; }

			//Negative flag
			if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
			else { reg.cpsr &= ~CPSR_N_FLAG; }

			set_reg(dest_reg, result);

			//Clock CPU and controllers - 1S
			clock(reg.r15, CODE_S16);

			break;

		//MUL
		case 0xD:
			result = (input * operand);

			//Zero flag
			if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
			else { reg.cpsr &= ~CPSR_Z_FLAG; }

			//Negative flag
			if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
			else { reg.cpsr &= ~CPSR_N_FLAG; }

			//TODO - Figure out what the carry flag should be for this opcode.
			//TODO - Figure out the timing for this opcode

			set_reg(dest_reg, result);
			break;

		//BIC
		case 0xE:
			result = (input & ~operand);

			//Zero flag
			if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
			else { reg.cpsr &= ~CPSR_Z_FLAG; }

			//Negative flag
			if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
			else { reg.cpsr &= ~CPSR_N_FLAG; }

			set_reg(dest_reg, result);

			//Clock CPU and controllers - 1S
			clock(reg.r15, CODE_S16);

			break;

		//MVN
		case 0xF:
			result = ~operand;

			//Zero flag
			if(result == 0) { reg.cpsr |= CPSR_Z_FLAG; }
			else { reg.cpsr &= ~CPSR_Z_FLAG; }

			//Negative flag
			if(result & 0x80000000) { reg.cpsr |= CPSR_N_FLAG; }
			else { reg.cpsr &= ~CPSR_N_FLAG; }

			set_reg(dest_reg, result);

			//Clock CPU and controllers - 1S
			clock(reg.r15, CODE_S16);

			break;
	}
}

/****** THUMB.5 High Register Operations + Branch Exchange ******/
void NTR_ARM9::hireg_bx(u16 current_thumb_instruction)
{
	//Grab destination register - Bits 0-2
	u8 dest_reg = (current_thumb_instruction & 0x7);

	//Grab source register - Bits 3-5
	u8 src_reg = ((current_thumb_instruction >> 3) & 0x7);

	//Grab source register MSB - Bit 6
	u8 sr_msb = (current_thumb_instruction & 0x40) ? 1 : 0;

	//Grab destination register MSB - Bit 7
	u8 dr_msb = (current_thumb_instruction & 0x80) ? 1 : 0;

	//Add MSB to source and destination registers
	if(sr_msb) { src_reg |= 0x8; }
	if(dr_msb) { dest_reg |= 0x8; }

	//Grab the opcode
	u8 op = ((current_thumb_instruction >> 8) & 0x3);

	u32 input = get_reg(dest_reg); //Still looks weird, but same as in THUMB.3
	u32 result = 0;
	u32 operand = get_reg(src_reg);

	if((op == 3) && (dr_msb != 0)) 
	{
		op = 4;
		
		if(src_reg == 15)
		{
			std::cout<<"CPU::ARM9::Error - THUMB.5 BLX using R15 as operand \n";
			running = false;
			return;
		}
	}

	//Perform ops or branch - Only CMP affects flags!
	switch(op)
	{
		//ADD
		case 0x0:
			//When the destination register is the PC, auto-align operand to half-word
			if(dest_reg == 15) { operand &= ~0x1; }

			//Destination is not PC
			if(dest_reg != 15)
			{
				result = (input + operand);
				set_reg(dest_reg, result);

				//Clock CPU and controllers - 1S
				clock(reg.r15, CODE_S16);
			}

			//Destination is PC
			else
			{
				//Clock CPU and controllers - 1N
				clock(reg.r15, CODE_N16);

				result = (input + operand);
				set_reg(dest_reg, result);
				needs_flush = true;

				//Clock CPU and controllers - 2S
				clock(reg.r15, CODE_S16);
				clock((reg.r15 + 2), CODE_S16);
			}

			break;

		//CMP
		case 0x1:
			result = (input - operand);
			update_condition_arithmetic(input, operand, result, false);

			//Clock CPU and controllers - 1S
			clock(reg.r15, CODE_S16);

			break;

		//MOV
		case 0x2:
			//When the destination register is the PC, auto-align operand to half-word
			if(dest_reg == 15) { operand &= ~0x1; }

			//Operand is not PC
			if(dest_reg != 15)
			{
				result = operand;
				set_reg(dest_reg, result);

				//Clock CPU and controllers - 1S
				clock(reg.r15, CODE_S16);
			}

			//Operand is PC
			else
			{
				//Clock CPU and controllers - 1N
				clock(reg.r15, CODE_N16);

				result = operand;
				set_reg(dest_reg, result);
				needs_flush = true;

				//Clock CPU and controllers - 2S
				clock(reg.r15, CODE_S16);
				clock((reg.r15 + 2), CODE_S16);
			}

			break;

		//BX
		case 0x3:
			//Switch to ARM mode if necessary
			if((operand & 0x1) == 0)
			{
				arm_mode = ARM;
				reg.cpsr &= ~0x20;
				operand &= ~0x3;
			}

			//Align operand to half-word
			else { operand &= ~0x1; }

			//Clock CPU and controllers - 1N
			clock(reg.r15, CODE_N16);

			//Auto-align PC when using R15 as an operand
			if(src_reg == 15)
			{
				reg.r15 &= ~0x2;
			}

			else { reg.r15 = operand; }

			//Clock CPU and controllers - 2S
			clock(reg.r15, CODE_S16);
			clock((reg.r15 + 2), CODE_S16);

			needs_flush = true;
			break;

		//BLX
		case 0x4:
			//Switch to ARM mode if necessary
			if((operand & 0x1) == 0)
			{
				arm_mode = ARM;
				reg.cpsr &= ~0x20;
			}

			//Align operand to half-word
			else { operand &= ~0x1; }

			//Clock CPU and controllers - 1N
			clock(reg.r15, CODE_N16);

			//LR is PC+3, but GBE+'s PC is always 4 ahead in THUMB mode anyway, so set to PC - 1.
			set_reg(14, (reg.r15 - 1));
			reg.r15 = operand;

			//Clock CPU and controllers - 2S
			clock(reg.r15, CODE_S16);
			clock((reg.r15 + 2), CODE_S16);

			needs_flush = true;
			break;
	}
}

/****** THUMB.6 Load PC Relative ******/
void NTR_ARM9::load_pc_relative(u16 current_thumb_instruction)
{
	//Grab 8-bit offset - Bits 0-7
	u16 offset = (current_thumb_instruction & 0xFF);
	
	//Grab destination register - Bits 8-10
	u8 dest_reg = ((current_thumb_instruction >> 8) & 0x7);

	offset *= 4;
	u32 value = 0;
	u32 load_addr = (reg.r15 & ~0x2) + offset;

	//Clock CPU and controllers - 1N
	clock(load_addr, DATA_N32);

	//Clock CPU and controllers - 1I
	mem_check_32(load_addr, value, true);
	clock();

	//Clock CPU and controllers - 1S
	set_reg(dest_reg, value);
	clock((reg.r15 + 2), CODE_S16);
}

/****** THUMB.7 Load-Store with Register Offset ******/
void NTR_ARM9::load_store_reg_offset(u16 current_thumb_instruction)
{
	//Grab source-destination register - Bits 0-2
	u8 src_dest_reg = (current_thumb_instruction & 0x7);

	//Grab base register - Bits 3-5
	u8 base_reg = ((current_thumb_instruction >> 3) & 0x7);

	//Grab offset register - Bits 6-8
	u8 offset_reg = ((current_thumb_instruction >> 6) & 0x7);

	//Grab opcode - Bits 10-11
	u8 op = ((current_thumb_instruction >> 10) & 0x3);

	u32 value = 0;
	u32 op_addr = get_reg(base_reg) + get_reg(offset_reg);

	//Perform Load-Store ops
	switch(op)
	{
		//STR
		case 0x0:
			//Clock CPU and controllers - 1N
			clock(reg.r15, CODE_N16);

			//Clock CPU and controllers - 1N
			value = get_reg(src_dest_reg);
			mem->write_u32(op_addr, value);
			clock(op_addr, DATA_N32);

			break;

		//STRB
		case 0x1:
			//Clock CPU and controllers - 1N
			clock(reg.r15, CODE_N16);

			//Clock CPU and controllers - 1N
			value = get_reg(src_dest_reg);
			value &= 0xFF;
			mem_check_8(op_addr, value, false);
			clock(op_addr, DATA_N16);

			break;

		//LDR
		case 0x2:
			//Clock CPU and controllers - 1N
			clock(op_addr, DATA_N32);

			//Clock CPU and controllers - 1I
			mem_check_32(op_addr, value, true);
			clock();

			//Clock CPU and controllers - 1S
			set_reg(src_dest_reg, value);
			clock((reg.r15 + 2), CODE_S16);

			break;

		//LDRB
		case 0x3:
			//Clock CPU and controllers - 1N
			clock(op_addr, DATA_N16);

			//Clock CPU and controllers - 1I
			mem_check_8(op_addr, value, true);
			clock();

			//Clock CPU and controllers - 1S
			set_reg(src_dest_reg, value);
			clock((reg.r15 + 2), CODE_S16);

			break;
	}
}

/****** THUMB.8 Load-Store Sign-Extended ******/
void NTR_ARM9::load_store_sign_ex(u16 current_thumb_instruction)
{
	//Grab source-destination register - Bits 0-2
	u8 src_dest_reg = (current_thumb_instruction & 0x7);

	//Grab base register - Bits 3-5
	u8 base_reg = ((current_thumb_instruction >> 3) & 0x7);

	//Grab offset register - Bits 6-8
	u8 offset_reg = ((current_thumb_instruction >> 6) & 0x7);

	//Grab opcode - Bits 10-11
	u8 op = ((current_thumb_instruction >> 10) & 0x3);

	u32 value = 0;
	u32 op_addr = get_reg(base_reg) + get_reg(offset_reg);

	//Perform Load-Store ops
	switch(op)
	{
		//STRH
		case 0x0:
			//Clock CPU and controllers - 1N
			clock(reg.r15, CODE_N16);

			//Clock CPU and controllers - 1N
			value = get_reg(src_dest_reg);
			value &= 0xFFFF;
			mem_check_16(op_addr, value, false);
			clock(op_addr,  DATA_N16);

			break;

		//LDSB
		case 0x1:
			//Clock CPU and controllers - 1N
			clock(op_addr, DATA_N16);

			//Clock CPU and controllers - 1I
			value = mem->read_u8(op_addr);
			clock();

			//Sign extend from Bit 7
			if(value & 0x80) { value |= 0xFFFFFF00; }

			//Clock CPU and controllers - 1S
			set_reg(src_dest_reg, value);
			clock((reg.r15 + 2), CODE_S16);

			break;

		//LDRH
		case 0x2:
			//Since value is u32 and 0, it is already zero-extended :)
			
			//Clock CPU and controllers - 1N
			clock(op_addr, DATA_N16);

			//Clock CPU and controllers - 1I
			mem_check_16(op_addr, value, true);
			clock();

			//Clock CPU and controllers - 1S
			set_reg(src_dest_reg, value);
			clock((reg.r15 + 2), CODE_S16);

			break;

		//LDSH
		case 0x3:
			//Clock CPU and controllers - 1N
			clock(op_addr, DATA_N16);

			//Clock CPU and controllers - 1I
			mem_check_16(op_addr, value, true);
			clock();

			//Sign extend from Bit 15
			if(value & 0x8000) { value |= 0xFFFF0000; }

			//Clock CPU and controllers - 1S
			set_reg(src_dest_reg, value);
			clock((reg.r15 + 2), CODE_S16);

			break;
	}		
}

/****** THUMB.9 Load-Store with Immediate Offset ******/
void NTR_ARM9::load_store_imm_offset(u16 current_thumb_instruction)
{
	//Grab source-destination register - Bits 0-2
	u8 src_dest_reg = (current_thumb_instruction & 0x7);

	//Grab base register - Bits 3-5
	u8 base_reg = ((current_thumb_instruction >> 3) & 0x7);

	//Grab offset - Bits 6-10
	u16 offset = ((current_thumb_instruction >> 6) & 0x1F);

	//Grab opcode - Bits 11-12
	u8 op = ((current_thumb_instruction >> 11) & 0x3);

	u32 value = 0;
	u32 op_addr = get_reg(base_reg);

	//Perform Load-Store ops
	switch(op)
	{
		//STR
		case 0x0:
			//Clock CPU and controllers - 1N
			value = get_reg(src_dest_reg);
			offset <<= 2;
			op_addr += offset;
			clock(reg.r15, CODE_N16);
			
			//Clock CPU and controllers - 1N
			mem_check_32(op_addr, value, false);
			clock(op_addr, DATA_N32);
			
			break;

		//LDR
		case 0x1:
			//Clock CPU and controllers - 1N
			offset <<= 2;
			op_addr += offset;
			clock(op_addr, DATA_N32);

			//Clock CPU and controllers - 1I
			mem_check_32(op_addr, value, true);
			clock();

			//Clock CPU and controllers - 1S
			set_reg(src_dest_reg, value);
			clock((reg.r15 + 2), CODE_S16);

			break;

		//STRB
		case 0x2:
			//Clock CPU and controllers - 1N
			value = get_reg(src_dest_reg);
			op_addr += offset;
			clock(reg.r15,  CODE_N16);

			//Clock CPU and controllers - 1N
			mem_check_8(op_addr, value, false);
			clock(op_addr, DATA_N16);

			break;

		//LDRB
		case 0x3:
			//Clock CPU and controllers - 1N
			op_addr += offset;
			clock(op_addr, DATA_N16);

			//Clock CPU and controllers - 1I
			mem_check_8(op_addr, value, true);
			clock();

			//Clock CPU and controllers - 1S
			set_reg(src_dest_reg, value);
			clock((reg.r15 + 2), CODE_S16);

			break;
	}
}
			
/****** THUMB.10 Load-Store Halfword ******/
void NTR_ARM9::load_store_halfword(u16 current_thumb_instruction)
{
	//Grab source-destination register - Bits 0-2
	u8 src_dest_reg = (current_thumb_instruction & 0x7);

	//Grab base register - Bits 3-5
	u8 base_reg = ((current_thumb_instruction >> 3) & 0x7);

	//Grab offset - Bits 6-10
	u16 offset = ((current_thumb_instruction >> 6) & 0x1F);

	//Grab opcode - Bit 11
	u8 op = (current_thumb_instruction & 0x800) ? 1 : 0;

	u32 value = 0;
	u32 op_addr = get_reg(base_reg);

	offset <<= 1;
	op_addr += offset;

	//Perform Load-Store ops
	switch(op)
	{
		//STRH
		case 0x0:
			//Clock CPU and controllers - 1N
			clock(reg.r15, CODE_N16);

			//Clock CPU and controllers - 1N
			value = get_reg(src_dest_reg);
			mem_check_16(op_addr, value, false);
			clock(op_addr, DATA_N16);

			break;

		//LDRH
		case 0x1:
			//Clock CPU and controllers - 1N
			clock(op_addr, DATA_N16);

			//Clock CPU and controllers - 1I
			mem_check_16(op_addr, value, true);
			clock();

			//Clock CPU and controllers - 1S
			set_reg(src_dest_reg, value);
			clock((reg.r15 + 2), CODE_S16);

			break;
	}
}

/****** THUMB.11 Load-Store SP-Relative ******/
void NTR_ARM9::load_store_sp_relative(u16 current_thumb_instruction)
{
	//Grab 8-bit offset - Bits 0-7
	u16 offset = (current_thumb_instruction & 0xFF);

	//Grab source-destination register - Bits 8-10
	u8 src_dest_reg = ((current_thumb_instruction >> 8) & 0x7);

	//Grab opcode - Bit 11
	u8 op = (current_thumb_instruction & 0x800) ? 1 : 0;

	u32 value = 0;
	u32 op_addr = get_reg(13);

	offset <<= 2;
	op_addr += offset;

	//Perform Load-Store ops
	switch(op)
	{
		//STR
		case 0x0:
			//Clock CPU and controllers - 1N
			clock(reg.r15, CODE_N16);

			//Clock CPU and controllers - 1N
			value = get_reg(src_dest_reg);
			mem_check_32(op_addr, value, false);
			clock(op_addr, DATA_N32);

			break;

		//LDR
		case 0x1:
			//Clock CPU and controllers - 1N
			clock(op_addr, DATA_N32);

			//Clock CPU and controllers - 1I
			mem_check_32(op_addr, value, true);
			clock();

			//Clock CPU and controllers - 1S
			set_reg(src_dest_reg, value);
			clock((reg.r15 + 2), CODE_S16);

			break;
	}
}

/****** THUMB.12 Get Relative Address ******/
void NTR_ARM9::get_relative_address(u16 current_thumb_instruction)
{
	//Grab 8-bit offset - Bits 0-7
	u16 offset = (current_thumb_instruction & 0xFF);

	//Grab destination register - Bits 8-10
	u8 dest_reg = ((current_thumb_instruction >> 8) & 0x7);

	//Grab opcode - Bit 11
	u8 op = (current_thumb_instruction & 0x800) ? 1 : 0;

	u32 value = 0;
	offset <<= 2;

	//Perform get relative address ops
	switch(op)
	{
		//Rd = PC + nn
		case 0x0:
			value = (reg.r15 & ~0x2) + offset;
			set_reg(dest_reg, value);
			break;

		//Rd = SP + nn
		case 0x1:
			value = get_reg(13) + offset;
			set_reg(dest_reg, value);
			break;
	}

	//Clock CPU and controllers - 1S
	clock(reg.r15, CODE_S16);
}

/****** THUMB.13 Add Offset to Stack Pointer ******/
void NTR_ARM9::add_offset_sp(u16 current_thumb_instruction)
{
	//Grab 7-bit offset - Bits 0-6
	u16 offset = (current_thumb_instruction & 0x7F);

	//Grab opcode - Bit 7
	u8 op = (current_thumb_instruction & 0x80) ? 1 : 0;

	offset <<= 2;

	//Grab stack pointer from current CPU mode
	u32 r13 = get_reg(13);

	//Perform add offset ops
	switch(op)
	{
		//SP = SP + nn
		case 0x0:
			r13 += offset;
			break;

		//SP = SP - nn
		case 0x1:
			r13 -= offset;
			break;
	}

	//Update stack pointer for current CPU mode
	set_reg(13, r13);

	//Clock CPU and controllers - 1S
	clock(reg.r15, CODE_S16);
}
		
/****** THUMB.14 Push-Pop Registers ******/
void NTR_ARM9::push_pop(u16 current_thumb_instruction)
{
	//Grab stack pointer from current CPU mode
	u32 r13 = get_reg(13);

	//Grab link register from current CPU mode
	u32 lr = get_reg(14);

	//Grab register list - Bits 0-7
	u8 r_list = (current_thumb_instruction & 0xFF);

	//Grab PC-LR bit - Bit 8
	bool pc_lr_bit = (current_thumb_instruction & 0x100) ? true : false;

	//Grab opcode - Bit 11
	u8 op = (current_thumb_instruction & 0x800) ? 1 : 0;
	
	u8 n_count = 0;

	//Grab n_count
	for(int x = 0; x < 8; x++)
	{
		if((r_list >> x) & 0x1) { n_count++; }
	}

	//Perform push-pop ops
	switch(op)
	{
		//PUSH
		case 0x0:
			//Clock CPU and controllers - 1N
			clock(reg.r15, CODE_N16);

			//Optionally store LR onto the stack
			if(pc_lr_bit) 
			{
				r13 -= 4;
				mem_check_32(r13, lr, false);
				set_reg(14, lr);  

				//Clock CPU and controllers - 1S
				clock(r13, DATA_S32);
			}

			//Cycle through the register list
			for(int x = 7; x >= 0; x--)
			{
				if(r_list & (1 << x))
				{
					r13 -= 4;
					u32 push_value = get_reg(x);
					mem_check_32(r13, push_value, false);

					//Clock CPU and controllers - (n)S
					if((n_count - 1) != 0) { clock(r13, DATA_S32); n_count--; }

					//Clock CPU and controllers - 1N
					else { clock(r13, DATA_N32); x = 10; break; }
				}
			}

			break;

		//POP
		case 0x1:
			//Clock CPU and controllers - 1N
			clock(reg.r15, CODE_N16);
			
			//Cycle through the register list
			for(int x = 0; x < 8; x++)
			{
				if(r_list & 0x1)
				{
					u32 pop_value = 0;
					mem_check_32(r13, pop_value, true);
					set_reg(x, pop_value);
					r13 += 4;

					//Clock CPU and controllers - (n)S
					if(n_count > 1) { clock(r13, DATA_S32); }
				}

				r_list >>= 1;
			}

			//Optionally load PC from the stack
			if(pc_lr_bit) 
			{
				//Clock CPU and controllers - 1I
				clock();

				//Clock CPU and controllers - 1N
				clock(r13, DATA_N32);

				//Clock CPU and controllers - 2S
				mem_check_32(r13, reg.r15, true);

				//ARMv5 - Switch to ARM when Bit 0 of the new PC is unset
				if((reg.r15 & 0x1) == 0)
				{
					arm_mode = ARM;
					reg.cpsr &= ~0x20;
				}

				reg.r15 &= ~0x1;
				r13 += 4;
				needs_flush = true;

				clock(reg.r15,  CODE_S16);
				clock((reg.r15 + 2), CODE_S16);

				
			}

			//If PC not loaded, last cycles are Internal then Sequential
			else
			{
				//Clock CPU and controllers - 1I
				clock();

				//Clock CPU and controllers - 1S
				clock((reg.r15 + 2), CODE_S16);
			}

			break;
	}

	//Update stack pointer for current CPU mode
	set_reg(13, r13);
}

/****** THUMB.15 Multiple Load-Store ******/
void NTR_ARM9::multiple_load_store(u16 current_thumb_instruction)
{
	//Grab register list - Bits 0-7
	u8 r_list = (current_thumb_instruction & 0xFF);

	//Grab base register - Bits 8-10
	u8 base_reg = ((current_thumb_instruction >> 8) & 0x7);

	//Grab opcode - Bit 11
	u8 op = (current_thumb_instruction & 0x800) ? 1 : 0;

	u32 base_addr = get_reg(base_reg);
	u32 reg_value = 0;
	u8 n_count = 0;

	u32 old_base = base_addr & ~0x03;
	u8 transfer_reg = 0xFF;
	bool write_back = true;

	//Find out the first register in the Register List
	for(int x = 0; x < 8; x++)
	{
		if(r_list & (1 << x))
		{
			transfer_reg = x;
			x = 0xFF;
			break;
		}
	}

	//Grab n_count
	for(int x = 0; x < 8; x++)
	{
		if((r_list >> x) & 0x1) { n_count++; }
	}

	//Perform multi load-store ops
	switch(op)
	{
		//STMIA
		case 0x0:
			//If register list is not empty, store normally
			if(r_list != 0)
			{
				//Clock CPU and controllers - 1N
				clock(reg.r15, CODE_N16);

				//Cycle through the register list
				for(int x = 0; x < 8; x++)
				{
					if(r_list & 0x1)
					{
						reg_value = get_reg(x);

						if((x == transfer_reg) && (base_reg == transfer_reg)) { mem_check_32(base_addr, old_base, false); }
						else { mem_check_32(base_addr, reg_value, false); }

						//Update base register
						base_addr += 4;
						set_reg(base_reg, base_addr);

						//Clock CPU and controllers - (n)S
						if((n_count - 1) != 0) { clock(base_addr, DATA_S32); n_count--; }

						//Clock CPU and controllers - 1N
						else { clock(base_addr, DATA_N32); x = 10; break; }
					}

					r_list >>= 1;
				}
			}

			//Special case with empty list
			else
			{
				//Store PC, then add 0x40 to base register
				mem_check_32(base_addr, reg.r15, false);
				base_addr += 0x40;
				set_reg(base_reg, base_addr);

				//Clock CPU and controllers - ???
				//TODO - find out what to do here...
			}

			break;

		//LDMIA
		case 0x1:
			//If register list is not empty, load normally
			if(r_list != 0)
			{
				//Clock CPU and controllers - 1N
				clock(reg.r15, CODE_N16);

				//Cycle through the register list
				for(int x = 0; x < 8; x++)
				{
					if(r_list & 0x1)
					{
						if((x == transfer_reg) && (base_reg == transfer_reg)) { write_back = false; }

						mem_check_32(base_addr, reg_value, true);
						set_reg(x, reg_value);

						//Update base register
						base_addr += 4;
						if(write_back) { set_reg(base_reg, base_addr); }

						//Clock CPU and controllers - (n)S
						if(n_count > 1) { clock(base_addr, DATA_S32); }
					}

					r_list >>= 1;
				}

				//Clock CPU and controllers - 1I
				clock();

				//Clock CPU and controllers - 1S
				clock((reg.r15 + 2), CODE_S16);
			}

			//Special case with empty list
			else
			{
				//Load PC, then add 0x40 to base register
				mem_check_32(base_addr, reg.r15, true);
				base_addr += 0x40;
				set_reg(base_reg, base_addr);

				//Clock CPU and controllers - ???
				//TODO - find out what to do here...
			}

			break;
	}
}
						
/****** THUMB.16 Conditional Branch ******/
void NTR_ARM9::conditional_branch(u16 current_thumb_instruction)
{
	//Grab 8-bit offset - Bits 0-7
	u32 offset = (current_thumb_instruction & 0xFF);
	offset <<= 1;

	//Grab opcode - Bits 8-11
	u8 op = ((current_thumb_instruction >> 8) & 0xF);

	//Convert Two's Complement
	if(offset & 0x100) { offset |= 0xFFFFFE00; }

	//Jump based on condition codes
	switch(op)
	{
		//BEQ
		case 0x0:
			if(reg.cpsr & CPSR_Z_FLAG) { needs_flush = true; }
			break;

		//BNE
		case 0x1:
			if((reg.cpsr & CPSR_Z_FLAG) == 0) { needs_flush = true; }
			break;

		//BCS
		case 0x2:
			if(reg.cpsr & CPSR_C_FLAG) { needs_flush = true; }
			break;

		//BCC
		case 0x3:
			if((reg.cpsr & CPSR_C_FLAG) == 0) { needs_flush = true; }
			break;

		//BMI
		case 0x4:
			if(reg.cpsr & CPSR_N_FLAG) { needs_flush = true; }
			break;

		//BPL
		case 0x5:
			if((reg.cpsr & CPSR_N_FLAG) == 0) { needs_flush = true; }
			break;

		//BVS
		case 0x6:
			if(reg.cpsr & CPSR_V_FLAG) { needs_flush = true; }
			break;

		//BVC
		case 0x7:
			if((reg.cpsr & CPSR_V_FLAG) == 0) { needs_flush = true; }
			break;

		//BHI
		case 0x8:
			if((reg.cpsr & CPSR_C_FLAG) && ((reg.cpsr & CPSR_Z_FLAG) == 0)) { needs_flush = true; }
			break;

		//BLS
		case 0x9:
			if((reg.cpsr & CPSR_Z_FLAG) || ((reg.cpsr & CPSR_C_FLAG) == 0)) { needs_flush = true; }
			break;

		//BGE
		case 0xA:
			{
				u8 n = (reg.cpsr & CPSR_N_FLAG) ? 1 : 0;
				u8 v = (reg.cpsr & CPSR_V_FLAG) ? 1 : 0;

				if(n == v) { needs_flush = true; }
			}
			
			break;

		//BLT
		case 0xB:
			{
				u8 n = (reg.cpsr & CPSR_N_FLAG) ? 1 : 0;
				u8 v = (reg.cpsr & CPSR_V_FLAG) ? 1 : 0;

				if(n != v) { needs_flush = true; }
			}
	
			break;

		//BGT
		case 0xC:
			{
				u8 n = (reg.cpsr & CPSR_N_FLAG) ? 1 : 0;
				u8 v = (reg.cpsr & CPSR_V_FLAG) ? 1 : 0;
				u8 z = (reg.cpsr & CPSR_Z_FLAG) ? 1 : 0;

				if((z == 0) && (n == v)) { needs_flush = true; }
			}

			break;

		//BLE
		case 0xD:
			{
				u8 n = (reg.cpsr & CPSR_N_FLAG) ? 1 : 0;
				u8 v = (reg.cpsr & CPSR_V_FLAG) ? 1 : 0;
				u8 z = (reg.cpsr & CPSR_Z_FLAG) ? 1 : 0;

				if((z == 1) || (n != v)) { needs_flush = true; }
			}

			break;

		//Undefined
		case 0xE:
			std::cout<<"CPU::ARM9::Error - THUMB.16 Undefined opcode 0xE \n";
			running = false;
			break;

		//SWI
		case 0xF:
			//Process SWIs via HLE
			//TODO: Make and LLE version
			process_swi((current_thumb_instruction & 0xFF));
			break;
	}

	if(needs_flush)
	{
		//Clock CPU and controllers - 1N
		clock(reg.r15, CODE_N16);

		//Clock CPU and controllers - 2S 
		reg.r15 += offset;  
		clock(reg.r15, CODE_S16);
		clock((reg.r15 + 2), CODE_S16);
	}

	else 
	{
		//Clock CPU and controllers - 1S
		clock(reg.r15, CODE_S16);
	} 
}

/****** THUMB.18 Unconditional Branch ******/
void NTR_ARM9::unconditional_branch(u16 current_thumb_instruction)
{
	//Grab 11-bit offset - Bits 0-10
	u32 offset = (current_thumb_instruction & 0x7FF);
	offset <<= 1;

	//Convert Twos Complement
	if(offset & 0x800) { offset |= 0xFFFFF000; }

	needs_flush = true;

	//Clock CPU and controllers - 1N
	clock(reg.r15, CODE_N16);

	//Clock CPU and controllers - 2S 
	reg.r15 += offset;  
	clock(reg.r15, CODE_S16);
	clock((reg.r15 + 2), CODE_S16);
}

/****** THUMB.19 Long Branch with Link ******/
void NTR_ARM9::long_branch_link(u16 current_thumb_instruction)
{
	//Determine if this is the first or second instruction executed
	bool first_op = (((current_thumb_instruction >> 11) & 0x1F) == 0x1E) ? true : false;

	u32 lbl_addr = 0;

	//Perform 1st 16-bit operation
	if(first_op)
	{
		u8 pre_bit = (reg.r15 & 0x800000) ? 1 : 0;

		//Grab upper 11-bits of destination address
		lbl_addr = ((current_thumb_instruction & 0x7FF) << 12);
	
		//Add as a 2's complement to PC
		if(lbl_addr & 0x400000) { lbl_addr |= 0xFF800000; }
		lbl_addr += reg.r15;

		//Save label to LR
		set_reg(14, lbl_addr);

		//Clock CPU and controllers - 1S
		clock(reg.r15, CODE_S16);
	}

	//Perform 2nd 16-bit operation
	else
	{
		//Grab address of the "next" instruction to place in LR, set Bit 0 to 1
		u32 next_instr_addr = (reg.r15 - 2);
		next_instr_addr |= 1;

		//Grab lower 11-bits of destination address
		lbl_addr = get_reg(14);
		lbl_addr += ((current_thumb_instruction & 0x7FF) << 1);

		//Clock CPU and controllers - 1N
		clock(reg.r15, CODE_N16);

		reg.r15 = lbl_addr;
		reg.r15 &= ~0x1;

		needs_flush = true;
		set_reg(14, next_instr_addr);

		//Clock CPU and controllers - 2S
		clock(reg.r15, CODE_S16);
		clock((reg.r15 + 2), CODE_S16);

		//BLX
		if(((current_thumb_instruction >> 11) & 0x1F) == 0x1D)
		{
			arm_mode = ARM;
			reg.cpsr &= ~0x20;

			//Auto-align destination to word
			reg.r15 &= ~0x2;
		}
	}

	thumb_long_branch = true;
}
