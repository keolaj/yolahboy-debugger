#include "../global_definitions.h"

Timer* create_timer();
void tick(Emulator* emu, int m_cycles);
void reset_clock(Timer* timer);