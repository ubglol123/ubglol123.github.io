// GB Enhanced+ Copyright Daniel Baxter 2022
// Licensed under the GPLv2
// See LICENSE.txt for full license text

// File : campho.cpp
// Date : December 28, 2022
// Description : Campho Advance
//
// Handles I/O for the Campho Advance (CAM-001)

#include "mmu.h"
#include "common/util.h"

/****** Resets Campho data structure ******/
void AGB_MMU::campho_reset()
{
	campho.data.clear();
	campho.g_stream.clear();

	campho.out_stream.clear();
	campho.config_data.clear();
	campho.config_data.resize(0x1C, 0x00);
	campho.contact_data.clear();
	campho.contact_data.resize(0x1C, 0x00);
	campho.read_out_stream = false;
	campho.out_stream_index = 0;

	campho.bank_index_lo = 0;
	campho.bank_index_hi = 0;
	campho.bank_id = 0;
	campho.rom_stat = 0xA00A;
	campho.rom_cnt = 0;
	campho.block_len = 0;
	campho.block_stat = 0;
	campho.bank_state = 0;
	campho.stream_started = false;
	campho.last_id = 0xFFFFFFFF;

	campho.video_capture_counter = 0;
	campho.video_frame_slice = 0;
	campho.video_frame_index = 0;
	campho.video_frame_size = 0;
	campho.capture_video = false;
	campho.new_frame = false;
	campho.is_large_frame = true;

	campho.dialed_number = "";

	campho.last_slice = 0;
	campho.repeated_slices = 0;
}

/****** Writes data to Campho I/O ******/
void AGB_MMU::write_campho(u32 address, u8 value)
{
	switch(address)
	{
		//Campho ROM Status
		case CAM_ROM_STAT:
		case CAM_ROM_STAT_B:
			campho.rom_stat &= 0xFF00;
			campho.rom_stat |= value;
			break;

		case CAM_ROM_STAT+1:
		case CAM_ROM_STAT_B+1:
			campho.rom_stat &= 0xFF;
			campho.rom_stat |= (value << 8);

			//Process commands to read Graphics ROM
			if(campho.rom_stat == 0x4015)
			{
				campho.stream_started = true;
			}

			break;

		//Campho ROM Control
		case CAM_ROM_CNT:
		case CAM_ROM_CNT_B:
			campho.rom_cnt &= 0xFF00;
			campho.rom_cnt |= value;
			break;

		case CAM_ROM_CNT+1:
			campho.rom_cnt &= 0xFF;
			campho.rom_cnt |= (value << 8);

			//Detect access to main Program ROM
			if(campho.rom_cnt == 0xA00A)
			{
				//Detect reading of first Program ROM bank
				if(!campho.block_stat)
				{
					campho.block_stat = 0xCC00;
					campho.bank_state = 1;

					std::cout<<"MMU::Campho Reading PROM Bank 0x" << campho.block_stat << "\n";
					u32 prom_bank_id = campho_get_bank_by_id(campho.block_stat);
					campho_set_rom_bank(campho.mapped_bank_id[prom_bank_id], campho.mapped_bank_index[prom_bank_id], false);
				}

				//Increment Program ROM bank
				else
				{
					campho.block_stat++;
					campho.bank_state = 1;

					//Signal end of Program ROM banks
					if(campho.block_stat == 0xCC10)
					{
						campho.block_stat = 0xCD00;
					}

					else if(campho.block_stat < 0xCC10)
					{
						std::cout<<"MMU::Campho Reading PROM Bank 0x" << campho.block_stat << "\n";
						u32 prom_bank_id = campho_get_bank_by_id(campho.block_stat);
						campho_set_rom_bank(campho.mapped_bank_id[prom_bank_id], campho.mapped_bank_index[prom_bank_id], false);
					}
				}
			}

			break;

		//Process Graphics ROM read requests
		case CAM_ROM_CNT_B+1:
			campho.rom_cnt &= 0xFF;
			campho.rom_cnt |= (value << 8);

			//Perform certain actions based on data from input stream (load graphics, camera commands, read/write settings)
			if(campho.rom_cnt == 0x4015)
			{
				campho_process_input_stream();
			}

			//Read next part of video camera framebuffer
			if((campho.rom_cnt == 0xA00A) && (campho.new_frame))
			{
				campho.video_frame_slice++;
				campho_set_video_data();

				campho.last_slice = campho.video_frame_slice;
				campho.repeated_slices = 0;
			}

			break;

		//Graphics ROM Stream
		case CAM_ROM_DATA_HI_B:
		case CAM_ROM_DATA_HI_B+1:
			if(campho.stream_started) { campho.g_stream.push_back(value); }
			break;
	}

	//std::cout<<"CAMPHO WRITE 0x" << address << " :: 0x" << (u32)value << "\n";
}

/****** Reads data from Campho I/O ******/
u8 AGB_MMU::read_campho(u32 address)
{
	u8 result = 0;

	switch(address)
	{
		//ROM Data Stream
		case CAM_ROM_DATA_LO:
			//Read Program ROM
			if(campho.bank_state)
			{
				//Return STAT LOW on first read
				if(campho.bank_state == 1)
				{
					result = (campho.block_stat & 0xFF);
				}

				//Return LEN LOW on second read
				//These 16-bit values should be fixed (0xFFA), except for last block (zero-length)
				else if(campho.bank_state == 2)
				{
					result = (campho.block_stat == 0xCD00) ? 0x00 : 0xFA;
				}
			}

			//Sequential ROM read
			else { result = read_campho_seq(address); }

			break;

		case CAM_ROM_DATA_LO+1:
			//Read Program ROM
			if(campho.bank_state)
			{
				//Return STAT HIGH on first read
				if(campho.bank_state == 1)
				{
					result = ((campho.block_stat >> 8) & 0xFF);
					campho.bank_state++;
				}

				//Return LEN HIGH on second read
				//These 16-bit values should be fixed (0xFFA), except for last block (zero-length)
				else if(campho.bank_state == 2)
				{
					result = (campho.block_stat == 0xCD00) ? 0x00 : 0x0F;
					campho.bank_state = (campho.block_stat == 0xCD00) ? 0x03 : 0x00;
				}
			}

			//Sequential ROM read
			else { result = read_campho_seq(address); }

			break;
		
		//Campho ROM Status
		case CAM_ROM_STAT:
		case CAM_ROM_STAT_B:
			result = (campho.rom_stat & 0xFF);
			break;

		case CAM_ROM_STAT+1:
		case CAM_ROM_STAT_B+1:
			result = ((campho.rom_stat >> 8) & 0xFF);
			break;

		//Campho ROM Control
		case CAM_ROM_CNT:
		case CAM_ROM_CNT_B:
			result = (campho.rom_cnt & 0xFF);
			break;

		case CAM_ROM_CNT+1:
		case CAM_ROM_CNT_B+1:
			result = ((campho.rom_cnt >> 8) & 0xFF);
			break;

		//Sequential ROM read
		default:
			result = read_campho_seq(address);
	}

	//std::cout<<"CAMPHO READ 0x" << address << " :: 0x" << (u32)result << "\n";
	return result;
}

/****** Reads sequential ROM data from Campho ******/
u8 AGB_MMU::read_campho_seq(u32 address)
{
	u8 result = 0;

	//Read Low ROM Data Stream
	if(address < 0x8008000)
	{
		if(campho.bank_index_lo < campho.data.size())
		{
			result = campho.data[campho.bank_index_lo++];
		}
	}

	//Read High ROM Data Stream, Camera Video Data, or Campho Config Data
	else
	{
		//Camera Video Data
		if(campho.new_frame)
		{
			if(campho.video_frame_index < campho.video_frame.size())
			{
				result = campho.video_frame[campho.video_frame_index++];
			}
		}

		//Misc Campho Data - Config Settings, Command Data, etc
		else if(campho.read_out_stream)
		{
			if(campho.out_stream_index < campho.out_stream.size())
			{
				result = campho.out_stream[campho.out_stream_index++];
			}
		}

		//ROM
		else
		{
			if(campho.bank_index_hi < campho.data.size())
			{
				result = campho.data[campho.bank_index_hi++];
			}
		}
	}

	return result;
}

/****** Handles processing Campho input stream for commands ******/
void AGB_MMU::campho_process_input_stream()
{
	campho.stream_started = false;
	campho.read_out_stream = false;

	u16 header = (campho.g_stream[0] | (campho.g_stream[1] << 8));

	//Determine action based on stream size
	if((!campho.g_stream.empty()) && (campho.g_stream.size() >= 4))
	{
		u32 pos = campho.g_stream.size() - 4;
		u32 index = (campho.g_stream[pos] | (campho.g_stream[pos+1] << 8) | (campho.g_stream[pos+2] << 16) | (campho.g_stream[pos+3] << 24));
		u16 param_1 = header;

		//Dial Phone Number
		if(header == 0x3740)
		{
			u16 number_len = (campho.g_stream[2] | (campho.g_stream[3] << 8));
			number_len = campho_convert_phone_number_length(number_len);
			
			campho.dialed_number = "";

			//Grab dialed number
			for(u32 x = 0, digit_index = 4; x < number_len; x++)
			{
				u16 val = (campho.g_stream[digit_index] | (campho.g_stream[digit_index + 1] << 8));

				//Even Digits
				if(x & 0x01)
				{
					campho.dialed_number += campho_convert_phone_number_even(val & 0x0FF0);
					digit_index += 2;
				}

				//Odd Digits
				else
				{
					campho.dialed_number += campho_convert_phone_number_odd(val & 0xF00F);
				}
			}

			std::cout<<"Dialed Phone Number: " << campho.dialed_number << "\n";	 
		}

		//Grab Graphics ROM data
		else if(campho.g_stream.size() == 0x0C)
		{
			u32 param_2 = (campho.g_stream[4] | (campho.g_stream[5] << 8) | (campho.g_stream[6] << 16) | (campho.g_stream[7] << 24));
			campho.last_id = param_2;

			//Set new Graphics ROM bank
			u32 g_bank_id = campho_get_bank_by_id(param_2, index);

			if(g_bank_id != 0xFFFFFFFF)
			{
				campho_set_rom_bank(campho.mapped_bank_id[g_bank_id], campho.mapped_bank_index[g_bank_id], true);
			}

			else
			{
				std::cout<<"Unknown Graphics ID -> 0x" << param_2 << "\n";
			}

			campho.video_capture_counter = 0;
			campho.new_frame = false;
			campho.video_frame_slice = 0;
			campho.last_slice = 0;

			std::cout<<"Graphics ROM ID -> 0x" << param_2 << "\n";
			std::cout<<"Graphics ROM Index -> 0x" << index << "\n";
		}

		//Camera commands
		else if(campho.g_stream.size() == 0x04)
		{
			//Stop camera?
			if(index == 0xF740)
			{
				campho.capture_video = false;

				campho.video_capture_counter = 0;
				campho.new_frame = false;
				campho.video_frame_slice = 0;
				campho.last_slice = 0;
			}

			//Turn on camera for large frame?
			else if(index == 0xD740)
			{
				campho.capture_video = true;
				campho.is_large_frame = true;

				//Large video frame = 176x144, drawn 12 lines at a time
				campho.video_frame_size = 176 * 12;

				campho.video_capture_counter = 0;
				campho.new_frame = false;
				campho.video_frame_slice = 0;
				campho.last_slice = 0;
			}

			//Switch to telecom mode??
			else if(index == 0xD778)
			{
				//Campho expects 16-bit status 0x7780
				campho.out_stream.clear();
				campho.out_stream.push_back(0x80);
				campho.out_stream.push_back(0x77);

				//Allow outstream to be read (until next stream)
				campho.out_stream_index = 0;
				campho.read_out_stream = true;
			}

			//Turn on camera for small frame?
			else if(index == 0xB740)
			{
				campho.capture_video = true;
				campho.is_large_frame = false;

				//Small video frame = 58x48, drawn 35 and 13 lines at a time
				campho.video_frame_size = 58 * 35;

				campho.video_capture_counter = 0;
				campho.new_frame = false;
				campho.video_frame_slice = 0;
				campho.last_slice = 0;
			}

			//Always end frame rendering
			else if(index == 0xFF9F)
			{
				campho.video_capture_counter = 0;
				campho.new_frame = false;
				campho.video_frame_slice = 0;
				campho.last_slice = 0;
			}

			else
			{
				std::cout<<"Unknown Camera Command Detected\n";
			} 

			std::cout<<"Camera Command -> 0x" << index << "\n";
		}

		//Change Campho settings
		else if(campho.g_stream.size() == 0x06)
		{
			u16 stream_stat = (campho.g_stream[1] << 8) | campho.g_stream[0];
			u16 hi_set = (index & 0xFFFF0000) >> 16;
			u16 lo_set = (index & 0xFFFF);

			//Read full settings
			if(stream_stat == 0xB778)
			{
				campho.out_stream.clear();

				//Set data to read from stream
				if(index == 0x1FFE4000)
				{
					for(u32 x = 0; x < campho.config_data.size(); x++)
					{
						campho.out_stream.push_back(campho.config_data[x]);
					}
				}

				else if(index == 0x4000)
				{
					for(u32 x = 0; x < campho.contact_data.size(); x++)
					{
						campho.out_stream.push_back(campho.contact_data[x]);
					}
				}
			}

			//Set microphone volume
			else if(stream_stat == 0x1742)
			{
				campho.mic_volume = campho_find_settings_val(hi_set);
				u32 read_data = (campho_convert_settings_val(campho.speaker_volume) << 16) | 0x4000;
				campho_make_settings_stream(read_data);
			}

			//Set speaker volume
			else if(stream_stat == 0x3742)
			{
				campho.speaker_volume = campho_find_settings_val(hi_set);
				u32 read_data = (campho_convert_settings_val(campho.speaker_volume) << 16) | 0x4000;
				campho_make_settings_stream(read_data);
			}

			//Set video brightness
			else if(stream_stat == 0x5742)
			{
				campho.video_brightness = campho_find_settings_val(hi_set);
				u32 read_data = (campho_convert_settings_val(campho.speaker_volume) << 16) | 0x4000;
				campho_make_settings_stream(read_data);
			}

			//Set video contrast
			else if(stream_stat == 0x7742)
			{
				campho.video_contrast = campho_find_settings_val(hi_set);
				u32 read_data = (campho_convert_settings_val(campho.speaker_volume) << 16) | 0x4000;
				campho_make_settings_stream(read_data);
			}

			//Allow settings to be read now (until next stream)
			campho.out_stream_index = 0;
			campho.read_out_stream = true;

			campho.video_capture_counter = 0;
			campho.new_frame = false;
			campho.video_frame_slice = 0;
			campho.last_slice = 0;

			std::cout<<"Campho Settings -> 0x" << index << " :: 0x" << stream_stat << "\n";
		}

		//Save Campho settings changes
		else if(campho.g_stream.size() == 0x1C)
		{
			u32 sub_header = (campho.g_stream[4] | (campho.g_stream[5] << 8) | (campho.g_stream[6] << 16) | (campho.g_stream[7] << 24));

			//Save configuration settings
			if(sub_header == 0xFFFF1FFE)
			{
				campho.config_data.clear();

				//32-bit metadata
				campho.config_data.push_back(0x31);
				campho.config_data.push_back(0x08);
				campho.config_data.push_back(0x03);
				campho.config_data.push_back(0x00);

				for(u32 x = 4; x < 0x1C; x++) { campho.config_data.push_back(campho.g_stream[x]); }

				//Allow settings to be read now (until next stream)
				campho.out_stream_index = 0;
				campho.read_out_stream = true;

				std::cout<<"Campho Config Saved\n";
			}

			//Save Name + Phone Number
			else if(sub_header == 0xFFFFFFFF)
			{
				campho.contact_data.clear();

				//32-bit metadata
				campho.contact_data.push_back(0x31);
				campho.contact_data.push_back(0x08);
				campho.contact_data.push_back(0x03);
				campho.contact_data.push_back(0x00);

				for(u32 x = 4; x < 0x1C; x++) { campho.contact_data.push_back(campho.g_stream[x]); }

				//Allow outstream to be read (until next stream)
				campho.out_stream_index = 0;
				campho.read_out_stream = true;

				std::string contact_number = "";

				//Parse incoming data for contact phone number (last 10 bytes of stream)
				for(u32 x = 0, digit_index = 18; x < 10; x++)
				{
					u16 val = (campho.g_stream[digit_index] | (campho.g_stream[digit_index + 1] << 8));

					//Even Digits
					if(x & 0x01)
					{
						contact_number += campho_convert_phone_number_even(val & 0x0FF0);
						digit_index += 2;
					}

					//Odd Digits
					else
					{
						contact_number += campho_convert_phone_number_odd(val & 0xF00F);
					}
				}

				std::cout<<"Contact Number: " << contact_number << "\n";
				std::cout<<"Campho Added Contact\n";
			}

			else
			{
				std::cout<<"Saving unknown settings: 0x" << sub_header << "\n";
			}

			campho.video_capture_counter = 0;
			campho.new_frame = false;
			campho.video_frame_slice = 0;
			campho.last_slice = 0;
		}

		else
		{
			std::cout<<"Unknown Campho Input. Size -> 0x" << campho.g_stream.size() << "\n";
		}
	}

	campho.g_stream.clear();
}

/****** Sets the absolute position within the Campho ROM for a bank's base address ******/
void AGB_MMU::campho_set_rom_bank(u32 bank, u32 address, bool set_hi_bank)
{
	//Abort if invalid bank is set
	if(bank == 0xFFFFFFFF) { return; }

	//Search all known banks for a specific ID
	for(u32 x = 0; x < campho.mapped_bank_id.size(); x++)
	{
		//Match bank ID and base address
		if((campho.mapped_bank_id[x] == bank) && (campho.mapped_bank_index[x] == address))
		{
			//Set High ROM bank
			if(set_hi_bank) { campho.bank_index_hi = campho.mapped_bank_pos[x]; }

			//Set Low ROM bank
			else { campho.bank_index_lo = campho.mapped_bank_pos[x]; }

			campho.block_len = campho.mapped_bank_len[x];
			return;
		}
	}

	std::cout<<"MMU::Warning - Campho Advance ROM position for Bank 0x" << bank << " @ 0x" << address << " was not found\n";
}

/****** Maps various ROM banks for the Campho ******/
void AGB_MMU::campho_map_rom_banks()
{
	//Currently it is unknown how much ROM data needs to be dumped from the Campho Advance
	//Also, the way the hardware maps data is not entirely understood and it's all over the place
	//Until more research is done, a basic PROVISIONAL mapper is implemented here
	//GBE+ will accept a ROM with a header that with the following data (MSB first!):

	//TOTAL HEADER LENGTH		1st 4 bytes
	//BANK ENTRIES			... rest of the header, see below for Bank Entry format

	//BANK ID			4 bytes
	//BANK INDEX			4 bytes
	//BANK LENGTH IN BYTES		4 bytes

	//Grab ROM header length
	if(campho.data.size() < 4) { return; }

	u32 header_len = (campho.data[0] << 24) | (campho.data[1] << 16) | (campho.data[2] << 8) | campho.data[3];

	u32 rom_pos = 0;
	u32 bank_pos = header_len;

	campho.mapped_bank_id.clear();
	campho.mapped_bank_index.clear();
	campho.mapped_bank_len.clear();
	campho.mapped_bank_pos.clear();

	//Grab bank entries and parse them accordingly
	for(u32 header_index = 4; header_index < header_len;)
	{
		rom_pos = header_index;
		if((rom_pos + 12) >= campho.data.size()) { break; }

		u32 bank_id = (campho.data[rom_pos] << 24) | (campho.data[rom_pos+1] << 16) | (campho.data[rom_pos+2] << 8) | campho.data[rom_pos+3];
		u32 bank_addr = (campho.data[rom_pos+4] << 24) | (campho.data[rom_pos+5] << 16) | (campho.data[rom_pos+6] << 8) | campho.data[rom_pos+7];
		u32 bank_len = (campho.data[rom_pos+8] << 24) | (campho.data[rom_pos+9] << 16) | (campho.data[rom_pos+10] << 8) | campho.data[rom_pos+11];

		campho.mapped_bank_id.push_back(bank_id);
		campho.mapped_bank_index.push_back(bank_addr);
		campho.mapped_bank_len.push_back(bank_len);
		campho.mapped_bank_pos.push_back(bank_pos);

		bank_pos += bank_len;
		header_index += 12;
	}

	//Setup initial BS1 and BS2
	u32 bs1_bank = campho_get_bank_by_id(0x08000000);
	u32 bs2_bank = campho_get_bank_by_id(0x08008000);
	
	campho_set_rom_bank(campho.mapped_bank_id[bs1_bank], campho.mapped_bank_index[bs1_bank], false);
	campho_set_rom_bank(campho.mapped_bank_id[bs2_bank], campho.mapped_bank_index[bs2_bank], true);
}

/****** Returns the internal ROM bank GBE+ needs - Mapped to the Campho Advance's IDs ******/
u32 AGB_MMU::campho_get_bank_by_id(u32 id)
{
	for(u32 x = 0; x < campho.mapped_bank_id.size(); x++)
	{
		if(campho.mapped_bank_id[x] == id) { return x; }
	}

	return 0xFFFFFFFF;
}

/****** Returns the internal ROM bank GBE+ needs - Mapped to the Campho Advance's IDs ******/
u32 AGB_MMU::campho_get_bank_by_id(u32 id, u32 index)
{
	for(u32 x = 0; x < campho.mapped_bank_id.size(); x++)
	{
		if((campho.mapped_bank_id[x] == id) && (campho.mapped_bank_index[x] == index)) { return x; }
	}

	return 0xFFFFFFFF;
}

/****** Processes regular events such as audio/video capture for the Campho Advance ******/
void AGB_MMU::process_campho()
{
	campho.video_capture_counter++;

	if(campho.video_capture_counter < 12)
	{
		return;
	}

	else { campho.video_capture_counter = 0; }

	//Abort/Finish video rendering midframe if delayed by other I/O like ROM
	if(campho.last_slice == campho.video_frame_slice)
	{
		campho.repeated_slices++;

		if(campho.repeated_slices == 2)
		{
			campho.rom_stat = 0x4015;
			campho.last_slice = 0;
			campho.repeated_slices = 0;
			campho.video_capture_counter = 0;
			campho.new_frame = false;
			campho.video_frame_slice = 0;
			return;
		}
	}

	//Update video capture data with new pixels for current frame - Update at ~5FPS
	if((campho.capture_video) && (!campho.new_frame))
	{
		campho.new_frame = true;
		campho.video_frame_slice = 0;
		campho.last_slice = 0;
		campho.rom_stat = 0xA00A;

		//Grab pixel data for captured video frame
		//Pull data from BMP file
		SDL_Surface* source = SDL_LoadBMP(config::external_camera_file.c_str());

		if(source != NULL)
		{
			SDL_Surface* temp_bmp = SDL_CreateRGBSurface(SDL_SWSURFACE, source->w, source->h, 32, 0, 0, 0, 0);
			u8* cam_pixel_data = (u8*)source->pixels;
			campho_get_image_data(cam_pixel_data, source->w, source->h);	
		}

		campho_set_video_data();
	}
}

/****** Sets the framebuffer data for Campho's video input ******/
void AGB_MMU::campho_set_video_data()
{
	//Setup new frame data
	campho.video_frame.clear();
	campho.video_frame_index = 0;
	u16 frame_msb = (campho.is_large_frame) ? 0xAA00 : 0xA900;

	//2-byte metadata, position and size of frame
	u16 pos = frame_msb + campho.video_frame_slice;
	pos &= (campho.is_large_frame) ? 0xFFFF : 0xFF01;
	pos = ((pos >> 3) | (pos << 13));

	u16 v_size = campho.video_frame_size / 4;
	u8 line_size = (campho.is_large_frame) ? 176 : 58;

	u8 slice_limit_prep = campho.is_large_frame ? 13 : 2;
	u8 slice_limit_end = campho.is_large_frame ? 14 : 3;

	//Switch video size from 58*35 to 58*13 for 2nd part of small video frame rendering
	//Normally okay to do 58*35 twice, except when changing settings (which draws garbage data)
	if((!campho.is_large_frame) && (campho.video_frame_slice == 1))
	{
		v_size = (58 * 13) / 4;
	}

	//Make sure number of bytes to read is a multiple of 4
	if(v_size & 0x3) { v_size += (4 - (v_size & 0x3)); }

	//Check whether the video frame has been fully rendered
	//In that case, set position and size to 0xCFFF and 0x00 respectively
	if(campho.video_frame_slice == slice_limit_prep)
	{
		pos = 0xF9FF;
		v_size = 0;
	}

	//Check whether 0xCFFF has been previously sent, end video frame rendering now
	//Forcing ROM_STAT to 0x4015 signals end all of video frame data from Campho
	else if(campho.video_frame_slice == slice_limit_end)
	{
		campho.rom_stat = 0x4015;
		return;
	}

	campho.video_frame.push_back(pos & 0xFF);
	campho.video_frame.push_back(pos >> 8);
	campho.video_frame.push_back(v_size & 0xFF);
	campho.video_frame.push_back(v_size >> 8);

	if(!v_size) { return; }

	u32 line_pos = campho.video_frame_slice;
	line_pos *= (campho.is_large_frame) ? 12 : 35;

	if(campho.is_large_frame)
	{
		if(campho.video_frame_slice != 0) { line_pos -= campho.video_frame_slice; }
	}

	line_pos *= (line_size * 2);

	for(u32 x = 0; x < campho.video_frame_size * 2; x++)
	{	
		if(line_pos < campho.capture_buffer.size())
		{
			campho.video_frame.push_back(campho.capture_buffer[line_pos++]);
		}
	}
}

/****** Converts 24-bit RGB data into 15-bit GBA colors for Campho video buffer ******/
void AGB_MMU::campho_get_image_data(u8* img_data, u32 width, u32 height)
{
	u32 len = width * height;
	u32 data_index = 0;
	std::vector <u8> temp_buffer;

	u8 target_width = (campho.is_large_frame) ? 176 : 58;
	u8 target_height = (campho.is_large_frame) ? 144 : 48;

	//Grab original image data, scale later if necessary
	for(u32 x = 0; x < len; x++)
	{
		u8 r = (img_data[data_index + 2] >> 3);
		u8 g = (img_data[data_index + 1] >> 3);
		u8 b = (img_data[data_index] >> 3);

		u16 color = ((b << 10) | (g << 5) | r);
		color = ((color >> 3) | (color << 13));

		temp_buffer.push_back(color & 0xFF);
		temp_buffer.push_back(color >> 0x08);

		data_index += 3;
	}

	campho.capture_buffer.clear();

	//Calculate X and Y ratio for stretching/shrinking
	float x_ratio = float(width) / target_width;
	float y_ratio = float(height) / target_height;

	u32 x_pos = 0;
	u32 y_pos = 0;
	u32 pos = 0;

	u32 target_len = target_width * target_height;

	for(u32 x = 0; x < target_len; x++)
	{
		u32 x_pos = (x % target_width) * x_ratio;
		u32 y_pos = (x / target_width) * y_ratio;
		u32 pos = ((y_pos * width) + x_pos) * 2;

		if(pos < temp_buffer.size())
		{
			campho.capture_buffer.push_back(temp_buffer[pos]);
			campho.capture_buffer.push_back(temp_buffer[pos+1]);
		}
	}
}

/****** Finds the regular integer value of the settings the Campho Advance uses ******/
u8 AGB_MMU::campho_find_settings_val(u16 input)
{
	for(u32 x = 0; x <= 10; x++)
	{
		u32 test = ((x << 14) | x);
		test += ((test & 0xFFFF0000) >> 16);
		test &= 0xFFFF;

		if(input == test) { return x; }
	}

	return 0;
}

/****** Converts a regular integer value into the same format the Campho Advances uses for settings ******/
u16 AGB_MMU::campho_convert_settings_val(u8 input)
{
	u32 result = ((input << 14) | input);
	result += ((result & 0xFFFF0000) >> 16);
	result &= 0xFFFF;
	return result;
}

/****** Makes an 8-byte stream that returns the a current settings value when read by the Campho ******/
void AGB_MMU::campho_make_settings_stream(u32 input)
{
	campho.out_stream.clear();

	//Set data to read from stream
	campho.out_stream.push_back(0x20);
	campho.out_stream.push_back(0x68);
	campho.out_stream.push_back(input);
	campho.out_stream.push_back(input >> 8);
	campho.out_stream.push_back(input >> 16);
	campho.out_stream.push_back(input >> 24);
	campho.out_stream.push_back(0x00);
	campho.out_stream.push_back(0x60);
}

/****** Converts a 16-bit value used by the Campho for number of digits in a phone number ******/
u8 AGB_MMU::campho_convert_phone_number_length(u16 input)
{
	u8 result = 0;

	switch(input)
	{
		case 0x2000: result = 1; break;
		case 0x4000: result = 2; break;
		case 0x6000: result = 3; break;
		case 0x8000: result = 4; break;
		case 0xA000: result = 5; break;
		case 0xC000: result = 6; break;
		case 0xE000: result = 7; break;
		case 0x0001: result = 8; break;
		case 0x2001: result = 9; break;
		case 0x4001: result = 10; break;
	}

	return result;
}

/****** Converts a 16-bit value used by the Campho for odd digits in a phone number ******/
std::string AGB_MMU::campho_convert_phone_number_odd(u16 input)
{
	std::string result = "";

	switch(input)
	{
		case 0xC005: result = "."; break;
		case 0x0006: result = "0"; break;
		case 0x2006: result = "1"; break;
		case 0x4006: result = "2"; break;
		case 0x6006: result = "3"; break;
		case 0x8006: result = "4"; break;
		case 0xA006: result = "5"; break;
		case 0xC006: result = "6"; break;
		case 0xE006: result = "7"; break;
		case 0x0007: result = "8"; break;
		case 0x2007: result = "9"; break;
	}

	return result;
}

/****** Converts a 16-bit value used by the Campho for even digits in a phone number ******/
std::string AGB_MMU::campho_convert_phone_number_even(u16 input)
{
	std::string result = "";

	switch(input)
	{
		case 0x05C0: result = "."; break;
		case 0x0600: result = "0"; break;
		case 0x0620: result = "1"; break;
		case 0x0640: result = "2"; break;
		case 0x0660: result = "3"; break;
		case 0x0680: result = "4"; break;
		case 0x06A0: result = "5"; break;
		case 0x06C0: result = "6"; break;
		case 0x06E0: result = "7"; break;
		case 0x0700: result = "8"; break;
		case 0x0720: result = "9"; break;
	}

	return result;
}
