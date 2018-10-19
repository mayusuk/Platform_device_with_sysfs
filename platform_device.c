
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "platform_device.h"


#define CLASS_NAME "HCSR"
#define DRIVER_NAME "PLT_HCSR"

struct hcsr_dev {
   	chip_hcsrdevice plat_device;
    	struct miscdevice misc_device;
	struct pins pin;
	struct parameters param;
	struct buff buffer;
	struct list_head device_entry;
	struct task_struct *task;
	char *dev_name;
	bool is_in_progress;
	int enable;
};

static void hcsrdevice_release(struct device *dev)
 {

 }

static struct HCSRdevice hcsr_device0 = {
		.name	= DEVICE_NAME1,
		.dev_no 	= 1,
		.plf_dev = {
			.name	= DEVICE_NAME1,
			.id	= -1,
			.dev = {.release = hcsrdevice_release,}
		}
};

static struct HCSRdevice hcsr_device1 = {
		.name	= DEVICE_NAME2,
		.dev_no 	= 2,
		.plf_dev = {
			.name	= DEVICE_NAME2,
			.id	= -1,
			.dev = {.release = hcsrdevice_release,}
		}
};


static int p_device_init(void)
{
	int ret = 0;

	platform_device_register(&hcsr_device0.dev);

	printk(KERN_ALERT "Platform device 1 is registered in init \n");

	platform_device_register(&hcsr_device1.dev);

	printk(KERN_ALERT "Platform device 2 is registered in init \n");

	return ret;
}

static void p_device_exit(void)
{
    platform_device_unregister(&hcsr_device0.dev);

	platform_device_unregister(&hcsr_device1.dev);

	printk(KERN_ALERT "Goodbye, unregister the device\n");
}

module_init(p_device_init);
module_exit(p_device_exit);
MODULE_LICENSE("GPL");
