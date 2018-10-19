#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include "platform_device.h"
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/device.h>
#include "gpio_ioctl.h"
#include <linux/errno.h>



int errno;
unsigned long long t1,t2;
short int irq_any_gpio    = 0;
int count =0;
uint64_t tsc1=0, tsc2=0;


static struct device *gko_device;
static struct class *gko_class;
static dev_t gko_dev= 1;
static int base_minor_num;


static int first = 1;
static const struct platform_device_id P_id_table[] = {
         { DEVICE_NAME1, 0 },
         { DEVICE_NAME2, 0 },
};




static LIST_HEAD(device_list);



unsigned long long get_tsc(void){
   unsigned a, d;

   __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));

   return ((unsigned long long)a) | (((unsigned long long)d) << 32);
}


static struct hcsr_dev *device_struct;
struct buff *all_samples;
unsigned long long  tsc_raising, tsc_falling;
static irq_handler_t interrupt_handler(unsigned int irq, void *dev_id) {

    printk(KERN_INFO"INTERRUPT HANDLER\n");
	struct hcsr_dev *hcsr_obj = (struct hcsr_dev*)dev_id;
	int distance;
	struct 	data measurement;
	int check = gpio_get_value(hcsr_obj->pin.gpio_echo_pin);

	if (irq == gpio_to_irq(hcsr_obj->pin.echo_pin))
	{
		if (check ==0)
		{
            printk("gpio pin is low\n");
	 		tsc_falling=get_tsc();
			distance =  ((int)(tsc_falling - tsc_raising)/(139200));
			measurement.timestamp = tsc_falling;
			measurement.distance = distance;
			printk(KERN_INFO "***Distance *** %d",distance);
			insert_buffer(all_samples, measurement, &all_samples->sem, &all_samples->Lock);
			irq_set_irq_type(irq, IRQ_TYPE_EDGE_RISING);
  		}
		else
		{
			printk("gpio pin is high\n");
			tsc_raising = get_tsc();
			irq_set_irq_type(irq, IRQ_TYPE_EDGE_FALLING);
		}
	}

   return (irq_handler_t) IRQ_HANDLED;
}


int set_interrupt(void *device_struct){

	struct hcsr_dev *hcsr_obj = (struct hcsr_dev*)device_struct;
    int irqNumber, result;
    irqNumber = gpio_to_irq(hcsr_obj->pin.gpio_echo_pin);
	if(irqNumber < 0){
		printk("GPIO to IRQ mapping failure \n" );
      		return -1;
	}
    printk(KERN_INFO "IRQNUMBER is %d", irqNumber);

	result = request_irq(irqNumber,               	// The interrupt number requested
		 (irq_handler_t) interrupt_handler, 	    // The pointer to the handler function (above)
		 IRQF_TRIGGER_RISING,                 		// Interrupt is on rising edge
		 "ebb_gpio_handler",                  		// Used in /proc/interrupts to identify the owner
		 (void*)device_struct);                     // The *dev_id for shared interrupt lines, NULL here

    if(result < 0){
         printk("Irq Request failure\n");
		return -1;
    }
    return 0;
}

int release_irq(void* handle)
{
	struct hcsr_dev *hcsr_obj = (struct hcsr_dev*)handle;
	printk(KERN_INFO "Releasing the interrupt %d on pin %d\n", gpio_to_irq(hcsr_obj->pin.gpio_echo_pin), hcsr_obj->pin.gpio_echo_pin);
	free_irq(gpio_to_irq(hcsr_obj->pin.gpio_echo_pin), handle );
	return 0;
}


int set_pin_conf(int pin, bool func){

	struct conf *config = NULL;
	config = (struct conf*)get_pin(pin,func);
	if(config == NULL){
		return -EINVAL;
	}

	if(func == INPUT_FUNC){
        printk(KERN_INFO "Setting gpio %d as input\n", config->gpio_pin.pin);
        gpio_request(config->gpio_pin.pin, "Input");
		gpio_direction_input(config->gpio_pin.pin);
	}
	if(config->direction_pin.pin != DONT_CARE){
        printk(KERN_INFO "Setting direction gpio  %d\n", config->direction_pin.pin);
        gpio_request(config->direction_pin.pin, "Direction");
		gpio_direction_output(config->direction_pin.pin, config->direction_pin.level);
	}
	if(config->function_pin_1.pin != DONT_CARE){
        printk(KERN_INFO "Setting function 1 gpio  %d\n", config->function_pin_1.pin);
        gpio_request(config->function_pin_1.pin, "function_1");
		gpio_direction_output(config->function_pin_1.pin, config->function_pin_1.level);
	}
	if(config->function_pin_2.pin != DONT_CARE){
        printk(KERN_INFO "Setting function 1 gpio  %d\n", config->function_pin_2.pin);
        gpio_request(config->function_pin_2.pin, "function_2");
		gpio_direction_output(config->function_pin_2.pin, config->function_pin_2.level);
	}

	return 	config->gpio_pin.pin;

}




static ssize_t show_echo_pin(struct device *dev_struct, struct device_attribute *attr, char *buf)
{
	struct hcsr_dev *device_struct  = dev_get_drvdata(dev_struct);
        return snprintf(buf, PAGE_SIZE, "%d\n", device_struct->pin.echo_pin);
}


static ssize_t show_distance(struct device *dev_struct, struct device_attribute *attr, char *buf)
{
	
       struct hcsr_dev *device_struct  = dev_get_drvdata(dev_struct);

	printk("distance = %d\n", pshcsr_dev_obj->Pring_buf->ptr[pshcsr_dev_obj->Pring_buf->tail].TSC);
	 return snprintf(buf, PAGE_SIZE, "%d\n",pshcsr_dev_obj->Pring_buf->ptr[pshcsr_dev_obj->Pring_buf->tail].TSC);
	
}

static ssize_t show_enable(struct device *dev_struct, struct device_attribute *attr, char *buf)
{
        struct hcsr_dev *device_struct  = dev_get_drvdata(dev_struct);
        return snprintf(buf, PAGE_SIZE, "%d\n", device_struct->enable);
}

static ssize_t show_trigger_pin(struct device *dev_struct, struct device_attribute *attr, char *buf)
{

	struct hcsr_dev *device_struct = dev_get_drvdata(dev_struct);
        return snprintf(buf, PAGE_SIZE, "%d\n", device_struct->pin.trigger_pin);
}


/*Store functions*/
static ssize_t store_trigger_pin(struct device *dev_struct,
                                  struct device_attribute *attr,
                                  const char *buf,
                                  size_t count)
{
	struct hcsr_dev *device_struct = dev_get_drvdata(dev_struct);
		
        sscanf(buf, "%d", &device_struct->pin.trigger_pin);
	set_pin_conf(device_struct->pin.trigger_pin, OUTPUT_FUNC);	
	return count;

}


static ssize_t store_echo_pin(struct device *dev_struct,
                                  struct device_attribute *attr,
                                  const char *buf,
                                  size_t count)
{
	struct hcsr_dev *device_struct = dev_get_drvdata(dev_struct);
		
        sscanf(buf, "%d", &pshcsr_dev_obj->echo_pin);
	set_pin_conf(device_struct->pin.echo_pin, INPUT_FUNC);
	return count;

}

static ssize_t store_enable(struct device *dev_struct,
                                  struct device_attribute *attr,
                                  const char *buf,
                                  size_t count)
{
	struct hcsr_dev *device_struct = dev_get_drvdata(dev_struct);
		
        sscanf(buf, "%d", &device_struct->enable);
	return count;

}


static ssize_t enable(struct device *dev_struct,
                                  struct device_attribute *attr,
                                  const char *buf,
                                  size_t count)
{
	int integer,ret;
	struct hcsr_dev *device_data = dev_get_drvdata(dev_struct);
	sscanf(buf, "%d",&integer);

	if(device_data->is_in_progress){
		printk(KERN_INFO "On-Going Measurement");
		return -EINVAL;
	}else{
		if(input  == 0) {
            	printk(KERN_INFO "Clearing the buffer\n");
			// write code to handle clearing of buffer
		}
		printk(KERN_INFO "Starting the Sampling now!!!!!\n");
		device_data->is_in_progress = true;
        	kthread_run(&start_sampling,(void *)device_data, "start_sampling");

	}
	
	printk("write freed\n");	
	return count;
}


static DEVICE_ATTR(trigger_pin,S_IRWXU, show_trigger_pin, store_trigger_pin);
static DEVICE_ATTR(echo_pin,S_IRWXU, show_echo_pin, store_echo_pin);
static DEVICE_ATTR(enable,S_IRWXU, show_enable, store_enable);
static DEVICE_ATTR(distance, S_IRWXU, show_distance, NULL);


static int Plat_driver_probe(struct platform_device *device_found)
{
	
	int rval;	
	struct hcsr_dev device_struct;

	struct HCSRdevice* plat_device;
	
	plat_device = container_of(device_found, struct HCSRdevice, dev);
	
	printk(KERN_ALERT "Found the device -- %s  %d \n", plat_device->name, pdevice->dev_n);
	if(first == 1)
	{
		 gko_class = class_create(THIS_MODULE, CLASS_NAME);
			if (IS_ERR(gko_class)) {
				printk( " cant create class %s\n", CLASS_NAME);
				goto class_err;

			}
		first =0;
	}
	if(!(device_struct =init_device((void*)plat_device)))
	{
		printk("device initialisation failed\n");
	}

	INIT_LIST_HEAD(&device_struct->device_entry) ;
	list_add(&device_struct->device_entry, &device_list );

	/* device */
        gko_device = device_create(gko_class, NULL, gko_dev, device_struct, device_struct->dev_name);
        if (IS_ERR(gko_device)) {
                
                printk( " cant create device %s\n", device_struct->dev_name);
                goto device_err;
        }

	printk("device create\n");	
	rval = device_create_file(gko_device, &dev_attr_trigger_pin);
        if (rval < 0) {
                printk(" cant create device attribute %s %s\n", 
                       device_struct->dev_name, dev_attr_trigger_pin.attr.name);
        }
	
	rval = device_create_file(gko_device, &dev_attr_echo_pin);
        if (rval < 0) {
               printk(" cant create device attribute %s %s\n", 
                       device_struct->dev_name, dev_attr_echo_pin.attr.name);
        }

	rval = device_create_file(gko_device, &dev_attr_mode);
        if (rval < 0) {
               printk(" cant create device attribute %s %s\n", 
                       device_struct->dev_name, dev_attr_mode.attr.name);
        }

	rval = device_create_file(gko_device, &dev_attr_frequency);
        if (rval < 0) {
       printk(" cant create device attribute %s %s\n", 
                       device_struct->dev_name, dev_attr_frequency.attr.name);
        }

	rval = device_create_file(gko_device, &dev_attr_Enable);
        if (rval < 0) {
     		printk(" cant create device attribute %s %s\n", 
                device_struct->dev_name, dev_attr_Enable.attr.name);
        }

        rval = device_create_file(gko_device, &dev_attr_distance);
        if (rval < 0) {
   	 printk(" cant create device attribute %s %s\n", 
                       device_struct->dev_name, dev_attr_distance.attr.name);
        }
	printk("probe done\n");

	return 0;
	device_err:
       		 device_destroy(gko_class, gko_dev);
	class_err:
        	class_unregister(gko_class);
        	class_destroy(gko_class);
return -EFAULT;			

};

static int Plat_driver_remove(struct platform_device *plat_dev)
{	
	struct hcsr_dev* device_struct;	
	printk("enter remove of the probe\n");

	list_for_each_entry(ptempcontext, &device_list, device_entry)
	{
	    	printk("remove a list\n");
		if(ptempcontext->dev_name == plat_dev->name)
		{
		    --usDeviceNo;
		    list_del(&device_struct->device_entry);
	    	    device_destroy(gko_class, device_struct->misc_device.minor);
		    misc_deregister(&device_struct->misc_device);
	   	    printk("freed misc\n");
		    kfree(device_struct);
	   	    printk("freed obj\n");
		    break;

		}
	}


}

static struct platform_driver Plat_driver = {
	.driver		= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
	.probe		= Plat_driver_probe,
	.remove		= Plat_driver_remove,
	.id_table	= Plat_id_table,
};

module_platform_driver(Plat_driver);
MODULE_LICENSE("GPL");
