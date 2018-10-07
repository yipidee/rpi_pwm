#ifndef BCM2835_PERI_H
#define BCM2835_PERI_H

#define PERI_BASE 0x20000000  //peripheral address offset in physical memory
#define GPIO_BASE (PERI_BASE + 0x200000)  //gpio address in physical memory
#define PWM_BASE (PERI_BASE + 0x20C000)  //HW PWM address in physical memory
#define CLK_BASE (PERI_BASE + 0x101000)  //CLK register address in physical memory

#define BLOCK_SIZE (4*1024)  //standard memory block on RPI

struct bcm2835_peripheral {
	unsigned long addr_p;	//the physical memory offset (address of start of GPIO)
	int mem_fd;		//file desciptor for /dev/mem/
	void *map;		//the pointer to memory provided by OS (mmap)
	volatile unsigned int *addr; //pointer to memory for GPIO access
};

extern struct bcm2835_peripheral gpio;
extern struct bcm2835_peripheral pwm0;
extern struct bcm2835_peripheral clk;

int init_peripheral(struct bcm2835_peripheral *p);

/**************************************************************************************
 * GPIO: Access to RPi's GPIO pins
 * **********************************************************************************/

#define GPIO_IN(p) (*(gpio.addr + (p) / 10) &= ~(7 << (((p) % 10) * 3)))
#define GPIO_OUT(p) GPIO_IN((p)); (*(gpio.addr + (p) / 10) |= (1 << (((p) % 10) * 3)))
#define GPIO_ON(p) (*(gpio.addr + 7) = (1 << (p)))
#define GPIO_CLEAR(p) *(gpio.addr + 10) = (1 << (p))
#define GPIO_READ(p) ((*(gpio.addr + 13) &= (1 << (p))) >> (p))
#define GPIO_ALT(p,a) GPIO_IN((p)); (*(gpio.addr + (p) / 10) |= ((a) << (((p) % 10) * 3)))

/**************************************************************************************
 * PWM0: Access to RPi's onboard HW PWM
 * **********************************************************************************/

//PWM registers
#define PWM_CTL *(pwm0.addr + 0)
#define PWM_STA *(pwm0.addr + 1)
#define PWM_DMAC *(pwm0.addr + 2)
#define PWM_RNG1 *(pwm0.addr + 4)
#define PWM_DAT1 *(pwm0.addr + 5)
#define PWM_FIF1 *(pwm0.addr + 6)
/*
 * pwm1 not exposed on model B header, ignoring
 *
#define PWM_RNG2 *(pwm0.addr + 8)
#define PWM_DAT2 *(pwm0.addr + 9)
*/

//PWM CLK
#define PWM_CLKCTL *(clk.addr + 40)
#define PWM_CLKDIV *(clk.addr + 41)
#define PWM_CLKPW (0x5a << 24)
#define PWM_CLKSRC 1

#define PWM_CLK_START() (PWM_CLKCTL |=  (1 << 4))
#define PWM_CLK_STOP() (PWM_CLKCTL &= ~(1 << 4))
#define PWM_CLK_BUSY() ((PWM_CLKCTL & (1 << 7)) >> 7)

// CTL register bits
/*
 * pwm1 not exposed on model B header, ignoring
 *
#define MSEN2 (1 << 15)
#define USEF2 (1 << 13)
#define POLA2 (1 << 12)
#define SBIT2 (1 << 11)
#define RPTL2 (1 << 10)
#define MODE2 (1 << 9)
#define PWEN2 (1 << 8)
*/
#define MSEN1 (1 << 7)
#define CLRF1 (1 << 6)
#define USEF1 (1 << 5)
#define POLA1 (1 << 4)
#define SBIT1 (1 << 3)
#define RPTL1 (1 << 2)
#define MODE1 (1 << 1)
#define PWEN1 (1 << 0)

// FIFO status
#define PWM_FIFO_FULL (PWM_STA & 1)
#define PWM_FIFO_EMPTY ((PWM_STA & (1 << 1)) >> 1)
#define PWM_FIFO_CLEAR() (PWM_CTL |= CLRF1)

int Peripheral_init(struct bcm2835_peripheral *p);
int Peripheral_close(struct bcm2835_peripheral *p);

#endif
