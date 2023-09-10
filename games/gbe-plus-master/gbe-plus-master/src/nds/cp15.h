// GB Enhanced Copyright Daniel Baxter 2016
// Licensed under the GPLv2
// See LICENSE.txt for full license text

// File : cp15.h
// Date : March 12 2016
// Description : ARM9 Coprocessor 15
//
// Handles the built in System Control Processor
// Reads and writes to coprocessor registers and performs system control based on that

#ifndef NDS9_CP15
#define NDS9_CP15

#include "common.h"

class CP15
{
	public:

	//CP15 register enumerations
	enum cp15_regs
	{
		C0_C0_0,
		C0_C0_1,
		C0_C0_2,
		C1_C0_0,
		C2_C0_0,
		C2_C0_1,
		C3_C0_0,
		C5_C0_0,
		C5_C0_1,
		C5_C0_2,
		C5_C0_3,

		C6_C0_0,
		C6_C1_0,
		C6_C2_0,
		C6_C3_0,
		C6_C4_0,
		C6_C5_0,
		C6_C6_0,
		C6_C7_0,

		C6_C0_1,
		C6_C1_1,
		C6_C2_1,
		C6_C3_1,
		C6_C4_1,
		C6_C5_1,
		C6_C6_1,
		C6_C7_1,

		C7_CM_XX,
		C9_C0_0,
		C9_C0_1,
		C9_C1_0,
		C9_C1_1,
		CP15_TEMP,
	};

	u32 regs[34];

	CP15();
	~CP15();

	//Control Register parameters
	bool pu_enable;
	bool unified_cache;
	bool instr_cache;
	u32 exception_vector;
	bool cache_replacement;
	bool pre_armv5;
	bool dtcm_enable;
	bool itcm_enable;

	void reset();

	void invalidate_instr_cache();
	void invalidate_instr_cache_line(u32 virtual_address);
	void invalidate_data_cache();
	void invalidate_data_cache_line(u32 virtual_address);
	void drain_write_buffer();
	void data_cache_lockdown();
	void instr_cache_lockdown();
	void set_dtcm_size_base();
	void set_itcm_size_base();
};

#endif // NDS9_CP15
