#include <stdio.h>

extern "C" {
#include <SDL3/SDL.h>
#include "debugger.h"
#include "../components/global_definitions.h"
#include "../components/emulator.h"
#include "../components/controller.h"
#include "../components/memory2.h"
#include "../components/operations.h"
#include "../components/operation_defitions.h"
	extern Operation operations[];
	extern Operation cb_operations[];
}

#include "imgui.h"
#include "./backends/imgui_impl_sdl3.h"
#include "./backends/imgui_impl_sdlrenderer3.h"
#include "../imgui_memory_editor.h"
#include "./imgui_custom_widgets.h"


bool breakpoints[0x10000];
ImVec4 clear_color = {
		0.45f,
		0.55f,
		0.60f,
		1.00f
};

void init_debug_ui(SDL_Window* window, SDL_Renderer* renderer, ImGuiContext* ig_ctx, ImGuiIO* ioptr) {
	if (ig_ctx == NULL) {
		ig_ctx = ImGui::CreateContext(NULL);
		ImGui::StyleColorsDark(NULL);

		ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
		ImGui_ImplSDLRenderer3_Init(renderer);
	}
	if (ioptr == NULL) {
		ioptr = &ImGui::GetIO();
		ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ioptr->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	}

}

void draw_debug_ui(SDL_Window* window, SDL_Renderer* renderer, ImGuiContext* ig_ctx, ImGuiIO* ioptr, Emulator* emu, SDL_FRect* emulator_screen_rect, SDL_FRect* tile_screen_rect, bool run_once) {

	static MemoryEditor ram_viewer;
	static ExampleAppLog log;
	static ImGuiListClipper clipper;

	static int DEBUGGER_X;
	static int DEBUGGER_Y;
	static int SCREEN_X = 0;
	static int SCREEN_Y = 0;
	static int TILE_X = 0;
	static int TILE_Y = 0;
	static int INSTRUCTION_X;
	static int INSTRUCTION_Y;
	static int TABS_X;
	static int TABS_Y;

	ImGui_ImplSDL3_NewFrame();
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui::NewFrame();

	// Do all ImGui widgets here

	ImGui::SetNextWindowSize({ (float)(800 - (SCREEN_X + TILE_X)) / 2, (float)SCREEN_Y});
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::Begin("DEBUGGER", NULL, ImGuiWindowFlags_NoResize);
	DEBUGGER_X = ImGui::GetWindowSize().x;
	DEBUGGER_Y = ImGui::GetWindowSize().y;
	if (ImGui::Button("RUN", { 40, 15 })) {
		emu->should_run = true;
	}

	ImGui::BeginChild("REGISTERS");

	char af_buf[10];
	char bc_buf[10];
	char de_buf[10];
	char hl_buf[10];
	char sp_buf[10];
	char pc_buf[10];

	sprintf(af_buf, "AF: %04hX", emu->cpu->registers.af);
	sprintf(bc_buf, "BC: %04hX", emu->cpu->registers.bc);
	sprintf(de_buf, "DE: %04hX", emu->cpu->registers.de);
	sprintf(hl_buf, "HL: %04hX", emu->cpu->registers.hl);
	sprintf(sp_buf, "SP: %04hX", emu->cpu->registers.sp);
	sprintf(pc_buf, "PC: %04hX", emu->cpu->registers.pc);

	ImGui::Text(af_buf);
	ImGui::Text(bc_buf);
	ImGui::Text(de_buf);
	ImGui::Text(hl_buf);
	ImGui::Text(sp_buf);
	ImGui::Text(pc_buf);
	ImGui::EndChild();
	ImGui::End();

	ImGui::SetNextWindowContentSize({ 160 * 2, 144 * 2 });
	ImGui::SetNextWindowPos({ (float)DEBUGGER_X ,(float)0 });
	if (emu->should_run) ImGui::SetNextWindowFocus();
	ImGui::Begin("SCREEN", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
	{
		SCREEN_X = ImGui::GetWindowSize().x;
		SCREEN_Y = ImGui::GetWindowSize().y;

		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();
		ImVec2 windowpos = ImGui::GetWindowPos();

		emulator_screen_rect->x = vMin.x + windowpos.x;
		emulator_screen_rect->y = vMin.y + windowpos.y;
		emulator_screen_rect->w = vMax.x - vMin.x;
		emulator_screen_rect->h = vMax.y - vMin.y;

	}
	ImGui::End();

	ImGui::SetNextWindowContentSize({ 128, 144 * 2 });
	ImGui::SetNextWindowPos({ (float)DEBUGGER_X + SCREEN_X, (float)0 });
	ImGui::Begin("TILES", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
	{
		TILE_X = ImGui::GetWindowSize().x;
		TILE_Y = ImGui::GetWindowSize().y;

		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();
		ImVec2 windowpos = ImGui::GetWindowPos();

		tile_screen_rect->x = vMin.x + windowpos.x;
		tile_screen_rect->y = vMin.y + windowpos.y;
		tile_screen_rect->w = vMax.x - vMin.x;
		tile_screen_rect->h = 192;
	}
	ImGui::End();

	ImGui::SetNextWindowSize({ (float)(800 - (SCREEN_X + TILE_X)) / 2, (float)600 });
	ImGui::SetNextWindowPos({ (float)DEBUGGER_X + SCREEN_X + TILE_X, (float)0 });
	ImGui::Begin("INSTRUCTIONS", NULL, ImGuiWindowFlags_NoResize);
	INSTRUCTION_X = ImGui::GetWindowSize().x;
	INSTRUCTION_Y = ImGui::GetWindowSize().y;

	u16 currentPC = emu->cpu->registers.pc;

	clipper.Begin(0x10000);
	while (clipper.Step()) {
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
			Operation* currentOp = &operations[read8(emu->memory, i)];
			if (currentOp->type == CB) {
				++i;
				clipper.DisplayEnd += 1;
				currentOp = &cb_operations[read8(emu->memory, i)];
			}

			char op_buf[10];
			sprintf(op_buf, "%04hX", i);

			ImGui::Checkbox(op_buf, &breakpoints[i]);
			ImGui::SameLine();
			if (currentOp->type != UNIMPLEMENTED) ImGui::Text(currentOp->mnemonic);
			else {
				char unimpl_buf[30];
				sprintf(unimpl_buf, "UNIMPL OP: 0x%02hX", read8(emu->memory, i));
				ImGui::Text(unimpl_buf);
			}
			switch (currentOp->source_addr_mode) {
			case MEM_READ:
			case MEM_READ_ADDR_OFFSET:
				clipper.DisplayEnd += 1;
				i += 1;
				sprintf(op_buf, "%04hX", read8(emu->memory, i));

				ImGui::SameLine();
				ImGui::Text(op_buf);
				break;
			case MEM_READ16:
				clipper.DisplayEnd += 2;
				i += 2;
				sprintf(op_buf, "%04hX", read16(emu->memory, i - 1));
				ImGui::SameLine();
				ImGui::Text(op_buf);
				break;
			}
		}
		if (run_once) {
			float item_pos_y = clipper.ItemsHeight * (emu->cpu->registers.pc) + clipper.ItemsHeight;
			ImGui::SetScrollY(item_pos_y);
		}

	}
	ImGui::End();

	ImGui::SetNextWindowSize({ (float)DEBUGGER_X + SCREEN_X + TILE_X, (float)600 - SCREEN_Y });
	ImGui::SetNextWindowPos({ (float)0, (float)DEBUGGER_Y });
	ImGui::Begin("OTHER", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
	ImGui::BeginTabBar("##SETTINGSTABS");
	if (ImGui::BeginTabItem("RAM")) {
		ram_viewer.DrawContents(emu->memory->memory, 0x10000);
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("CONSOLE")) {
		// TODO
		log.Draw();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("SETTINGS")) {
		// TODO
		ImGui::Text("Settings");
		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
	ImGui::End();
	ImGui::Render();
}

void destroy_debug_ui(SDL_Window* window, SDL_Renderer* renderer, ImGuiContext* ig_ctx, ImGuiIO* ioptr) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();

	ImGui::DestroyContext(ig_ctx);
}

//int debugger_run_threaded(HANDLE emulator_thread, args* t_args) {
//
//	SDL_Window* window = SDL_CreateWindow("Yolahboy Debugger", 400, 400, 0);
//	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
//
//	if (window == NULL) {
//		printf("could not initialize debugger window");
//		return -1;
//	}
//
//	if (renderer == NULL) {
//		printf("could not initialize debugger renderer");
//		SDL_DestroyWindow(window);
//		return -1;
//	}
//
//	if (renderer == NULL) {
//		printf("could not initialize debugger renderer");
//		SDL_DestroyWindow(window);
//		return -1;
//	}
//
//	EnterCriticalSection(&emu_crit);
//	if (init_emulator(&emu, t_args->argv[1], t_args->argv[2], t_args->breakpoint_arr) < 0) {
//		destroy_emulator(&emu);
//	}
//	LeaveCriticalSection(&emu_crit);
//
//	ResumeThread(emulator_thread);
//
//	ImGuiContext* ig_ctx = NULL;
//	ImGuiIO* ioptr = NULL;
//
//	init_debug_ui(window, renderer, ig_ctx, ioptr);
//
//	bool quit = false;
//	while (!quit) {
//
//		switch (WaitForSingleObject(emu_breakpoint_event, 0)) {
//		case WAIT_OBJECT_0:
//			EnterCriticalSection(&emu_crit);
//			if (emu.cpu == NULL) {
//				quit = true;
//				if (emu.tile_window) SDL_DestroyWindow(emu.tile_window);
//				if (emu.emulator_renderer) SDL_DestroyRenderer(emu.emulator_renderer);
//				if (emu.tile_renderer) SDL_DestroyRenderer(emu.tile_renderer);
//				if (emu.emulator_window) SDL_DestroyWindow(emu.emulator_window);
//				if (emu.game_controller) SDL_CloseGamepad(emu.game_controller);
//				destroy_emulator(&emu);
//				break;
//			}
//			print_registers(emu.cpu);
//			emu.should_quit = true;
//			quit = true;
//			LeaveCriticalSection(&emu_crit);
//			ResumeThread(emulator_thread);
//			break;
//		case WAIT_TIMEOUT:
//			break;
//		case WAIT_FAILED:
//			break;
//		default:
//			break;
//
//		}
//
//		switch (WaitForSingleObject(emu_draw_event, 0)) {
//		case WAIT_OBJECT_0: {
//			EnterCriticalSection(&emu_crit);
//			SDL_Event e;
//			while (SDL_PollEvent(&e)) {
//			ImGui_ImplSDL3_ProcessEvent(&e);
//
//				if (e.type == SDL_EVENT_QUIT || e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
//					emu.should_quit = true;
//					quit = true;
//				}
//				else {
//					update_emu_controller(&emu, get_controller_state(emu.game_controller));
//					// print_controller(emu.memory->controller);
//				}
//			}
//			updateWindow(emu.gpu->screen, emu.emulator_window);
//			updateWindow(emu.gpu->tile_screen, emu.tile_window);
//
//			// draw debug window
//			draw_debug_ui(window, renderer, ig_ctx, ioptr, &emu);
//
//			LeaveCriticalSection(&emu_crit);
//			ResumeThread(emulator_thread);
//			break;
//		}
//		case WAIT_TIMEOUT:
//			break;
//		case WAIT_FAILED:
//			break;
//		}
//
//	}
//
//	SDL_DestroyWindow(window);
//	SDL_DestroyRenderer(renderer);
//
//	return 0;
//
//}

int debugger_run(args* emu_args) {
	SDL_Window* window = SDL_CreateWindow("Yolahboy Debugger", 800, 600, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
	SDL_Texture* screen_tex;
	SDL_Texture* tile_tex;
	SDL_FRect emulator_screen_rect{ 0, 0, 160, 144 };
	SDL_FRect tile_screen_rect{ 0, 0, 128, 192 };


	if (window == NULL) {
		printf("could not initialize debugger window");
		return -1;
	}

	if (renderer == NULL) {
		printf("could not initialize debugger renderer");
		SDL_DestroyWindow(window);
		return -1;
	}

	Emulator emu;
	if (init_emulator(&emu, emu_args->argv[1], emu_args->argv[2], emu_args->breakpoint_arr) < 0) {
		destroy_emulator(&emu);
		return -1;
	}
	screen_tex = SDL_CreateTextureFromSurface(renderer, emu.gpu->screen);
	tile_tex = SDL_CreateTextureFromSurface(renderer, emu.gpu->tile_screen);

	ImGuiContext* ig_ctx = NULL;
	ImGuiIO* ioptr = NULL;

	init_debug_ui(window, renderer, ig_ctx, ioptr);

	bool quit = false;
	int clocks = 0;
	bool run_once = false;
	bool set_run_once = false;
	SDL_Event e;

	while (!quit) {

		if (emu.should_run) {
			int c = step(&emu);
			if (c < 0) goto end; // if step returns negative the operation failed to execute
			clocks += c;
			set_run_once = false;
		}

		if (breakpoints[emu.cpu->registers.pc]) {
			emu.should_run = false;
			if (!set_run_once) { // logic for only running a function once in draw debug ui
				run_once = true;
				set_run_once = true;
			}
		}

		if (clocks > 29780 || !emu.should_run) {
			while (SDL_PollEvent(&e)) {
				ImGui_ImplSDL3_ProcessEvent(&e);

				switch (e.type) {
				case SDL_EVENT_QUIT:
				case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
					goto end;
				case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
				case SDL_EVENT_GAMEPAD_BUTTON_UP:
					update_emu_controller(&emu, get_controller_state(emu.game_controller)); // TODO make this logic less ugly. I think I should store SDL_Gamepad here instead of in emulator
					break;
				}
			}
			if (!emu.should_run) {
				draw_debug_ui(window, renderer, ig_ctx, ioptr, &emu, &emulator_screen_rect, &tile_screen_rect, run_once);
				run_once = false;
			}
			if (clocks > 29780) {
				SDL_DestroyTexture(screen_tex);
				screen_tex = SDL_CreateTextureFromSurface(renderer, emu.gpu->screen);
				SDL_SetTextureScaleMode(screen_tex, SDL_SCALEMODE_NEAREST);
				SDL_DestroyTexture(tile_tex);
				tile_tex = SDL_CreateTextureFromSurface(renderer, emu.gpu->tile_screen);
				SDL_SetTextureScaleMode(tile_tex, SDL_SCALEMODE_NEAREST);
				clocks = 0;
			}
			SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
			SDL_RenderClear(renderer);
			ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
			SDL_RenderTexture(renderer, screen_tex, nullptr, &emulator_screen_rect);
			SDL_RenderTexture(renderer, tile_tex, nullptr, &tile_screen_rect);
			SDL_RenderPresent(renderer);
		}
	}
end:
	destroy_emulator(&emu);
	destroy_debug_ui(window, renderer, ig_ctx, ioptr);
	return 0;
}

