


#include "sys.h"
#include "spi_flash.h"
#include "spi.h"
#include "timer.h"
#include "encoding.h"
#include "gpt.h"
#include "clkgen_ctrl_macro.h"
#include "syscon_sysmain_ctrl_macro.h"
#include "rstgen_ctrl_macro.h"
#include "crc32.h"

#define SRAM_ADD 0x08000000
#define TRCD_DEFAULT 14

void bit_selection();
void ChangeTrcd(uint32_t Trcd);
void bit_pattern_tuning();
void trcd_tuning();

