#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

#define BANKSIZE 0x4000
#define TITLE 0x134
#define TITLE_SIZE 0x10
#define CARTRIDGE_TYPE 0x147

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

#define LCD_CONTROL 0xFF40 // LCD control address
#define LCD_STATUS 0xFF41 // LCD status address
#define LY 0xFF44 // Current horizontal line address
#define LYC 0xFF45 // Current horizonal line compare address
#define SCY 0xFF42 // Scroll Y address
#define SCX 0xFF43 // Scroll X address
#define WY 0xFF4A // Window Y address
#define WX 0xFF4B // Window X address
#define BGP 0xFF47 // Background Palette address

// #define IF 0xFF0F // Interrupt flag

#define BLACK 0x252b25FF
#define DARK 0x555a56FF
#define LIGHT 0x5e785dFF
#define WHITE 0x84d07dFF

#define NUM_TILES 384
#define TILES_Y 24
#define TILES_X 16
#define TILE_WIDTH 8
#define TILE_HEIGHT 8


#define MAX_BREAKPOINTS 0x100

#define IF 0xFF0F
#define IE 0xFFFF

#define DMA 0xFF46

#define OBP0 0xFF48
#define OBP1 0xFF49


#define VBLANK_INTERRUPT 1
#define VBLANK_ADDRESS 0x40

#define LCDSTAT_INTERRUPT 1 << 1
#define LCDSTAT_ADDRESS 0x48

#define TIMER_INTERRUPT 1 << 2
#define TIMER_ADDRESS 0x50

#define SERIAL_INTERRUPT 1 << 3
#define SERIAL_ADDRESS 0x58

#define JOYPAD_INTERRUPT 1 << 4
#define JOYPAD_ADDRESS 0x60

typedef unsigned char u8;
typedef char i8;
typedef short i16;
typedef unsigned short u16;
typedef unsigned int u32;

#define FLAG_ZERO 0b10000000
#define FLAG_SUB 0b01000000
#define FLAG_HALFCARRY 0b00100000
#define FLAG_CARRY 0b00010000

typedef struct {
	bool up;
	bool down;
	bool left;
	bool right;

	bool start;
	bool select;

	bool a;
	bool b;
} Controller;

typedef enum {
	ROM_ONLY,
	MBC1,
	MBC1_RAM,
	MBC1_RAM_BATTERY,
	MBC2
} CART_TYPE;

typedef enum {
	RAM_NONE,
	RAM_UNUSED,
	RAM_8KB,
	RAM_32KB,
	RAM128KB,
	RAM_64KB
} RAM_TYPE;

#define BANKMODESIMPLE false
#define BANKMODEADVANCED true

typedef struct {
	u8* rom;
	u8* ram;
	u8 type;
	u8 rom_bank;
	u8 ram_bank;
	bool banking_mode;
	bool ram_enabled;
	u8 cgb_flag;
} Cartridge;

typedef struct mem_ctx Memory;
typedef struct gpu_ctx Gpu;

typedef struct mem_ctx {
	u8 bios[0x100];
	u8 memory[0x10000];
	Cartridge cartridge;
	bool in_bios;
	bool use_gbd_log;
	Gpu* gpu;
	Controller* controller;
} Memory;

typedef struct {
	union {
		struct {
			u8 f;
			u8 a;
		};
		u16 af;
	};
	union {
		struct {
			u8 c;
			u8 b;
		};
		u16 bc;
	};
	union {
		struct {
			u8 e;
			u8 d;
		};
		u16 de;
	};
	union {
		struct {
			u8 l;
			u8 h;
		};
		u16 hl;
	};
	u16 sp;
	u16 pc;
} Registers;

typedef struct {
	Registers registers;
	bool IME;
	bool should_update_IME;
	bool update_IME_value;
	int update_IME_counter;
} Cpu;

typedef struct {
	int m_cycles;
	int t_cycles;
} Cycles;


typedef u8** Tile;

typedef enum {
	HBLANK,
	VBLANK,
	OAM_ACCESS,
	VRAM_ACCESS
} gpu_mode;

struct gpu_ctx {
	int line;
	int clock;
	u32 framebuffer[23040];

	Memory* mem;

	Tile* tiles;
	SDL_Surface* screen;
	SDL_Surface* tile_screen;

	gpu_mode mode;
	bool drawline;
};

typedef enum {
	MODE256,
	MODE4,
	MODE16,
	MODE64
} TimerMode;

typedef struct { // TODO finish this for timer emulation
	TimerMode mode;
	int count;
} Timer;

typedef struct {
	Cpu* cpu;
	Memory* memory;
	Gpu* gpu;
	Controller controller;
	int clock;	
	bool should_run;
	bool should_draw;
} Emulator;

typedef struct {
	SDL_Scancode a;
	SDL_Scancode b;
	SDL_Scancode start;
	SDL_Scancode select;
	SDL_Scancode up;
	SDL_Scancode down;
	SDL_Scancode left;
	SDL_Scancode right;
} KeyboardConfig;
