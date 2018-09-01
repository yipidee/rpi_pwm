#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#define PERI_BASE 0x20000000  //peripheral address offset in physical memory
#define GPIO_BASE (PERI_BASE + 0x200000)  //gpio address offset in physical memory

#define BLOCK_SIZE (4*1024)  //standard memory block on RPI

struct bcm2835_peripheral {
	unsigned long addr_p;	//the physical memory offset (address of start of GPIO)
	int mem_fd;		//file desciptor for /dev/mem/
	void *map;		//the pointer to memory provided by OS (mmap)
	volatile unsigned int *addr; //pointer to memory for GPIO access
};

#define GPIO_IN(p) (*(gpio.addr + (p) / 10) &= ~(7 << (((p) % 10) * 3)))
#define GPIO_OUT(p) GPIO_IN(p); (*(gpio.addr + (p) / 10) |= (1 << (((p) % 10) * 3)))
#define GPIO_ON(p) (*(gpio.addr + 7) = (1 << (p)))
#define GPIO_CLEAR(p) *(gpio.addr + 10) = (1 << (p))
#define GPIO_READ(p) ((*(gpio.addr + 13) &= (1 << (p))) >> (p))

int main()
{
	struct bcm2835_peripheral gpio = {GPIO_BASE};

	//open /dev/mem
	if((gpio.mem_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0){
		printf("Failed to open /dev/mem\n");
		return -1;
	}

	//map physical memory to process space
	gpio.map = mmap(
			NULL,
			BLOCK_SIZE,
			PROT_READ|PROT_WRITE,
			MAP_SHARED,
			gpio.mem_fd,
			gpio.addr_p
		       );

	if(gpio.map == MAP_FAILED) {
		printf("mapping failed\n");
		return -1;
	}

	//assign mapped address to gpio access point
	gpio.addr = (volatile unsigned int *)gpio.map;

	//pin to use for this test
	int pin = 18;

	//set pin as output
	GPIO_OUT(pin);

	int c;
	for(c = 0; c < 5; ++c)
	{
		//read value of pin and print for info
		int current_level = GPIO_READ(pin);
		printf("current level of pin %i is %i\n", pin, current_level);
		
		//turn pin on
		GPIO_ON(pin);

		//read value of pin and print for info
		current_level = GPIO_READ(pin);
		printf("current level of pin %i is %i\n", pin, current_level);

		sleep(1);

		//turn pin off
		GPIO_CLEAR(pin);

		//read value of pin and print for info
		current_level = GPIO_READ(pin);
		printf("current level of pin %i is %i\n", pin, current_level);

		sleep(1);
	}

	munmap(gpio.map, BLOCK_SIZE);
	close(gpio.mem_fd);
	return 0;
}

