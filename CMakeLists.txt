﻿# CMakeList.txt : CMake project for yolahboy, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# set(VCPKG_TARGET_TRIPLET "x64-windows")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")

project ("yolahboy")

find_package(SDL3 REQUIRED)

set(IMGUI_SRC 
	./imgui/imgui.cpp
	./imgui/imgui_draw.cpp
	./imgui/imgui_tables.cpp
	./imgui/imgui_widgets.cpp
	./imgui/backends/imgui_impl_sdlrenderer3.cpp
	./imgui/backends/imgui_impl_sdl3.cpp
)

set(CORE_SRC
	"core/cpu/cpu.h"
	"core/cpu/cpu.c"
	"core/emulator.h"
	"core/emulator.c"
	"core/mmu/mmu.h"
	"core/mmu/mmu.c"
	"core/cpu/operation_definitions.h"
	"core/cpu/operations.h" 
	"core/cpu/operations.c" 
	"core/gpu/gpu.h" 
	"core/gpu/gpu.c" 
	"core/controller/controller.h" 
	"core/controller/controller.c"
	"core/timer/timer.c" 
	"core/timer/timer.h" 
	"core/mmu/cartridge.c" 
	"core/apu/apu.c"
)

add_library(core STATIC ${CORE_SRC})

# Add source to this project's executable.
add_executable (yolahboy WIN32 "yolahboy.c" "yolahboy.h" "debugger/debugger.cpp" "debugger/debugger.h" "debugger/imgui_memory_editor.h" ${IMGUI_SRC} "debugger/imgui_custom_widgets.h" "debugger/imgui_custom_widget_wrapper.h" "debugger/imgui_custom_widget_wrapper.cpp" "debugger/imgui_custom_widgets.cpp" "debugger/ImGuiFileDialog.cpp" )
target_include_directories(yolahboy PUBLIC ./imgui)
target_link_libraries(yolahboy PUBLIC SDL3::SDL3 core)
set_property(TARGET yolahboy PROPERTY CXX_STANDARD 20)

# TODO: Add tests and install targets if needed.
