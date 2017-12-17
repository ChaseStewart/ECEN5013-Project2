/*****************************************************
 * Redistribution, modification or use of this software in source or binary forms 
 * is permitted as long as the files maintain this copyright. Users are permitted to 
 * modify this and use it to learn about the field of embedded software but don't copy 
 * my (Chase E Stewart's) work for class, I worked really hard on this. Alex Fosdick and 
 * the University of Colorado and Chase E Stewart are not liable for any misuse of this material. 
 * License copyright (C) 2017 originally from Alex Fosdick, code by Chase E Stewart.
 *****************************************************/
/*
 * @file led_driver.c
 * @brief A driver to control Beaglebone LEDs in userspace 
 * 
 * A driver with an init, an exit, and an IRQ handler
 * 
 *
 * @author Chase E Stewart
 * @date November 11 2017
 * @version 1.0
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

/* device vars*/
#define DEVICE_NAME "myledchar"
#define CLASS_NAME  "myled"
#define INPUT_LEN 1024
#define DELIM_STR ":\t\n"

/* GPIO vars */
#define LED_GPIO 53
#define LED_ON   1
#define LED_OFF  0 

/* module info */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chase E Stewart");
MODULE_DESCRIPTION("A dead simple char driver for BBB LEDs");
MODULE_VERSION("1.0");

/* prototypes */
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static int dev_read(struct file *, char *, size_t, loff_t *);
static int dev_write(struct file *, const char *, size_t, loff_t *);

/* file ops -> file pointers structs */
static struct file_operations fops = 
{
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release
};

/* state vars */
static bool led_state      = 0;
static int  led_freq       = 0;
static int  led_duty       = 0;
static int  led_period     = 0;
static int  num_opens      = 0;
static int  update_timers  = 0;

/* led timer */
static int  led_on_time  = 0;
static int  led_off_time = 0;
static struct timer_list led_on_timer;
static struct timer_list led_off_timer;

/* class-related vars */
static char   *first_word;
static char   *second_word;
static int    driver_number = 0;
static struct class*  ledDriverClass  = NULL; 
static struct device* ledDriverDevice = NULL; 


static void on_timer_tick(unsigned long data)
{
	int retval;
	if (update_timers)
	{
		retval = mod_timer(&led_off_timer, jiffies+msecs_to_jiffies(led_on_time));
	}
	gpio_set_value(LED_GPIO, LED_ON);
}

static void off_timer_tick(unsigned long data)
{
	int retval;
	if (update_timers)
	{
		retval = mod_timer(&led_on_timer, jiffies+msecs_to_jiffies(led_off_time));
	}
	gpio_set_value(LED_GPIO, LED_OFF);
}


/*
 * Init LED driver needs to setup the LED, the driver
 */
static int __init init_led_driver(void){
	
	/* register the gpio */
	if (!gpio_is_valid(LED_GPIO))
	{
		printk(KERN_ERR "[led_driver] Invalid GPIO pin provided\n");
		return 1;
	}
	led_state = LED_ON;
	gpio_request(LED_GPIO, "sysfs");
	gpio_direction_output(LED_GPIO, LED_ON);
	gpio_export(LED_GPIO, false);
	printk(KERN_INFO "[led_driver] LED GPIO pin %d Initialized\n", LED_GPIO);

	/* grab a major number for the device driver */
	printk(KERN_INFO "[led_driver] Initiating LED Driver\n");
	driver_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (driver_number < 0)
	{
		printk(KERN_ERR "[led_driver] could not register the driver!\n");
		return driver_number;
	}
	printk(KERN_INFO "[led_driver] registered the driver with driver number %d\n", driver_number);

	/* attempt to register the device with the driver number */
	ledDriverClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(ledDriverClass))
	{
		unregister_chrdev(driver_number, DEVICE_NAME);
		printk(KERN_ERR "Failed to register device class\n");
		return PTR_ERR(ledDriverClass);
	}
	printk(KERN_INFO "[led_driver] Successfully registered device class");

	/* register device driver */
	ledDriverDevice = device_create(ledDriverClass, NULL, MKDEV(driver_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(ledDriverDevice))
	{
		class_destroy(ledDriverClass);
		unregister_chrdev(driver_number, DEVICE_NAME);
		printk(KERN_ERR "Failed to create device!\n");
		return PTR_ERR(ledDriverDevice);
	}
	printk(KERN_INFO "[led_driver] Successfully initialized the driver\n");

	setup_timer( &led_on_timer,  on_timer_tick,  0);
	setup_timer( &led_off_timer, off_timer_tick, 0);
	update_timers = 1;

	return 0;
}

/*
 * Open is kinda a throwaway, just log number of times this has been accessed.
 */
static int dev_open(struct inode *inodep, struct file *filep)
{
	num_opens++;
	printk(KERN_INFO "[led_driver] LED Driver has been accessed %d times\n", num_opens );
	return 0;
}

/*
 * Use read to return the status of the LED. Should maybe also copy-to-user
 */
static int dev_read(struct file *myfile, char *mybuffer, size_t len, loff_t *myoffset)
{
	int retval;
	char mystring[INPUT_LEN];
	sprintf(mystring, "%d:%d:%d:%d\n", led_state, led_freq, led_duty, num_opens);

	retval = copy_to_user(mybuffer, mystring, strlen(mystring));
	if (retval != 0)
	{
		printk(KERN_ERR "[led_driver] Failed to copy %d bytes to user\n", retval);
		return 1;
	}
	return 0;
}



/*
 * Set the LED value based on what is written to the driver
 */
static int dev_write(struct file *myfile, const char *mybuffer, size_t len, loff_t *myoffset)
{
	int retval;
	char * char_message;
	char * destruct_message;
	
	char_message     = (char *) kmalloc(INPUT_LEN, GFP_KERNEL);
	destruct_message = (char *) kmalloc(INPUT_LEN, GFP_KERNEL);

	retval = copy_from_user(char_message, mybuffer, len);
	if (retval != 0)
	{
		printk(KERN_ERR "[led_driver] Failed to copy %d bytes from user\n", retval);
		return 1;
	}

	printk(KERN_INFO "char_message is <%s>\n", char_message);
	strncpy(destruct_message, char_message, strlen(char_message));
	first_word = strsep(&destruct_message, DELIM_STR);
	second_word = strsep(&destruct_message, DELIM_STR);
	
	printk(KERN_INFO "[led_driver] First word is <%s> and second word is <%s> \n", first_word, second_word);

	if (strcmp(first_word, "state") == 0)
	{
		if (strcmp(second_word, "on") == 0)
		{
			led_state = LED_ON;
			gpio_set_value(LED_GPIO, LED_ON);
			update_timers = 0;
			led_duty = 50;
			printk(KERN_INFO "[led_driver] Setting LED On\n");
		}
		else if (strcmp(second_word, "off") == 0)
		{
			led_state = LED_OFF;
			gpio_set_value(LED_GPIO, LED_OFF);
			update_timers = 0;
			led_duty = 50;
			printk(KERN_INFO "[led_driver] Setting LED Off\n");
		}
		else
		{
			printk(KERN_ALERT "[led_driver] Driver received invalid response\n");
			update_timers = 0;
		}
	}	
	else if (strcmp(first_word, "freq") == 0)
	{
		if ((led_freq >=0) && (led_freq <=1000) )
		{
			sscanf(second_word, "%d", &led_freq);
			printk(KERN_INFO "[led_driver] setting led freq to %d\n", led_freq);
			led_period   = (1000 / led_freq);
			led_on_time  = (led_period * led_duty ) / ( 100 );
			led_off_time = (led_period * (100-led_duty) ) / ( 100 );
			printk(KERN_INFO "[led_driver] led period %d freq: %d duty: %d \n", led_period, led_freq, led_duty);
			printk(KERN_INFO "[led_driver] led on-time: %d off-time: %d \n", led_on_time, led_off_time);
			update_timers = 1;
			mod_timer(&led_on_timer, jiffies+msecs_to_jiffies(led_on_time));
		}
		
	}
	else if (strcmp(first_word, "duty") == 0)
	{
		sscanf(second_word, "%d", &led_duty);
		if ((led_duty >=0) && (led_duty <=100) )
		{
			printk(KERN_INFO "[led_driver] Setting led duty cycle to %d\n", led_duty);
			led_period   = (1000 / led_freq);
			led_on_time  = (led_period * led_duty ) / ( 100 );
			led_off_time = (led_period * (100-led_duty) ) / ( 100 );
			printk(KERN_INFO "[led_driver] led period %d freq: %d duty: %d \n", led_period, led_freq, led_duty);
			printk(KERN_INFO "[led_driver] LED on-time: %d off-time: %d \n", led_on_time, led_off_time);
			update_timers = 1;
			mod_timer(&led_on_timer, jiffies+msecs_to_jiffies(led_on_time));
		}
		else
		{
			printk(KERN_ALERT "[led_driver] Tried to set duty cycle to invalid value %d\n", led_duty);
			led_duty = 50;
		}
	}

	return len;
}



/*
 * On a /dev release, just log a message 
 */
static int dev_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "[led_driver] Closed the LED driver device\n");
	return 0;
}



/*
 * On exit, free GPIO and driver resources and log a result message
 */
static void __exit exit_led_driver(void){
	printk(KERN_INFO "[led_driver] Closing  LED Driver\n");

	/* First, free GPIO resources */
	gpio_set_value(LED_GPIO, LED_OFF);
	gpio_unexport(LED_GPIO);
	gpio_free(LED_GPIO);
	printk(KERN_INFO "[led_driver] GPIO links terminated\n");

	/* Now destroy driver */
	device_destroy(ledDriverClass, MKDEV(driver_number, 0));
	printk(KERN_INFO "[led_driver] destroyed device\n");
	
	class_unregister(ledDriverClass);
	printk(KERN_INFO "[led_driver] unregistered class\n");

	class_destroy(ledDriverClass);
	printk(KERN_INFO "[led_driver] destroyed class\n");

	unregister_chrdev(driver_number, DEVICE_NAME);
	printk(KERN_INFO "[led_driver] unregistered chrdev\n");

	update_timers = 0;
	del_timer(&led_on_timer);
	del_timer(&led_off_timer);
	
	printk(KERN_INFO "[led_driver] LED Driver exit... Goodbye!\n");
}


/* use macros to setup init and teardown functions */
module_init(init_led_driver);
module_exit(exit_led_driver);


