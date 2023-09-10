// GB Enhanced+ Copyright Daniel Baxter 2014
// Licensed under the GPLv2
// See LICENSE.txt for full license text

// File : core.h
// Date : August 16, 2014
// Description : Emulator core
//
// The "Core" is an abstraction for all of emulated components
// It controls the large-scale behavior of the CPU, LCD/GPU, MMU, and APU/DSP
// Can start, stop, and reset emulator
// Also contains a debugging unit


#ifndef GBA_CORE
#define GBA_CORE

#include "common/core_emu.h"
#include "mmu.h"
#include "arm7.h"

class AGB_core : virtual public core_emu
{
	public:
		AGB_core();
		~AGB_core();

		//Core control
		void start();
		void stop();
		void reset();
		void shutdown();
		void sleep();
		void step();
		void handle_hotkey(SDL_Event& event);
		void handle_hotkey(int input, bool pressed);
		void update_volume(u8 volume);
		void feed_key_input(int sdl_key, bool pressed);
		void save_state(u8 slot);
		void load_state(u8 slot);
		void run_core();
		void buffer_audio_data();

		//Core debugging
		void debug_step();
		void debug_display() const;
		void debug_process_command();
		std::string debug_get_mnemonic(u32 addr);
		std::string debug_get_mnemonic(u32 data, bool is_addr);

		//CPU related functions
		u32 ex_get_reg(u8 reg_index);

		//MMU related functions
		bool read_file(std::string filename);
		bool read_bios(std::string filename);
		bool read_firmware(std::string filename);
		u8 ex_read_u8(u16 address);
		void ex_write_u8(u16 address, u8 value);

		//Netplay interface
		void start_netplay();
		void stop_netplay();
		void hard_sync();

		//Misc
		u32 get_core_data(u32 core_index);

		AGB_MMU core_mmu;
		ARM7 core_cpu;
		AGB_GamePad core_pad;
};
		
#endif // GBA_CORE
