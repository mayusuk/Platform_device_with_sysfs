
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "platform_device.h"

#define CLASS_NAME "HCSR"

static void hcsrdevice_release(struct device *dev)
 {

 }

static struct HCSRChipdevice hcsr_device0 = {
		.name	= DEVICE_NAME1,
		.dev_n 	= 1,
		.dev = {
			.name	= DEVICE_NAME1,
			.id	= -1,
			.dev = {.release = hcsrdevice_release,}
		}
};


static struct HCSRChipdevice hcsr_device1 = {
		.name	= DEVICE_NAME2,
		.dev_n 	= 2,
		.dev = {
			.name	= DEVICE_NAME2,
			.id	= -1,
			.dev = {.release = hcsrdevice_release,}
		}
};

static int platform_device_init(void)
{
	int result = 0;

	platform_device_register(&hcsr_device0.dev);

	printk(KERN_INFO "Platform device 1 is registered in init \n");

	//platform_device_register(&hcsr_device1.dev);

	printk(KERN_INFO "Platform device 2 is registered in init \n");

	return result;
}

static void platform_device_exit(void)
{
    	platform_device_unregister(&hcsr_device0.dev);

	//platform_device_unregister(&hcsr_device1.dev);

	printk(KERN_INFO "Unregistering the platform device\n");
}

module_init(platform_device_init);
module_exit(platform_device_exit);
MODULE_LICENSE("GPL");
