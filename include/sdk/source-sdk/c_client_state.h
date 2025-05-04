#pragma once

class c_client_state
{
public:
	char	pad_0000[0x20B68];
	float	last_server_ticktime;
	int		in_simulation;
	int		old_tick_count;
	float	tick_remainder;
	float	frametime;
	int		last_out_going_command;
	int		choked_commands;
	int		last_command_ack;
	int		command_ack;
};