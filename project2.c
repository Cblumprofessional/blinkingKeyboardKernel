#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/tty.h>
#include <linux/kd.h>
#include <linux/vt_kern.h>

static struct timer_list my_timer;
static int led_state = 0; // Global variable to store the LED state (0-7)
static int blink_delay = HZ; // Global variable to store the blink delay, initialized with HZ

void blink_led(unsigned long data)
{
    static bool on = false;
    struct tty_driver *my_driver;
    my_driver = vc_cons[fg_console].d->port.tty->driver;

    // Toggle LED state
    if (on)
        (my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, led_state);
    else
        (my_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, 0);

    on = !on;

    // Reset the timer
    mod_timer(&my_timer, jiffies + blink_delay);
}

ssize_t proc_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    char buf[128];
    if (count > sizeof(buf))
        return -EINVAL;

    if (copy_from_user(buf, ubuf, count))
        return -EFAULT;

    buf[count] = '\0';

    // Parse user input
    if (buf[0] == 'L') {
        sscanf(buf, "L%d", &led_state);
        led_state &= 0x07; // Ensure led_state is between 0-7
    }
    else if (buf[0] == 'D') {
        int divisor;
        sscanf(buf, "D%d", &divisor);
        blink_delay = HZ / (divisor ? divisor : 1); // Prevent division by zero
    }
    static int __init led_blink_init(void)
{
    // Create /proc file
    proc_create("led_blink", 0666, NULL, &proc_fops);

    // Setup the timer
    setup_timer(&my_timer, blink_led, 0);
    mod_timer(&my_timer, jiffies + blink_delay);

    return 0;
}

static void __exit led_blink_cleanup(void)
{
    del_timer(&my_timer);
    remove_proc_entry("led_blink", NULL);
}

module_init(led_blink_init);
module_exit(led_blink_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A kernel module to blink keyboard LEDs controlled via /proc file.");


    return count;
}

static const struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .write = proc_write,
};
