#include <stdbool.h>

#include "emulator.h"
#include "global_definitions.h"
#include "./timer/timer.h"
#include "./controller/controller.h"
#include "./cpu/cpu.h"
#include "./mmu/mmu.h"
#include "./cpu/operations.h"
#include "./controller/controller.h"
#include "./gpu/gpu.h"
#include "./apu/apu.h"
#include "../debugger/imgui_custom_widget_wrapper.h"

#include <SDL3/SDL.h>

int init_emulator(Emulator* emu) {
	memset(&emu->cpu, 0, sizeof(Cpu));
	init_mmu(&emu->mmu);
	init_gpu(&emu->gpu);
	init_timer(&emu->timer);
	init_apu(&emu->apu, 48000, 1024);
	memset(&emu->controller, 0, sizeof(Controller));

	emu->should_run = false;
	emu->clock = 0;

	return 0;
}


void update_emu_controller(Emulator* emu, Controller controller) {
	emu->controller = controller;
}

int step(Emulator* emu) {
	Operation to_exec = get_operation(emu);
	Cycles c;
	if (!emu->cpu.halted) {
		c = cpu_step(emu, to_exec);
	}
	else {
		c = run_halted(emu);
	}
	if (c.t_cycles < 0) {
		return -1;
	}
	tick(emu, c.t_cycles);
	gpu_step(emu, c.t_cycles);
	apu_step(&emu->apu, c.t_cycles);

	return 0;
}


void destroy_emulator(Emulator* emu) {
	// if (emu->cpu) destroy_cpu(emu->cpu);
}

bool cartridge_loaded(Emulator* emu) {
	return emu->mmu.cartridge.rom != NULL;
}

void skip_bootrom(Emulator* emu) {
	emu->cpu.registers.a = 0x01;
	emu->cpu.registers.f = 0xB0;
	emu->cpu.registers.b = 0x00;
	emu->cpu.registers.c = 0x13;
	emu->cpu.registers.d = 0x00;
	emu->cpu.registers.e = 0xD8;
	emu->cpu.registers.h = 0x01;
	emu->cpu.registers.l = 0x4D;
	emu->cpu.registers.sp = 0xFFFE;
	emu->cpu.registers.pc = 0x0100;

	emu->mmu.in_bios = false;
}