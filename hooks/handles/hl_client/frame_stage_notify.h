#include <SDK/Interface.h>
#include "../../hooks.h"
#include <globals/settings.h>
#include <cheats/aimbot/backtrack/history.h>

#pragma once
void __fastcall raicu::hooks::handles::frame_stage_notify(c_hl_client* client, client_frame_stage_t stage)
{
	if (!handles::originals::frame_stage_notify)
	{
		return;
	}

	originals::frame_stage_notify(client, stage);

	if (stage == frame_net_update_end)
		history::update();
}