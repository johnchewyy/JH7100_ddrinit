

#include "dram_puf.h"

void bit_pattern_tuning() {


  /* Configuration */
  uint32_t size = 0x400; //4kb
  uint32_t value[5] = {0xFFFFFFFF, 0x00000000, 0x5A5A5A5A, 0xFFFF0000, 0xF0F0F0F0};
  uint32_t start_trcd = 15; //0-14

  /* Result */
  uint32_t result[5][start_trcd];

  /* init the result to 0 */
  for (int i = 0; i < 5; i++) {
      for (int j = 0; j < start_trcd; j++) {
          result[i][j] = 0;
      }
  }

  /* find number of words different */
  for(uint16_t j = 0; j < 5; j++) {

    int trcd_reduced;
    for(trcd_reduced = 0; trcd_reduced < start_trcd; trcd_reduced++) {

      //write to ddr
      for(int i = 0; i < size; i++)
      {
          writel(value[j], 0x1000000000 + i *4); //*4 because write in 32-bit 4 byte ff_ff_ff_ff
      }

      ChangeTrcd(trcd_reduced);
      udelay(300);

      // Read error addr from ddr
      uint32_t tmp;
      uint32_t count = 0;
      for(int k = 0; k < size; k++)
      {
          tmp = readl(0x1000000000 + k *4);
          udelay(15);
          if (tmp != value[j]) {
            //printk("Address: %x| read:%x | default: %x | trcd: %d| count: %d\r\n", (0x1000000000 + k *4), tmp, value[j], trcd_reduced, count);
            count++;
          }
      }

      result[j][trcd_reduced] = count;

      //printk("%x: %d | %d\r\n", value[j], trcd_reduced, count);

      ChangeTrcd(TRCD_DEFAULT);
      udelay(300);
    }
  }

  /*Display result*/
  for (uint16_t i = 0; i < 5; i++) {

    printk("%x: ", value[i]);

    for (int j = 0; j < start_trcd ; j++) {
        printk("%d", result[i][j]);

        if (j < start_trcd-1) {
            printk(", ");
        }
    }
    printk(" \r\n");
  }
}
