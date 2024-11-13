#pragma once

#include "global_definitions.h"

void update_emu_controller(Emulator* emu, Controller controller);
int init_emulator(Emulator* emu, const char* bootrom_path, const char* rom_path);
void destroy_emulator(Emulator* emu);
int step(Emulator* emu);