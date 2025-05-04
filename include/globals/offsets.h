#ifndef OFFSETS_H
#define OFFSETS_H

#include <cstdint>
//#include <SDK/tier0/Color.h>
#include <string>
//#include <SDK/shared_lua/CLuaShared.h>
//#include <SDK/shared_lua/CLuaInterface.h>
//#include <SDK/VGui/VPanelWrapper.h>
#include <atomic>

namespace raicu::globals::offsets { // not used at ALL (lmfao)
    inline uintptr_t m_iClip1 = 0x40;
    inline uintptr_t m_flNextPrimaryAttach = 0x30;
    inline uintptr_t m_iHealth = 0xFFFFFFFFFFFFFFA9; // ???
    inline uintptr_t m_iTeamNum = 0x30;
    inline uintptr_t m_vecOrigin = 0x40;
    inline uintptr_t m_angRotation = 0x40;
    inline uintptr_t m_Hands = 0x40;

    inline std::atomic<uint64_t> waitingToBeExecuted;

    namespace sigs {
        inline std::string PresentPattern = "\x48\x8B\x05????\x4C\x8B\xC5\x49\x8B\xD7\x4C\x89\x64\x24?\x48\x8B\xCE\xFF\xD0";
        inline std::string GetClassNamePattern = "\xE8????\x4D\x8B\x47\x10";
        inline std::string CL_MovePattern = "\xE8????\xFF\x15????\xF2\x0F\x10\x0D????\x85\xFF";
        inline std::string PredictionSeedPattern = "\x48\x8B\xD1\x8B\x0D????";
        inline std::string ConColorMsgDec = "?ConColorMsg@@YAXAEBVColor@@PEBDZZ";
        inline std::string HostNamePattern = "\x48\x8D\x15????\x45\x33\xC0\x48\x8B\x01\xFF\x90????\xB8????\x48\x83\xC4\x28\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC";
        inline std::string MoveHelperPattern = "\x48\x89\x05????\xE9????\xCC\xCC\xCC\xCC\xCC\xCC\x48\x83\xEC\x28";
    }

}

#endif //OFFSETS_H
