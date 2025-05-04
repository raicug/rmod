#pragma once

class i_client_renderable 
{
public:
	void* get_model()
	{
		return memory::call_v_function<void* (__thiscall*)(void*)>(this, 9)(this);
	}

	bool setup_bones(matrix3x4* bone_to_world, int max_bones, int mask, float curtime)
	{
		return memory::call_v_function<bool(__thiscall*)(void*, matrix3x4*, int, int, float)>(this, 16)(this, bone_to_world, max_bones, mask, curtime);
	}
};