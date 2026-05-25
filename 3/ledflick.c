#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/configfs.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/console_struct.h>
#include <linux/vt_kern.h>
#include <linux/timer.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

#define LEDFLICK_BLINK_DELAY   HZ/5
#define LEDFLICK_ALL_LEDS_ON   0x07
#define LEDFLICK_RESTORE_LEDS  0xFF

#define LEDFLICK_SYSFS_NAME "ledflick"

static struct timer_list ledflick_timer;
static struct tty_driver* ledflick_driver;

static struct kobject* ledflick_kobject;
static int ledflick_mask = 0;
static int _ledflickstatus = 0;

static ssize_t ledflick_show(struct kobject *kobj, struct kobj_attribute *attr,
							 char *buf)
{
	return snprintf(buf, strlen(buf), "%d\n", ledflick_mask);
}

static ssize_t ledflick_store(struct kobject *kobj, struct kobj_attribute *attr,
							  const char *buf, size_t count)
{
	sscanf(buf, "%du", &ledflick_mask);
	pr_info("ledflick: written mask %d\n", ledflick_mask);
	return count;
}

static struct kobj_attribute ledflick_attribute =__ATTR(ledflick, 0660, ledflick_show,
														ledflick_store);

static void ledflick_timer_func(struct timer_list* timer) {
	int* pstatus = &_ledflickstatus;	

	if(*pstatus == ledflick_mask) {
		*pstatus = LEDFLICK_RESTORE_LEDS;

	} else {
		*pstatus = ledflick_mask;
	}

	(ledflick_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, *pstatus);
	ledflick_timer.expires = jiffies + LEDFLICK_BLINK_DELAY;
	add_timer(&ledflick_timer);
}

static int __init ledflick_init(void) {

	int error = 0;
	pr_info("ledflick: initializing module\n");

	ledflick_kobject = kobject_create_and_add(LEDFLICK_SYSFS_NAME, kernel_kobj);
	if(!ledflick_kobject) {
		return -ENOMEM;
	}

	error = sysfs_create_file(ledflick_kobject, &ledflick_attribute.attr);
	if(error) {
		pr_err("ledflick ERROR: failed to create sysfs file %s\n", LEDFLICK_SYSFS_NAME);
	}

	pr_info("ledflick: fgconsole is %x\n", fg_console);
	for (int i = 0; i < MAX_NR_CONSOLES; i++) {
		if (!vc_cons[i].d) {
			break;
		}
		pr_info("ledflick: console[%i/%i] #%i, tty %lx\n", i,
		 MAX_NR_CONSOLES, vc_cons[i].d->vc_num,
		 (unsigned long)vc_cons[i].d->port.tty);
	}

	ledflick_driver = vc_cons[fg_console].d->port.tty->driver;

	timer_setup(&ledflick_timer, ledflick_timer_func, 0);
	ledflick_timer.expires = jiffies + LEDFLICK_BLINK_DELAY;
	add_timer(&ledflick_timer);

	pr_info("ledflick: initialization complete\n");
	return error;
}

static void __exit ledflick_exit(void) {
	pr_info("ledflick: stopping module\n");
	kobject_put(ledflick_kobject);
	timer_delete(&ledflick_timer);
	(ledflick_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, LEDFLICK_RESTORE_LEDS);
	pr_info("ledflick: module stopped\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Artemiy Dement'ev");
MODULE_DESCRIPTION("LED flicker using ioctl && sysfs");

module_init(ledflick_init);
module_exit(ledflick_exit);
