// EXT_DATA_USAGE_2
// GB Enhanced+ Copyright Daniel Baxter 2016
// Licensed under the GPLv2
// See LICENSE.txt for full license text

// File : chrono.fs
// Date : August 16, 2016
// Description : GBE+ Fragment Shader - Chrono
//
// Shades the screen with different colors depending on the current time
// Intended for black and white GB games to set the mood according to what time you're playing

#version 330 core
in vec2 texture_coordinates;

out vec4 color;

uniform sampler2D screen_texture;
uniform int screen_x_size;
uniform int screen_y_size;

uniform float ext_data_1;
uniform float ext_data_2;

void main()
{
	vec4 current_color = texture(screen_texture, texture_coordinates);
	vec4 blend_color = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 next_color = vec4(1.0, 1.0, 1.0, 1.0);

	float r_dist;
	float g_dist;
	float b_dist;

	//ext_data_1 is the current hour, 0-23
	float chrono_hour = ext_data_1;
	float chrono_minute = ext_data_2;

	//00:00 - 03:59 - Dark Blue to Blue
	if((chrono_hour >= 0) && (chrono_hour < 4))
	{
		chrono_minute += (chrono_hour * 60);

		blend_color = vec4(0.0, 0.14, 0.89, 1.0);
		next_color = vec4(0.09, 0.24, 0.75, 1.0);

		r_dist = (next_color.r - blend_color.r)/240.0;
		r_dist *= chrono_minute;
		blend_color.r += r_dist;

		g_dist = (next_color.g - blend_color.g)/240.0;
		g_dist *= chrono_minute;
		blend_color.g += g_dist;

		b_dist = (next_color.b - blend_color.b)/240.0;
		b_dist *= chrono_minute;
		blend_color.b += b_dist;

		current_color = mix(blend_color, current_color, 0.33);
	}

	//04:00 - 06:59 Blue to Red
	else if((chrono_hour >= 4) && (chrono_hour < 7))
	{
		chrono_minute += ((chrono_hour - 4) * 60);

		blend_color = vec4(0.09, 0.24, 0.75, 1.0);
		next_color = vec4(0.75, 0.16, 0.09, 1.0);

		r_dist = (next_color.r - blend_color.r)/180.0;
		r_dist *= chrono_minute;
		blend_color.r += r_dist;

		g_dist = (next_color.g - blend_color.g)/180.0;
		g_dist *= chrono_minute;
		blend_color.g += g_dist;

		b_dist = (next_color.b - blend_color.b)/180.0;
		b_dist *= chrono_minute;
		blend_color.b += b_dist;

		current_color = mix(blend_color, current_color, 0.33);
	}

	//07:00 - 9:59 Red to Yellow
	else if((chrono_hour >= 7) && (chrono_hour < 10))
	{
		chrono_minute += ((chrono_hour - 7) * 60);

		blend_color = vec4(0.75, 0.16, 0.09, 1.0);
		next_color = vec4(0.88, 0.88, 0.05, 1.0);

		r_dist = (next_color.r - blend_color.r)/180.0;
		r_dist *= chrono_minute;
		blend_color.r += r_dist;

		g_dist = (next_color.g - blend_color.g)/180.0;
		g_dist *= chrono_minute;
		blend_color.g += g_dist;

		b_dist = (next_color.b - blend_color.b)/180.0;
		b_dist *= chrono_minute;
		blend_color.b += b_dist;

		current_color = mix(blend_color, current_color, 0.33);
	}

	//10:00 - 12:59 Yellow to White-Yellow
	else if((chrono_hour >= 10) && (chrono_hour < 13))
	{
		chrono_minute += ((chrono_hour - 10) * 60);

		blend_color = vec4(0.88, 0.88, 0.05, 1.0);
		next_color = vec4(0.88, 0.88, 0.74, 1.0);

		r_dist = (next_color.r - blend_color.r)/180.0;
		r_dist *= chrono_minute;
		blend_color.r += r_dist;

		g_dist = (next_color.g - blend_color.g)/180.0;
		g_dist *= chrono_minute;
		blend_color.g += g_dist;

		b_dist = (next_color.b - blend_color.b)/180.0;
		b_dist *= chrono_minute;
		blend_color.b += b_dist;

		current_color = mix(blend_color, current_color, 0.33);
	}

	//13:00 - 16:59 White-Yellow to Red
	else if((chrono_hour >= 13) && (chrono_hour < 17))
	{
		chrono_minute += ((chrono_hour - 13) * 60);

		blend_color = vec4(0.88, 0.88, 0.74, 1.0);
		next_color = vec4(0.75, 0.16, 0.09, 1.0);

		r_dist = (next_color.r - blend_color.r)/240.0;
		r_dist *= chrono_minute;
		blend_color.r += r_dist;

		g_dist = (next_color.g - blend_color.g)/240.0;
		g_dist *= chrono_minute;
		blend_color.g += g_dist;

		b_dist = (next_color.b - blend_color.b)/240.0;
		b_dist *= chrono_minute;
		blend_color.b += b_dist;

		current_color = mix(blend_color, current_color, 0.33);
	}

	//17:00 - 19:59 Red to Light Blue
	else if((chrono_hour >= 17) && (chrono_hour < 20))
	{
		chrono_minute += ((chrono_hour - 17) * 60);

		blend_color = vec4(0.75, 0.16, 0.09, 1.0);
		next_color = vec4(0.03, 0.54, 1.0, 1.0);

		r_dist = (next_color.r - blend_color.r)/180.0;
		r_dist *= chrono_minute;
		blend_color.r += r_dist;

		g_dist = (next_color.g - blend_color.g)/180.0;
		g_dist *= chrono_minute;
		blend_color.g += g_dist;

		b_dist = (next_color.b - blend_color.b)/180.0;
		b_dist *= chrono_minute;
		blend_color.b += b_dist;

		current_color = mix(blend_color, current_color, 0.33);
	}

	//20:00 - 23:00 Light Blue to Dark Blue
	else
	{
		chrono_minute += ((chrono_hour - 20) * 60);

		blend_color = vec4(0.03, 0.54, 1.0, 1.0);
		next_color = vec4(0.0, 0.14, 0.89, 1.0);

		r_dist = (next_color.r - blend_color.r)/180.0;
		r_dist *= chrono_minute;
		blend_color.r += r_dist;

		g_dist = (next_color.g - blend_color.g)/180.0;
		g_dist *= chrono_minute;
		blend_color.g += g_dist;

		b_dist = (next_color.b - blend_color.b)/180.0;
		b_dist *= chrono_minute;
		blend_color.b += b_dist;

		current_color = mix(blend_color, current_color, 0.33);
	}

	if((texture(screen_texture, texture_coordinates).r == 0) && (texture(screen_texture, texture_coordinates).g == 0) && (texture(screen_texture, texture_coordinates).b == 0))
	{
		current_color = texture(screen_texture, texture_coordinates);
	}

	color = current_color;
}
