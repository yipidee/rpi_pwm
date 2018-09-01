#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "bcm2835_peri.h"

struct bcm2835_peripheral gpio = {GPIO_BASE};
struct bcm2835_peripheral pwm0 = {PWM_BASE};
struct bcm2835_peripheral clk_pwm = {CLK_BASE};

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

int PWM_init()
{
	int error_count = 0;
	while(1)
	{
		if (error_count > 100)
		{
			printf("error count out\n");
			return -1;
		}
		if(PWM_CLK_BUSY)
		{
			++error_count;
		}else
		{
			PWM_CLKCTL = (PWM_CLKPW | PWM_CLKSRC);
			usleep(10);
			PWM_CLKDIV = (PWM_CLKPW | (0x1F9 << 12) | 0xA48);
			usleep(10);
			//PWM_DMAC = ((1<<31) | (7 << 8) | 7);
			PWM_RNG1 = 0x400;
			usleep(10);
			PWM_DAT1 = 0x400;
			usleep(10);
			break;
		}
	}
	return 0;
}

int main()
{
	Peripheral_init(&gpio);
	Peripheral_init(&pwm0);
	Peripheral_init(&clk_pwm);

	PWM_CTL = 0;
	usleep(10);
	PWM_INIT();
	usleep(10);

	PWM_CLKCTL = (PWM_CLKPW | PWM_CLKSRC);
	usleep(10);
	PWM_CLKDIV = (PWM_CLKPW | (0x1F9 << 12) | 0xA48);
	usleep(10);
	PWM_RNG1 = 0x400;
	usleep(10);
	PWM_CLK_START();
	while(1)
	{
		printf("running\n");
		PWM_DAT1 = 0x400;
		usleep(10);
	}


/*
	PWM_init();
	usleep(10);
	PWM_CLK_START();
	PWM_CLK_STOP();
*/
	Peripheral_close(&gpio);
	Peripheral_close(&pwm0);
	Peripheral_close(&clk_pwm);

	return 0;
}
