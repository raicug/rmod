cmake_minimum_required(VERSION 3.10)
project(rmod)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

set(DIRECTX_SDK_DIR "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)")

include_directories("${DIRECTX_SDK_DIR}/Include")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    link_directories("${DIRECTX_SDK_DIR}/Lib/x64")
else ()
    link_directories("${DIRECTX_SDK_DIR}/Lib/x86")
endif ()

if (MSVC)
    add_definitions(
            -D_AMD64_
            -D_WIN64
            -D_CRT_SECURE_NO_WARNINGS
            -DBUILDING_DLL
            -DUNICODE
            -D_UNICODE
    )
    add_compile_options(
            /WX-
            /wd4244
    )

    set(CMAKE_GENERATOR_PLATFORM x64)
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
endif ()

set(CMAKE_CXX_FLAGS_RELEASE "-O2")

include_directories(${CMAKE_SOURCE_DIR}/include)

set(MINHOOK_SOURCES
        include/ext/minhooks/buffer.c
        include/ext/minhooks/buffer.h
        include/ext/minhooks/minhook.c
        include/ext/minhooks/minhook.h
        include/ext/minhooks/trampoline.c
        include/ext/minhooks/trampoline.h
        include/ext/minhooks/hde/hde32.c
        include/ext/minhooks/hde/hde32.h
        include/ext/minhooks/hde/hde64.c
        include/ext/minhooks/hde/hde64.h
        include/ext/minhooks/hde/pstdint.h
        include/ext/minhooks/hde/table32.h
        include/ext/minhooks/hde/table64.h
)

set(IMGUI_SOURCES
        include/ext/imgui/imconfig.h
        include/ext/imgui/imgui.cpp
        include/ext/imgui/imgui.h
        include/ext/imgui/imgui_demo.cpp
        include/ext/imgui/imgui_draw.cpp
        include/ext/imgui/imgui_internal.h
        include/ext/imgui/imgui_tables.cpp
        include/ext/imgui/imgui_widgets.cpp
        include/ext/imgui/implot.cpp
        include/ext/imgui/implot.h
        include/ext/imgui/implot_demo.cpp
        include/ext/imgui/implot_internal.h
        include/ext/imgui/implot_items.cpp
        include/ext/imgui/imstb_rectpack.h
        include/ext/imgui/imstb_textedit.h
        include/ext/imgui/imstb_truetype.h
        include/ext/imgui/custom/imgui_notify.h
        include/ext/imgui/custom/TextEditor.cpp
        include/ext/imgui/custom/TextEditor.h
        include/ext/imgui/backends/imgui_impl_dx9.cpp
        include/ext/imgui/backends/imgui_impl_dx9.h
        include/ext/imgui/backends/imgui_impl_win32.cpp
        include/ext/imgui/backends/imgui_impl_win32.h)

set(ENCODING_SOURCES
        include/encoding/randstr.cpp
        include/encoding/randstr.h
        include/encoding/sha256.cpp
        include/encoding/sha256.h)

set(FONTS_SOURCES
        include/fonts/fa-regular-400.h
        include/fonts/fa-solid-900.h
        include/fonts/IconFontAwesome6.h
        include/fonts/Inter.h
        include/fonts/tahoma.h)

set(SDK_SOURCES
        include/sdk/Interface.h
        include/sdk/Interface.cpp
        include/sdk/source-sdk/math/vector.h
        include/sdk/source-sdk/math/radian_euler.cpp
        include/sdk/source-sdk/math/quaternion.cpp
        include/sdk/source-sdk/math/quaternion.h
        include/sdk/source-sdk/math/vector.cpp
        include/sdk/source-sdk/math/math.cpp
        include/sdk/source-sdk/math/radian_euler.h
        include/sdk/source-sdk/math/math.h
        include/sdk/memory/memory.cpp
        include/sdk/memory/memory.h
        include/sdk/utils/lua_utilities.cpp
        include/sdk/utils/lua_utilities.h
        include/sdk/utils/utilities.cpp
        include/sdk/utils/utilities.h)

set(CHEATS_SOURCES
        include/cheats/Visuals.cpp
        include/cheats/Visuals.h
        include/cheats/aimbot/backtrack/history.cpp
        include/cheats/aimbot/backtrack/history.h
        include/cheats/aimbot/backtrack/lag_compensation.cpp
        include/cheats/aimbot/backtrack/lag_compensation.h
        include/cheats/aimbot/helpers/engine_prediction.cpp
        include/cheats/aimbot/helpers/engine_prediction.h
        include/cheats/aimbot/helpers/predict_spread.cpp
        include/cheats/aimbot/helpers/predict_spread.h
        include/cheats/aimbot/aimbot.cpp
        include/cheats/aimbot/aimbot.h
        include/cheats/aimbot/penetrate_walls.cpp
        include/cheats/aimbot/penetrate_walls.h
        include/cheats/Drawing/Drawing.cpp
        include/cheats/Drawing/Drawing.h
        include/cheats/lua/lua.cpp
        include/cheats/lua/lua.h
        include/cheats/misc/movement.cpp
        include/cheats/misc/movement.h
        include/cheats/misc/misc.h
        include/cheats/misc/misc.cpp
        hooks/handles/d3d9/Reset.h
        hooks/handles/d3d9/Present.h
        public/d3d9_includes.h
        hooks/handles/html_panel/load_url.h
        hooks/handles/client_mode_shared/override_view.h
        hooks/handles/html_panel/paint.h)

add_library(rmod SHARED
        library.cpp
        include/logging/logger.cpp
        include/logging/logger.h
        include/globals/configManager.cpp
        include/globals/configManager.h
        include/globals/offsets.h
        include/globals/settings.h
        include/nlohmann/json.h
        include/themes.cpp
        include/themes.h
        ${MINHOOK_SOURCES}
        ${IMGUI_SOURCES}
        ${FONTS_SOURCES}
        ${ENCODING_SOURCES}
        ${SDK_SOURCES}
        ${CHEATS_SOURCES}
        hooks/hooks.cpp
        hooks/hooks.h
        gui/gui.cpp
        gui/gui.h
        gui/helpers/hotkey.cpp
        gui/helpers/hotkey.h
)


target_link_libraries(rmod
        winmm
        d3d9
        d3dx9
        dwmapi
        wininet
)

set_target_properties(rmod PROPERTIES
        OUTPUT_NAME "libcheat"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)