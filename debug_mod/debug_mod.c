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

#define AUTHOR "Nicholas Hunt <nhunt@cs.washington.edu>"
#define DESC   "Make debug registers accesible via the /proc filesystem"

#define PROC_DIRNAME "dr"
#define NR_DRS    8

struct proc_dir_entry *proc_dir = NULL;
struct proc_dir_entry *drs[NR_DRS] = {NULL};
static int create_proc_entries(void);
static void remove_proc_entries(void);

static int debug_mod_read(char *page, char **start, off_t off, int count, int *eof, void *data);
static int debug_mod_write(struct file *file, const char __user *buffer, unsigned long cnt, void *data);

static int __init debug_mod_init(void)
{
	int ret = 0;

	printk(KERN_INFO "debug_mod loading...\n");

	ret = create_proc_entries();
	if (ret != 0) {
		printk(KERN_INFO "Something bad happened.\n");
		return ret;
	}

	printk(KERN_INFO "done.\n");
	return 0;
}

static void __exit debug_mod_exit(void)
{
	remove_proc_entries();
	printk(KERN_INFO "debug_mod unloaded.\n");
}

int create_proc_entries(void)
{
	int i, j, ret = 0;
	char buffer[32];

	printk(KERN_INFO "  Creating proc entries\n");

	proc_dir = proc_mkdir(PROC_DIRNAME, NULL);
	if (proc_dir == NULL) {
		ret = -1;
		printk(KERN_INFO "Couldn't create dir /proc/%s\n", PROC_DIRNAME);
		goto out_ret;
	}

	for (i = 0; i < NR_DRS; i++) {
		snprintf(buffer, sizeof buffer, "dr%d", i);
		printk(KERN_INFO "    Creating /proc/%s/%s...\n", PROC_DIRNAME, buffer);

		drs[i] = create_proc_entry(buffer, 0666, proc_dir);
		if (drs[i] == NULL) {
			printk(KERN_INFO "  Couldn't create proc entry\n");
			ret = -1;
			goto out_free_dir;
		}

		drs[i]->read_proc  = debug_mod_read;
		drs[i]->write_proc = debug_mod_write;
		drs[i]->owner = THIS_MODULE;
		drs[i]->mode  = S_IFREG | S_IRUGO | S_IWUGO;
		drs[i]->uid   = 0;
		drs[i]->gid   = 0;
		drs[i]->size  = 8;
		drs[i]->data  = (void *)((uint64_t)i);
	}

	return ret;

 out_free_dir:
 	for (j = 0; j < i; j++) {
		snprintf(buffer, sizeof buffer, "dr%d", i);
		printk(KERN_INFO "  Removing /proc/%s/%s\n", PROC_DIRNAME, buffer);
		remove_proc_entry(buffer, proc_dir);
	}

	printk(KERN_INFO "  Removing /proc/%s\n", PROC_DIRNAME);
	remove_proc_entry(PROC_DIRNAME, NULL);

 out_ret:
 	return ret;
}

void remove_proc_entries(void)
{
	int i;
	char buffer[32];

	printk(KERN_INFO "Removing proc entries\n");

	for (i = 0; i < NR_DRS; i++) {
		snprintf(buffer, sizeof buffer, "dr%d", i);
		printk(KERN_INFO "    Removing /proc/%s/%s...\n", PROC_DIRNAME, buffer);
		remove_proc_entry(buffer, proc_dir);
	}

	remove_proc_entry(PROC_DIRNAME, NULL);

	printk(KERN_INFO "Done.\n");
}


static long read_dr(int nr)
{
	long ret = -EINVAL;
	switch (nr) {
		case 0: asm("movq %%dr0, %0" : "=r" (ret)); break;
		case 1: asm("movq %%dr1, %0" : "=r" (ret)); break;
		case 2: asm("movq %%dr2, %0" : "=r" (ret)); break;
		case 3: asm("movq %%dr3, %0" : "=r" (ret)); break;
		case 4: asm("movq %%dr4, %0" : "=r" (ret)); break;
		case 5: asm("movq %%dr5, %0" : "=r" (ret)); break;
		case 6: asm("movq %%dr6, %0" : "=r" (ret)); break;
		case 7: asm("movq %%dr7, %0" : "=r" (ret)); break;
		default:
			printk(KERN_INFO "Invalid debug register number %d\n", nr);
			break;
	}

	return ret;
}

static long write_dr(int nr, long val)
{
	long ret = 0;
	switch (nr) {
		case 0: asm("movq %0, %%dr0" :: "r" (val)); current->thread.debugreg0 = val; break;
		case 1: asm("movq %0, %%dr1" :: "r" (val)); current->thread.debugreg1 = val; break;
		case 2: asm("movq %0, %%dr2" :: "r" (val)); current->thread.debugreg2 = val; break;
		case 3: asm("movq %0, %%dr3" :: "r" (val)); current->thread.debugreg3 = val; break;
		case 6: asm("movq %0, %%dr6" :: "r" (val)); current->thread.debugreg6 = val; break;
		case 7: asm("movq %0, %%dr7" :: "r" (val)); current->thread.debugreg7 = val; break;
		default:
			printk(KERN_INFO "Invalid debug register number %d\n", nr);
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

	printk(KERN_INFO "Reading %d:dr%d\n", current->pid, nr);
	val = read_dr(nr);

	printk(KERN_INFO "  Got value %ld (%lx)\n", val, val);
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
		printk(KERN_INFO "Bad user address\n");
		return -EPERM;
	}

	if (copy_from_user(&val, buffer, cnt) != 0) {
		printk(KERN_INFO "Couldn't copy %ld bytes\n", cnt);
		return -EACCES;
	}

	set_thread_flag(TIF_DEBUG);

	printk(KERN_INFO "Writing %d:dr%d, val:%ld (%lx)\n", current->pid, nr, val, val);
	ret = write_dr(nr, val);
	val = read_dr(nr);
	printk(KERN_INFO "  Reading to check: %ld (%lx)\n", val, val);

	return cnt;
}

module_init(debug_mod_init);
module_exit(debug_mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESC);
