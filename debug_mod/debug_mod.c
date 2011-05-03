/*
 * debug_mod -- 
 *
 * A kernel module to expose the debug registers of a process through the /proc
 * filesystem. /proc/dbg/dr[0..8]
 */
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/moduleparam.h>

#define PROC_DIRNAME "dr"
#define NR_DRS       8

#define DBG_VERSION 0.8
#define DBG_AUTHOR "Nicholas Hunt <nhunt@cs.washington.edu>"
#define DBG_DESC   "Make debug registers accesible via the /proc filesystem"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DBG_AUTHOR);
MODULE_DESCRIPTION(DBG_DESC);
//MODULE_VERSION(DBG_VERSION);

#ifdef CONFIG_X86_32
	#define DR_SIZE 4
	#define READ_DR(nr, ret) asm("movl %%dr" #nr ", %0" : "=r" (ret));
	#define WRITE_DR(nr, val) \
       		do { \
			asm("movl %0, %%dr" #nr :: "r" (val)); \
			current->thread.debugreg ##nr = val;  \
		} while(0)

#elif CONFIG_X86_64
	#define DR_SIZE 8
	#define READ_DR(nr, ret) asm("movq %%dr" #nr ", %0" : "=r" (ret));
	#define WRITE_DR(nr, val) \
       		do { \
			asm("movq %0, %%dr" #nr :: "r" (val)); \
			current->thread.debugreg ##nr = val;  \
		} while(0)
#else
	#error "Only x86 and x86_64 architectures are supported\n"
#endif


static int debug = 0;
module_param(debug, int, 0);
//MODULE_PARAM_DESC(debug, "Set to a non-zero value to enable debug messages");

struct proc_dir_entry *proc_dir = NULL;
struct proc_dir_entry *drs[NR_DRS] = {NULL};
static int create_proc_entries(void);
static void remove_proc_entries(void);

static int debug_mod_read(char *page, char **start, off_t off, int count, int *eof, void *data);
static int debug_mod_write(struct file *file, const char __user *buffer, unsigned long cnt, void *data);

static void debug_print(const char *fmt, ...)
{
        va_list args;
        int r;

	if (debug) {
		va_start(args, fmt);
		r = vprintk(fmt, args);
		va_end(args);
	}
}

static int __init debug_mod_init(void)
{
	int ret = 0;

	debug_print(KERN_INFO "debug_mod loading...\n");

	ret = create_proc_entries();
	if (ret != 0) {
		debug_print(KERN_INFO "Something bad happened.\n");
		return ret;
	}

	debug_print(KERN_INFO "done.\n");
	return 0;
}

static void __exit debug_mod_exit(void)
{
	remove_proc_entries();
	debug_print(KERN_INFO "debug_mod unloaded.\n");
}

int create_proc_entries(void)
{
	long i, j, ret = 0;
	char buffer[32];

	debug_print(KERN_INFO "  Creating proc entries\n");

	proc_dir = proc_mkdir(PROC_DIRNAME, NULL);
	if (proc_dir == NULL) {
		ret = -1;
		debug_print(KERN_INFO "Couldn't create dir /proc/%s\n", PROC_DIRNAME);
		goto out_ret;
	}

	for (i = 0; i < NR_DRS; i++) {
		snprintf(buffer, sizeof buffer, "dr%ld", i);
		debug_print(KERN_INFO "    Creating /proc/%s/%s...\n", PROC_DIRNAME, buffer);

		drs[i] = create_proc_entry(buffer, 0666, proc_dir);
		if (drs[i] == NULL) {
			debug_print(KERN_INFO "  Couldn't create proc entry\n");
			ret = -1;
			goto out_free_dir;
		}

		drs[i]->read_proc  = debug_mod_read;
		drs[i]->write_proc = debug_mod_write;
		drs[i]->owner = THIS_MODULE;
		drs[i]->mode  = S_IFREG | S_IRUGO | S_IWUGO;
		drs[i]->uid   = 0;
		drs[i]->gid   = 0;
		drs[i]->size  = DR_SIZE;
		drs[i]->data  = (void *)(i);
	}

	return ret;

 out_free_dir:
 	for (j = 0; j < i; j++) {
		snprintf(buffer, sizeof buffer, "dr%ld", j);
		debug_print(KERN_INFO "  Removing /proc/%s/%s\n", PROC_DIRNAME, buffer);
		remove_proc_entry(buffer, proc_dir);
	}

	debug_print(KERN_INFO "  Removing /proc/%s\n", PROC_DIRNAME);
	remove_proc_entry(PROC_DIRNAME, NULL);

 out_ret:
 	return ret;
}

void remove_proc_entries(void)
{
	int i;
	char buffer[32];

	debug_print(KERN_INFO "Removing proc entries\n");

	for (i = 0; i < NR_DRS; i++) {
		snprintf(buffer, sizeof buffer, "dr%d", i);
		debug_print(KERN_INFO "    Removing /proc/%s/%s...\n", PROC_DIRNAME, buffer);
		remove_proc_entry(buffer, proc_dir);
	}

	remove_proc_entry(PROC_DIRNAME, NULL);
	debug_print(KERN_INFO "Done.\n");
}

static long read_dr(int nr)
{
	long ret = -EINVAL;
	switch (nr) {
		case 0: READ_DR(0, ret); break;
		case 1: READ_DR(1, ret); break;
		case 2: READ_DR(2, ret); break;
		case 3: READ_DR(3, ret); break;
		case 4: READ_DR(4, ret); break;
		case 5: READ_DR(5, ret); break;
		case 6: READ_DR(6, ret); break;
		case 7: READ_DR(7, ret); break;
		default:
			debug_print(KERN_INFO "Invalid debug register number %d\n", nr);
			break;
	}

	return ret;
}

static long write_dr(int nr, long val)
{
	long ret = 0;
	switch (nr) {
		case 0: WRITE_DR(0, val); break;
		case 1: WRITE_DR(1, val); break;
		case 2: WRITE_DR(2, val); break;
		case 3: WRITE_DR(3, val); break;
		case 6: WRITE_DR(6, val); break;
		case 7: WRITE_DR(7, val); break;
		default:
			debug_print(KERN_INFO "Invalid debug register number %d\n", nr);
			ret = -EINVAL;
			break;
	}

	return ret;
}

static int debug_mod_read(char *page, char **start, off_t off,
                          int count, int *eof, void *data)
{
	int nr = (int)((long)data);
	long val;

	//debug_print(KERN_INFO "Reading %d:dr%d\n", current->pid, nr);
	val = read_dr(nr);

	//debug_print(KERN_INFO "  Got value %ld (%lx)\n", val, val);
	*((long *)page) = val;
	*eof = 1;

	return sizeof val;
}

static int debug_mod_write(struct file *file, const char __user *buffer,
                           unsigned long cnt, void *data)
{
	int ret, nr = (int)((long)data);
	long val = 0;

	cnt = cnt > sizeof val ? sizeof val : cnt;

	if (!access_ok(VERIFY_READ, buffer, cnt)) {
		debug_print(KERN_INFO "Bad user address\n");
		return -EPERM;
	}

	if (copy_from_user(&val, buffer, cnt) != 0) {
		debug_print(KERN_INFO "Couldn't copy %ld bytes\n", cnt);
		return -EACCES;
	}

	set_thread_flag(TIF_DEBUG);

	//debug_print(KERN_INFO "Writing %d:dr%d, val:%ld (%lx)\n", current->pid, nr, val, val);
	ret = write_dr(nr, val);
	//val = read_dr(nr);
	//debug_print(KERN_INFO "  Reading to check: %ld (%lx)\n", val, val);

	return cnt;
}

module_init(debug_mod_init);
module_exit(debug_mod_exit);

