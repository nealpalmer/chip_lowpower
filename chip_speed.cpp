#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main() {
#define MAPPED_SIZE (0x200000) //place the size here
#define DDR_RAM_PHYS 0x01c00000 //place the physical address here

#define PLL1_CFG_REG (0x20000/sizeof(int))
#define PLL1_TUN_REG (0x20004/sizeof(int))
#define PLL2_CFG_REG (0x20008/sizeof(int))
#define PLL2_TUN_REG (0x2000c/sizeof(int))
#define PLL3_CFG_REG (0x20010/sizeof(int))
#define PLL4_CFG_REG (0x20018/sizeof(int))
#define PLL5_CFG_REG (0x20020/sizeof(int))
#define PLL7_CFG_REG (0x20030/sizeof(int))

#define IR_SCLK_CFG_REG (0x200B0/sizeof(int))
#define BE_CFG_REG (0x20104/sizeof(int))
#define FE_CFG_REG (0x2010c/sizeof(int))
#define LCD_CH1_CFG_REG (0x2012c/sizeof(int))
#define AUDIO_CODEC_SCLK_CFG_REG (0x20140/sizeof(int))
#define AVS_SCLK_CFG_REG (0x20144/sizeof(int))
#define MALI_CLOCK_CFG_REG (0x20154/sizeof(int))

#define CPUAHBAPB0_CLK (0x20054/sizeof(int))
#define APB1_CLK_DIV_REG (0x20058/sizeof(int))

#define TCON_GCTL_REG (0x0c000/sizeof(int))

#define DEFE_EN_REG (0x100000/sizeof(int))

#define DEBE_EN_REG (0x160000/sizeof(int))

#define AC_DAC_DPC (0x022c00/sizeof(int))
#define AC_DAC_ACTRL (0x022c10/sizeof(int))
#define AC_ADC_FIFOC (0x022c1c/sizeof(int))

#define PB_CFG0 (0x020824/sizeof(int))
#define PB_DAT (0x020834/sizeof(int))

#define PC_CFG2 (0x020850/sizeof(int))
#define PC_DAT (0x020858/sizeof(int))

int _fdmem;
int *map = NULL;
const char memDevice[] = "/dev/mem";

/* open /dev/mem and error checking */
_fdmem = open( memDevice, O_RDWR | O_SYNC );

if (_fdmem < 0){
printf("Failed to open the /dev/mem !\n");
return 0;
}
else{
printf("open /dev/mem successfully !\n");
}

/* mmap() the opened /dev/mem */
map= (int *)(mmap(0,MAPPED_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,_fdmem,DDR_RAM_PHYS));

/* use 'map' pointer to access the mapped area! */
//for (int i=0;i<MAPPED_SIZE/sizeof(int);i++)
//printf("content: mmap[0x%x]=0x%x\n",i*sizeof(int) +DDR_RAM_PHYS,*(map+i));

// Print some power statistics
printf("pll1_cfg_reg=0x%x\n",map[PLL1_CFG_REG]);
double pll1_freq = 0;
if ((map[PLL1_CFG_REG]&0x80000000)!=0) {
	int P = 1 << ((map[PLL1_CFG_REG]>>16) & 0x3);
	int N = (map[PLL1_CFG_REG]>>8) & 0x1f;
	int K = ((map[PLL1_CFG_REG]>>4) & 0x3) + 1;
	int M = ((map[PLL1_CFG_REG]>>0) & 0x3) + 1;
	printf("PLL1 enabled: %lf Mhz\n",pll1_freq = (24.0*N*K/M/P));
} else
	printf("PLL1 disabled\n");

switch ((map[CPUAHBAPB0_CLK]>>16)&0x3) {
	case 0: printf("cpu = 32Khz\n"); break;
	case 1: printf("cpu = 24Mhz\n"); break;
	case 2: printf("cpu = PLL1 = %0.0lf Mhz\n",pll1_freq); break;
	case 3: printf("cpu = 200Mhz\n"); break;
}

//dont ever use this one... map[CPUAHBAPB0_CLK] &= 0xfffcffff; // set to 32Khz for core clock (too slow for the OS to run at all!)
map[CPUAHBAPB0_CLK] = (map[CPUAHBAPB0_CLK]&0xfffcffff) | 0x10000; // set to 24Mhz for core clock
//map[CPUAHBAPB0_CLK] = (map[CPUAHBAPB0_CLK]&0xfffcffff) | 0x20000; // set to PLL1 for core clock
//map[CPUAHBAPB0_CLK] |= 0x30000; // set to 200Mhz for core clock

map[APB1_CLK_DIV_REG] = 9; // 24Mhz/10=2.4Mhz for APB1
//printf("APB1=0x%x\n",map[APB1_CLK_DIV_REG]);

//map[PLL1_CFG_REG] = 0xa1005510; // 1Ghz PLL1
map[PLL1_CFG_REG] = 0x00000000; // disable PLL1 (core clk)
map[PLL2_CFG_REG] = 0x00000000; // disable PLL2 (audio)
map[PLL3_CFG_REG] = 0x00000000; // disable PLL3 (video)
map[PLL4_CFG_REG] = 0x00000000; // disable PLL4 (video)
//map[PLL5_CFG_REG] = dont change it 0x00000000; // DRAM PLL, don't change it!
//map[PLL6_CFG_REG] = 0x00000000; // disable PLL6 (video)
map[PLL7_CFG_REG] = 0x00000000; // disable PLL7 (video)


map[CPUAHBAPB0_CLK] = (map[CPUAHBAPB0_CLK]&0xfffffffc) | 0x3; // set AXI bus a slow as possible
map[CPUAHBAPB0_CLK] = (map[CPUAHBAPB0_CLK]&0xffffffcf) | 0x30; // set AHB bus a slow as possible
//map[CPUAHBAPB0_CLK] = (map[CPUAHBAPB0_CLK]&0xffffff0f) | 0x30; // set AHB bus a slow as possible
map[CPUAHBAPB0_CLK] = (map[CPUAHBAPB0_CLK]&0xfffffcff) | 0x300; // set APB0 bus a slow as possible


/*if ((map[TCON_GCTL_REG]&0x80000000)!=0)
	printf("LCD/TV enabled\n");
else
	printf("LCD/TV disabled\n");
*/

map[TCON_GCTL_REG] &= 0x7fffffff; // disable the LCD/TV module (didn't make any power difference)
//map[TCON_GCTL_REG] |= 0x80000000; // enable the LCD/TV module (didn't make any power difference)



map[DEFE_EN_REG] = 0; // disable the DISPLAY_FRONT_END module (didn't make any power difference)
map[DEBE_EN_REG] = 0; // disable the DISPLAY_BACK_END module (didn't make any power difference)

map[AC_DAC_DPC] = 0x01000000; // disable the AUDIO module (didn't make any power difference)
map[AC_DAC_ACTRL] = 0x00000000; // disable the AUDIO module (didn't make any power difference)
map[AC_ADC_FIFOC] = 0xa0000000; // disable the AUDIO module (didn't make any power difference)

map[AUDIO_CODEC_SCLK_CFG_REG] = 0; // disable the AUDIO SCLK 
map[AVS_SCLK_CFG_REG] = 0; // disable the AVS SCLK 
map[LCD_CH1_CFG_REG] = 0; // disable the LCD CH1 SCLK 0/1
map[BE_CFG_REG] = 0; // disable the display_back_end clock
map[FE_CFG_REG] = 0; // disable the display_front_end clock
map[IR_SCLK_CFG_REG] = 0; // disable the IR clock
map[MALI_CLOCK_CFG_REG] = 0; // disable the MALI-400 clock



// DISABLE WIFI (will probably break everything, but this is just a test)
/*
map[PB_DAT] = (map[PB_DAT]&0xffffffef) | 0x00; // set PB4 to drive 0 (i.e. BT-RST-N=0)
map[PB_CFG0] = (map[PB_CFG0]&0xfff8ffff) | 0x10000; // set PB4 as an output
//map[PB_CFG0] = (map[PB_CFG0]&0xfff8ffff) | 0x00000; // set PB4 as an input
map[PB_DAT] = (map[PB_DAT]&0xffffffef) | 0x00; // set PB4 to drive 0 (i.e. BT-RST-N=0)
printf("PB_CFG0=0x%x\n",map[PB_CFG0]);
printf("PB_DAT=0x%x\n",map[PB_DAT]);

map[PC_CFG2] = (map[PC_CFG2]&0xffff8fff) | 0x1000; // set PC19 as an input
map[PC_DAT] = (map[PC_DAT]&0xfff7ffff) | 0x00; // set PC19 to drive 0 (i.e. WL-PMU-EN=0)
printf("PC_CFG2=0x%x\n",map[PC_CFG2]);
printf("PC_DAT=0x%x\n",map[PC_DAT]);
*/

// Change the DDR3 PLL's frequency
unsigned int pll5[] = {
0xb1048f91, // default 360Mhz, 24*15*2/2
0xb1048e91, // 15->14  336Mhz, 24*14*2/2
0xb1048d91, // 14->13  312Mhz, 24*13*2/2
0xb1048c91, // 13->12  288Mhz, 24*12*2/2
0xb1048b91, // 13->12  264Mhz, 24*11*2/2
//0xb1048a91, // 13->12  240Mhz, 24*10*2/2
//0xb1048991, // 13->12  216Mhz, 24*9*2/2
//0xb1048891, // 13->12  192Mhz, 24*8*2/2
//0xb1048791, // 13->12  168Mhz, 24*7*2/2 seems to crash
0
};

/*
for (int i=0;pll5[i]!=0;i++) {
	unsigned int val;
	val = pll5[i];
	//val = map[PLL5_CFG_REG];
	int P = 1 << ((val >> 16) & 3);
	int N = (val >> 8) & 0x1f;
	int K = 1 + ((val >> 4) & 3);
	int M1 = (val >> 2) & 3;
	int M = 1 + ((val >> 0) & 3);
	printf("pll5=0x%8.8x, P=%d, N=%d, K=%d, M=%d. %lfMhz\n",
		val,
		P,N,K,M,
		24.0*N*K/M);
	fflush(stdout);
	sleep(1);
	map[PLL5_CFG_REG] = val;
	sleep(1);
}
*/


/* unmap the area & error checking */
if (munmap(map,MAPPED_SIZE)==-1){
perror("Error un-mmapping the file");
}

/* close the character device */
close(_fdmem);
return(0);
}
