#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/list.h>
//#include <sys/ioctl.h>
#include <linux/semaphore.h>

#include <linux/kernel.h>    // kernel stuff
#include <linux/gpio.h>      // GPIO functions/macros
#include <linux/interrupt.h> // interrupt functions/macros
#include <linux/time.h>
#include <linux/unistd.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/spinlock.h>



#define MAX_BUFF_SIZE 5



#define GPIO_INT_NAME "gpio_int"


#define CLASS_NAME "HCSR"
#define DRIVER_NAME "HCSR_DRV"
#define DEVICE_NAME1 "HCSRdev1"
#define DEVICE_NAME2 "HCSRdev2"


#ifndef __SAMPLE_PLATFORM_H__

#define __SAMPLE_PLATFORM_H__



struct HCSRdevice {

		char 			*name;
		int			dev_n;
		struct platform_device 	dev;

};


struct pins {
	int echo_pin;
	int trigger_pin;
	int gpio_echo_pin;
	int gpio_trigger_pin;
};

struct parameters {
	int number_of_samples;
	int delta;
};

#endif /* __GPIO_FUNC_H__ */

