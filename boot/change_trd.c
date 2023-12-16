
#include "dram_puf.h"

void ChangeTrcd(uint32_t Trcd)
{
    apb_write(OMC_CFG0_BASE_ADDR + 0x330, 0x00000000);
    apb_write(OMC_CFG0_BASE_ADDR + 0x628, 0x402d0880 + Trcd);
    apb_write(OMC_CFG0_BASE_ADDR + 0x330, 0x09313fff);
    apb_write(OMC_CFG1_BASE_ADDR + 0x330, 0x00000000);
    apb_write(OMC_CFG1_BASE_ADDR + 0x628, 0x402d0880 + Trcd);
    apb_write(OMC_CFG1_BASE_ADDR + 0x330, 0x09313fff);
}
