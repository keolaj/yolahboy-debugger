#pragma once

#include "components/global_definitions.h"

typedef struct {
	// HANDLE mem_pipe_handle;
	int argc;
	char** argv;
	int* breakpoint_arr;

} args;


int run_emulator(LPVOID t_args);