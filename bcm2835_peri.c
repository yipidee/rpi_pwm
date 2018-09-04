#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <time.h>
#include "bcm2835_peri.h"

#define PIN_PWM 18
#define ALT5 2

struct bcm2835_peripheral gpio = {GPIO_BASE};
struct bcm2835_peripheral pwm0 = {PWM_BASE};
struct bcm2835_peripheral clk = {CLK_BASE};

void delay(int ms)
{
	struct timespec delay_time;
	delay_time.tv_sec = (time_t) ms / 1000;
	delay_time.tv_nsec = (long) ((ms % 1000) * 1000);
	nanosleep(&delay_time, NULL);
}

int Peripheral_init(struct bcm2835_peripheral *p)
{
	//open /dev/mem
	if((p->mem_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0){
		printf("Failed to open /dev/mem\n");
		return -1;
	}

	//map physical memory to process space
	p->map = mmap(
			NULL,
			BLOCK_SIZE,
			PROT_READ|PROT_WRITE,
			MAP_SHARED,
			p->mem_fd,
			p->addr_p
		       );

	if(p->map == MAP_FAILED) {
		printf("mapping failed\n");
		return -1;
	}

	//assign mapped address to gpio access point
	p->addr = (volatile unsigned int *)p->map;
	
	return 0;
}

int Peripheral_close(struct bcm2835_peripheral *p)
{
	munmap(p->map, BLOCK_SIZE);
	close(p->mem_fd);
	p = NULL;

	return 0;
}

int main()
{
	// Initialize the bcm2835 peripherals to be used
	Peripheral_init(&gpio);
	Peripheral_init(&pwm0);
	Peripheral_init(&clk);

	/**********************************************
	 * Steps required
	 * 1. setup GPIO pins for PWM
	 * 2. setup PWM clock
	 * 3. setup PWM
	 *********************************************/

	// Step 1
	GPIO_OUT(PIN_PWM);
	GPIO_ALT(PIN_PWM, ALT5);

	// Step 2
	// Stop clock
	PWM_CLKCTL = (PWM_CLKPW | 0x10);
	delay(100);
	// Wait for clock to not be busy
	while((PWM_CLKCTL & 0x80) != 0) delay(0);
	// set clock divider and enable oscillator
	PWM_CLKDIV = (PWM_CLKPW | (0x2 << 12));
	PWM_CLKCTL =  (PWM_CLKPW | 0x11);

	// Step 3
	

	// Close the bcm2835 periperals
	Peripheral_close(&gpio);
	Peripheral_close(&pwm0);
	Peripheral_close(&clk);

	return 0;
}
