cmake_minimum_required(VERSION 3.21)
project(LumX VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_executable(LumX
    LumX/src/main.cpp
    LumX/src/Core/Application.cpp
    LumX/src/Core/SystemTrayIntegration.cpp
    LumX/src/Core/ConfigManager.cpp
    LumX/src/Core/MonitorManager.cpp
    LumX/src/Core/EventManager.cpp
    LumX/src/Core/AutoStartManager.cpp
    LumX/src/Core/AnimationManager.cpp
    LumX/src/Performance/PerformanceController.cpp
    LumX/src/Modules/Dock/DockModule.cpp
    LumX/src/Modules/TopBar/TopBarModule.cpp
    LumX/src/Modules/WindowEffects/WindowEffectsModule.cpp
)

target_include_directories(LumX PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/LumX/include)

target_link_libraries(LumX PRIVATE
    d2d1.lib
    dwrite.lib
    dcomp.lib
    dwmapi.lib
    user32.lib
    kernel32.lib
    shell32.lib
    shlwapi.lib
    psapi.lib
    wininet.lib
    ole32.lib
    windowscodecs.lib
    powrprof.lib
    shcore.lib
)

if(MSVC)
    target_compile_options(LumX PRIVATE /W4 /WX /permissive- /fp:precise)
    set_target_properties(LumX PROPERTIES
        VS_STARTUP_PROJECT LumX
        WIN32_EXECUTABLE ON
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(LumX PRIVATE /O2 /Oi /Ot)
endif()
