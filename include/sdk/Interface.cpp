#include <Windows.h>
#include <d3d9.h>

#include "Interface.h"
#include "./memory/memory.h"
#include <logging/logger.h>


// Define the memory functions
typedef void* (*interface_type)(const char* name, int* return_code);

template <typename T>
T* capture_interface(const char* module_name, const char* interface_name)
{
    HMODULE module_handle = GetModuleHandleA(module_name);
    if (!module_handle)
        throw std::runtime_error("Failed to get module handle");

    FARPROC proc_address = GetProcAddress(module_handle, "CreateInterface");
    if (!proc_address)
        throw std::runtime_error("Failed to get proc address");

    interface_type create_interface = reinterpret_cast<interface_type>(proc_address);
    if (!create_interface)
        throw std::runtime_error("Failed to create interface");

    T* interface = static_cast<T*>(create_interface(interface_name, nullptr));
    if (!interface)
        throw std::runtime_error("Failed to get interface");

    return interface;
}

// Pattern scanning method
const char* pattern_scanner(const char* module_name, std::string pattern) noexcept
{
    return memory::findPattern(module_name, pattern, "Pattern Scanner");
}

// Define relative to absolute conversion
constexpr uintptr_t relative_to_absolute(uintptr_t address, int offset, int instruction_size = 7)
{
    auto instruction = address + offset;
    int relative_address = *(int*)(instruction);
    return address + instruction_size + relative_address;
}

// Main initialize method
void interfaces::initialize()
{
    engine = capture_interface<c_engine_client>(xorstr("engine.dll"), xorstr("VEngineClient015"));
    if (!engine)
        throw std::runtime_error("Engine interface capture failed!");

    panel = capture_interface<i_panel>("vgui2.dll", "VGUI_Panel009");
    if (!panel)
        throw std::runtime_error("Panel interface capture failed!");

    render_view = capture_interface<i_render_view>("engine.dll", "VEngineRenderView014");
    if (!render_view)
        throw std::runtime_error("RenderView interface capture failed!");

    entity_list = capture_interface<c_entity_list>("client.dll", "VClientEntityList003");
    if (!entity_list)
        throw std::runtime_error("EntityList interface capture failed!");

    material_system = capture_interface<c_material_system>("materialsystem.dll", "VMaterialSystem080");
    if (!material_system)
        throw std::runtime_error("MaterialSystem interface capture failed!");

    hl_client = capture_interface<c_hl_client>("client.dll", "VClient017");
    if (!hl_client)
        throw std::runtime_error("HLClient interface capture failed!");

    engine_vgui = capture_interface<c_engine_vgui>("engine.dll", "VEngineVGui001");
    if (!engine_vgui)
        throw std::runtime_error("EngineVGui interface capture failed!");

    model_info = capture_interface<c_model_info>("engine.dll", "VModelInfoClient006");
    if (!model_info)
        throw std::runtime_error("ModelInfo interface capture failed!");

    engine_trace = capture_interface<i_engine_trace>("engine.dll", "EngineTraceClient003");
    if (!engine_trace)
        throw std::runtime_error("EngineTrace interface capture failed!");

    cvar = capture_interface<c_cvar>("vstdlib.dll", "VEngineCvar007");
    if (!cvar)
        throw std::runtime_error("Cvar interface capture failed!");

    model_render = capture_interface<c_model_render>("engine.dll", "VEngineModel016");
    if (!model_render)
        throw std::runtime_error("ModelRender interface capture failed!");

    prediction = capture_interface<i_prediction>("client.dll", "VClientPrediction001");
    if (!prediction)
        throw std::runtime_error("Prediction interface capture failed!");

    game_movement = capture_interface<i_game_movement>("client.dll", "GameMovement001");
    if (!game_movement)
        throw std::runtime_error("GameMovement interface capture failed!");

    physics_surface_props = capture_interface<i_physics_surface_props>("vphysics.dll", "VPhysicsSurfaceProps001");
    if (!physics_surface_props)
        throw std::runtime_error("PhysicsSurfaceProps interface capture failed!");

    lua_shared = memory::capture_interface<c_lua_shared>(xorstr("lua_shared.dll"), xorstr("LUASHARED003"));
    if (!lua_shared)
        throw std::runtime_error("LUASHARED003 interface capture failed!");

    view_render = memory::get_vmt_from_instruction<i_view_render>((uintptr_t)memory::pattern_scanner(xorstr("client.dll"), xorstr("48 8B 0D ? ? ? ? 48 8B 01 FF 50 18 48 8B 0D ? ? ? ? E8 ? ? ? ?")));
    if (!view_render)
        throw std::runtime_error("Failed to get view_render from signature");

    input = memory::get_vmt_from_instruction<c_input>((uintptr_t)memory::pattern_scanner(xorstr("client.dll"), xorstr("48 8B 0D ? ? ? ? 48 8B 01 48 FF A0 90 00 00 00 CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC 48 8B 0D ? ? ? ?")));
    if (!input)
        throw std::runtime_error("Failed to get input from signature");

    global_vars = *(c_global_vars**)memory::relative_to_absolute((uintptr_t)(memory::pattern_scanner(xorstr("client.dll"), xorstr("48 8B 05 ? ? ? ? 83 78 14 01 75 09"))), 0x3, 7);
    if (!global_vars)
        throw std::runtime_error("Failed to get global_vars from signature");;

    random_stream = memory::get_vmt_from_instruction<c_uniform_random_stream>((uintptr_t)GetProcAddress(GetModuleHandleA(xorstr("vstdlib.dll")), xorstr("RandomSeed")), 0x2);
    if (!random_stream)
        throw std::runtime_error("Failed to get random_stream");

    window = FindWindowW(L"Valve001", NULL);
    if (!window)
        throw std::runtime_error("Window capture failed!");

    logger::Log(5, "Successfully initiated interfaces");
}
