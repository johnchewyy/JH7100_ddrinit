

#include "sys.h"
#include "spi_flash.h"
#include "spi.h"
#include "timer.h"
#include "encoding.h"
#include "gpt.h"
#include "clkgen_ctrl_macro.h"
#include "syscon_sysmain_ctrl_macro.h"
#include "rstgen_ctrl_macro.h"
#include "ddr_puf.h"
#include "crc32.h"
#include "ddr_puf.h"

extern void ddr_puf();

void ddr_puf()
{
  int fail_flag = 0;
  uint32_t tmp;
  uint32_t OMC_APB_BASE_ADDR, OMC_SECURE_APB_BASE_ADDR, PHY_APB_BASE_ADDR;
  uint64_t mem_addr;
  uint32_t wdata;
  int i = 0, j = 0;
  u32 count = 0;
  
  uint64_t base_addr = U74_SYS_PORT_DDRC_BASE_ADDR;

  _SWITCH_CLOCK_clk_dla_root_SOURCE_clk_osc_sys_;

  MA_OUTW(syscon_sysmain_ctrl_SCFG_pll1_REG_ADDR,0x292905);//set reset
  udelay(10); //wait(500*(1/25M))
  MA_OUTW(syscon_sysmain_ctrl_SCFG_pll1_REG_ADDR,0x0292904);//clear reset
  udelay(10); //wait(500*(1/25M))


  _SWITCH_CLOCK_clk_dla_root_SOURCE_clk_pll1_out_;
  _SWITCH_CLOCK_clk_ddrc0_SOURCE_clk_ddrosc_div2_;
  _SWITCH_CLOCK_clk_ddrc1_SOURCE_clk_ddrosc_div2_; 
  
  _ENABLE_CLOCK_clk_ddrc0_;
  _ENABLE_CLOCK_clk_ddrc1_;

  //---- config ddrphy0/omc0 ----
  for(int ddr_num=0; ddr_num<2; ddr_num++) {
    if(ddr_num == 0) {
      OMC_APB_BASE_ADDR        = OMC_CFG0_BASE_ADDR;
      OMC_SECURE_APB_BASE_ADDR = OMC_CFG0_BASE_ADDR+0x1000;
      PHY_APB_BASE_ADDR        = DDRPHY0_CSR_BASE_ADDR;
    } else {
      OMC_APB_BASE_ADDR        = OMC_CFG1_BASE_ADDR;
      OMC_SECURE_APB_BASE_ADDR = OMC_CFG1_BASE_ADDR+0x1000;
      PHY_APB_BASE_ADDR        = DDRPHY1_CSR_BASE_ADDR;
    }

    regconfig_h_sim_pi(OMC_APB_BASE_ADDR,  OMC_SECURE_APB_BASE_ADDR,  PHY_APB_BASE_ADDR);
    regconfig_h_sim_phy(OMC_APB_BASE_ADDR,  OMC_SECURE_APB_BASE_ADDR,  PHY_APB_BASE_ADDR);
    
    
    regconfig_pi_start(OMC_APB_BASE_ADDR,  OMC_SECURE_APB_BASE_ADDR,  PHY_APB_BASE_ADDR, ddr_num);
           
    if(ddr_num == 0) //ddrc_clock=12.5M
    {
      _SWITCH_CLOCK_clk_ddrc0_SOURCE_clk_ddrosc_div2_;
    }
    else
    {
      _SWITCH_CLOCK_clk_ddrc1_SOURCE_clk_ddrosc_div2_;
    }
    udelay(300);
    apb_write(PHY_APB_BASE_ADDR + (0 +0 << 2), 0x01);//release dll_rst_n
    udelay(300);
    
    orbit_boot(OMC_APB_BASE_ADDR,  OMC_SECURE_APB_BASE_ADDR,  PHY_APB_BASE_ADDR, ddr_num);
  }

  int size = 0x80000;


  printk("DRAM PUF start...\n");
  //apb_write(OMC_APB_BASE_ADDR + 0x628, 0x402d0880); //trcd = 5

  printk("DRAM PUF write 0xffffffff...\n");
  for(i = 0; i < size; i++)
  {
    writel(0xffffffff, 0x1000000000 + i *4); //*4 because write in 32-bit 4 byte ff_ff_ff_ff

    if((i% (size/2)) == 0)	
    {								
      count++;								
      printk("ddr write 0x%x, %dM test\n",(i * 4), count);						
    }
  }

  printk("DRAM PUF change Trcd...\n");
    // timing_reg->trc = 0x40;
    // timing_reg->tras = 0x2D;
    // timing_reg->trtp = 0x08;
    // timing_reg->adv_al = 0x1;

    // timing_reg->trcd = 0x00 -> 0x...80 (Trcd = 0)
    // ...
    // timing_reg->trcd = 0x0F -> 0x...8F (Trcd = 15)
    // ...
    // timing_reg->trcd = 0x1F -> 0x...9F (Trcd = 31)

  for(int ddr_num=0; ddr_num<2; ddr_num++) {
    // if(ddr_num == 0) {
    //   OMC_APB_BASE_ADDR        = OMC_CFG0_BASE_ADDR;
    //   OMC_SECURE_APB_BASE_ADDR = OMC_CFG0_BASE_ADDR+0x1000;
    //   PHY_APB_BASE_ADDR        = DDRPHY0_CSR_BASE_ADDR;
    // } else {
    //   OMC_APB_BASE_ADDR        = OMC_CFG1_BASE_ADDR;
    //   OMC_SECURE_APB_BASE_ADDR = OMC_CFG1_BASE_ADDR+0x1000;
    //   PHY_APB_BASE_ADDR        = DDRPHY1_CSR_BASE_ADDR;
    // }

    // if(ddr_num == 0) //ddrc_clock=12.5M
    // {
    //   _SWITCH_CLOCK_clk_ddrc0_SOURCE_clk_ddrosc_div2_;
    // }
    // else
    // {
    //   _SWITCH_CLOCK_clk_ddrc1_SOURCE_clk_ddrosc_div2_;
    // }
    // udelay(300);
    // apb_write(PHY_APB_BASE_ADDR + (0 +0 << 2), 0x01);//release dll_rst_n
    // udelay(300);
    
    orbit_boot2(OMC_APB_BASE_ADDR,  OMC_SECURE_APB_BASE_ADDR,  PHY_APB_BASE_ADDR, ddr_num);
  }
  printk("DRAM PUF change Trcd done\n");
  
  printk("DRAM PUF read...\n");
  for(j = 0; j < size; j++)
  {
    tmp = readl(0x1000000000 + j *4);
    if (tmp != 0xffffffff)
      printk("%x", tmp);
  }

  printk("\nDRAM PUF restore Trcd...\n");
  //apb_write(OMC_APB_BASE_ADDR + 0x628, 0x402d0894);

  printk("DRAM PUF end...\n");
//     while(1);
}