#include <stdio.h>
#include <stdint.h>
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
	delay_time.tv_nsec = (long) ((ms % 1000) * 1000 * 1000);
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

int PWM_setRange(int r)
{
	int res = 0;
	
	PWM_RNG1 = r;

	return res;
}

int PWM_setFrequency(int f)
{
	int res = 0;

	//want to setup PWM with frequency f Hz
	//base clock frequency is 19.2MHz
	//which can be divided using PWM_CLKDIV * PWM_RNG
	
	uint32_t currRange = PWM_RNG1;
	int CLK_DIV = 19200000 / currRange / f;
	CLK_DIV &= 0xFFF;
	printf("divider %d\n", CLK_DIV);

	// Stop clock
	PWM_CLKCTL = (PWM_CLKPW | 0x01);
	delay(10);
	// Wait for clock to not be busy
	while((PWM_CLKCTL & 0x80) != 0) delay(10);

	// set clock divider and enable oscillator
	PWM_CLKDIV = (PWM_CLKPW | (CLK_DIV << 12));
	PWM_CLKCTL =  (PWM_CLKPW | 0x11);
	delay(10);

	return res;
}

int PWM_init(int f)
{
	int res = 0;
	
	//set hardware PWM output pin to PWM mode
	GPIO_ALT(PIN_PWM, ALT5);
	delay(10);

	//set resolution, currently defaults to this value
	PWM_setRange(0x400);
	PWM_setFrequency(f);

	unsigned int control = PWM_CTL;
	control |= (1<<7);  //don't use mark-space mode
	control |= 1; //enable pwm0
	PWM_CTL = control;
	
	return res;
}

int PWM_setDuty(int d)
{
	int res = 0;

	uint32_t range = PWM_RNG1;
	d = (range * d) / 100;

	PWM_DAT1 = d;

	return res;
}

int main()
{
	// Initialize the bcm2835 peripherals to be used
	Peripheral_init(&gpio);
	Peripheral_init(&pwm0);
	Peripheral_init(&clk);

	PWM_init(38500);
	PWM_setDuty(50);

	/*
	for(int a=1; a<=100000; ++a)
	{
		PWM_setFrequency(a);
		delay(100);
	}
	*/
	delay(5000);

	PWM_CLKCTL = (PWM_CLKPW | 0x01);

	// Close the bcm2835 periperals
	Peripheral_close(&gpio);
	Peripheral_close(&pwm0);
	Peripheral_close(&clk);

	return 0;
}
