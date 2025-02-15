

#include "dram_puf.h"


// Function to count set bits in an integer
static uint32_t countSetBits(uint32_t n) {
    uint32_t count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}

// Function to find the difference between two hex values
uint32_t getBitDifference(uint32_t hex1, uint32_t hex2) {
    if (hex1 == hex2) {
        return 0; // Values are identical, no differing bits
    }
    return countSetBits(hex1 ^ hex2);
}

/* Function algorithm:

  1. Loop n times at the same memory to get the entrophy from a memory region in dram
  2. Based on all the capture result from the same memory region, get number of bit difference between each iterations
  3. If similarity between bit input pattern and word is lower than thershold then skip
  4. If difference between each block word if higher than thershold then skip

  Purpose:
    - Find the word at dram that have enough differences than input pattern and minimum differences within each iteration

  Assumption:
    - Assume temperature remain constant

 */
void bit_selection() {

    /*configuration*/
    uint32_t size = 0x400; //4kb
    uint32_t trialNum = 10; //Cache size 2MB maximum is 511 
    uint32_t maxBlkDifference = 2; //bits (maximum bit difference in the same DRAM address)
    uint32_t minDefaultDiff = 10; //bits (minimum bit difference between read value and default value)

    /* fixed value according to bit_pattern_tuning and trcd_tuning */
    uint32_t value = 0x5A5A5A5A;
    uint32_t Trcd = 10; //default = 20, okay = 18, 15, 13, 12 


    uint32_t count;
    uint32_t tmp;
    uint32_t blk;

    for (blk = 0; blk < trialNum; blk++) {

        //printk("blk number = 0x%x\n", blk);

        //write to ddr
        for(int i = 0; i < size; i++)
        {
            writel(value, 0x1000000000 + i *4); //*4 because write in 32-bit 4 byte ff_ff_ff_ff
        }
        
        ChangeTrcd(Trcd);

        // Read error addr from ddr and write to CACHE
        for(int j = 0; j < size; j++)
        {
            tmp = readl(0x1000000000 + j *4);
            if (tmp != value) {
            //     printk("error addr %d = 0x%x\r\n", j *4, tmp);
                writel(tmp, (SRAM_ADD + (blk*size))  + j *4); //copy to cache
            }
        }

        ChangeTrcd(TRCD_DEFAULT); //Recover Trcd
        
        //printk("\n*********************************************************\n");
    }

    //From cache, get all the similar string
    int difference;
    uint32_t ref = 0;
    uint32_t ref2 = 0;
    uint32_t same = true;

    for(int i = 0; i < size; i++) //loop the first blk from start to finish
    {
        same = true;
        ref = readl(SRAM_ADD + i *4);

        difference = getBitDifference(ref, value);
        if (difference <= minDefaultDiff) { //if too less difference
            continue;
        }

        for(int j = 1; j < trialNum; j++) { //check the same address with other blk
           ref2 = readl((SRAM_ADD + (j*size))  + i *4);

            difference = getBitDifference(ref2, value);
            if (difference <= minDefaultDiff) { //if too less difference
                same = false; 
                break;
            }

           difference = getBitDifference(ref, ref2);
           if (difference > maxBlkDifference) { //if there is difference or too much
                //printk("addr %x: difference: %d\n", i *4, difference);
                same = false; 
                break;
           }
        }
        if (same == true) {
            printk("same addr %d = 0x%x\r\n", i *4, ref);
        }
    }

}

