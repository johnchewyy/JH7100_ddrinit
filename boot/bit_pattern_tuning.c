

#include "dram_puf.h"


/* Function algorithm:

  1. Loop via every bit pattern define in the array
  2. For each pattern, reduce the Trcd value from 14 -> 0
  3. During each loop, add the number of word and bit flip if there is any
  4. Display the number of word and bit flip for each trcd value

  Purpose:
    - find the bit pattern that have the most bit flip at any given Trcd value

  Assumption:
    - temperature remain constant at every iterations

 */
void bit_pattern_tuning() {


  /* Configuration */
  uint32_t size = 0x400; //4kb
  uint32_t value[5] = {0xFFFFFFFF, 0x00000000, 0x5A5A5A5A, 0xFFFF0000, 0xF0F0F0F0};
  uint32_t start_trcd = 15; //0-14

  /* Result */
  uint32_t result_word[5][start_trcd];
  uint32_t result_bit[5][start_trcd];

  /* init the result to 0 */
  for (int i = 0; i < 5; i++) {
      for (int j = 0; j < start_trcd; j++) {
          result_word[i][j] = 0;
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
      uint32_t count_word = 0;
      uint32_t count_bit = 0;
      for(int k = 0; k < size; k++)
      {
          tmp = readl(0x1000000000 + k *4);
          udelay(15);
          if (tmp != value[j]) {
            //printk("Address: %x| read:%x | default: %x | trcd: %d| count_word: %d\r\n", (0x1000000000 + k *4), tmp, value[j], trcd_reduced, count);
            count_word++;
            count_bit = count_bit + getBitDifference(tmp, value[j]);
          }
      }

      result_word[j][trcd_reduced] = count_word;
      result_bit[j][trcd_reduced] = count_bit;

      //printk("%x: %d | %d\r\n", value[j], trcd_reduced, count_word);

      ChangeTrcd(TRCD_DEFAULT);
      udelay(300);
    }
  }

  /* Display the number of word that have at least 1 bit flip for each Trcd value */
  printk("Word count difference: \r\n");
  for (uint16_t i = 0; i < 5; i++) {

    printk("%x: ", value[i]);

    for (int j = 0; j < start_trcd ; j++) {
        printk("%d", result_word[i][j]);

        if (j < start_trcd-1) {
            printk(", ");
        }
    }
    printk(" \r\n\n");
  }

  /* Display the number of bit flips for each Trcd value */
  printk("Bit count difference: \r\n");
  for (uint16_t i = 0; i < 5; i++) {

    printk("%x: ", value[i]);

    for (int j = 0; j < start_trcd ; j++) {
        printk("%d", result_bit[i][j]);

        if (j < start_trcd-1) {
            printk(", ");
        }
    }
    printk(" \r\n\n");
  }
}
