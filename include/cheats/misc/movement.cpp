#include "movement.h"
#include <globals/settings.h>

void movement::run(c_user_cmd* cmd) {
    c_base_entity* local_player = interfaces::entity_list->get_entity(interfaces::engine->get_local_player());
    
    if (!local_player || !local_player->is_alive())
        return;

    if (raicu::globals::settings::movement::bhop)
    {

        if (cmd->buttons & IN_JUMP && !(local_player->get_flags() & FL_ONGROUND))
            cmd->buttons &= ~IN_JUMP;
    }
}

void movement::fix(c_user_cmd* cmd, c_user_cmd old) {
    c_base_entity* local_player = interfaces::entity_list->get_entity(interfaces::engine->get_local_player());
    if (!local_player || !local_player->is_alive())
        return;

    /*if (local_player->get_move_type() == MOVETYPE_NOCLIP || local_player->get_move_type() == MOVETYPE_LADDER || local_player->get_flags() & FL_INWATER)
        return;*/

    float delta_yaw = math::deg2rad(cmd->view_angles.y - old.view_angles.y);

    cmd->forward_move = (cos(delta_yaw) * old.forward_move) - (sin(delta_yaw) * old.side_move);
    cmd->side_move = (sin(delta_yaw) * old.forward_move) + (cos(delta_yaw) * old.side_move);
}