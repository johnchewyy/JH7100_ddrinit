
#include "dram_puf.h"

/* Function algorithm:

  1. Loop via every bit pattern define in the array
  2. For each pattern, reduce the Trcd value from 14 -> 0
  3. Stop the loop once the first bit flip is found
  4. Store the Trcd value into result buffer
  5. Display the highest Trcd value when the first bit flip occur

  Purpose:
    - find the highest value of Trcd value for every bit pattern

  Assumption:
    - temperature remain constant at every iterations

 */
void trcd_tuning() {

  /* Configuration */
  uint32_t size = 0x400; //4kb
  uint32_t value[5] = {0xFFFFFFFF, 0x00000000, 0X5A5A5A5A, 0XFFFF0000, 0XF0F0F0F0};

  /* Result */
  uint32_t result[5];

  for(uint16_t j = 0; j < 5; j++) { //number of bit pattern = 5

    uint16_t trcd_reduced;
    uint16_t done_flg = false;
    for(trcd_reduced = TRCD_DEFAULT; trcd_reduced > 0; trcd_reduced--) {

      //write to ddr
      for(int i = 0; i < size; i++)
      {
          writel(value[j], 0x1000000000 + i *4); //*4 because write in 32-bit 4 byte ff_ff_ff_ff
      }

      ChangeTrcd(trcd_reduced);
      udelay(300);

      // Read error addr from ddr
      uint32_t tmp;
      for(int k = 0; k < size; k++)
      {
          tmp = readl(0x1000000000 + k *4);
          if (tmp != value[j]) {
              printk("Address: %x| read:%x | default: %x\r\n", (0x1000000000 + k *4), tmp, value[j]);
              done_flg = true;
              break;
          }
      }

      ChangeTrcd(TRCD_DEFAULT);
      udelay(300);

      if (done_flg == true)
        break;
    }

    result[j] = trcd_reduced;
  }

  /* display result */
  for(int i = 0; i < 5; i++)
  {
    printk("bit pattern: %x | Trcd: %d\r\n", value[i], result[i]);
  }

}
