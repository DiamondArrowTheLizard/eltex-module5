#include <linux/init.h>       
#include <linux/module.h>     
#include <linux/proc_fs.h>    
#include <linux/seq_file.h>   

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Artemiy Dementev");
MODULE_DESCRIPTION("Simple /proc creation module");

#define PROC_NAME "proc_greeter"
#define PROC_PERMS 0644
#define PROC_PARENT_DIR NULL

#define PROC_MESSAGE "Hello World"
#define PROC_MESSAGE_SIZE 200


static int greeter_proc_show(struct seq_file* m, void* v);
static int greeter_proc_open(struct inode* inode, struct file* file);
static const struct proc_ops greeter_proc_opts = {
    .proc_open = greeter_proc_open,
    .proc_read = seq_read,
    .proc_release = single_release,
};
static struct proc_dir_entry *greeter_proc_file;  

static int greeter_proc_show(struct seq_file* m, void* v) {
	seq_printf(m, "/proc/%s was created\n", PROC_NAME);
	return 0;
}

static int greeter_proc_open(struct inode* inode, struct file* file) {
	return single_open(file, greeter_proc_show, NULL);
}
 
static int __init proc_init (void) {
    printk(KERN_INFO "infoshare: module loaded\n");
	greeter_proc_file = proc_create(
		PROC_NAME,
		PROC_PERMS,
		PROC_PARENT_DIR,
		&greeter_proc_opts
	);
	
	if(!greeter_proc_file) {
		printk(KERN_ERR, "infoshare: failed to create /proc/%s\n", PROC_NAME);
		return -ENOMEM;
	}

	printk(KERN_INFO, "infoshare: created /proc/%s\n", PROC_NAME);
    return 0;
}
 
static void __exit proc_cleanup(void) {
	proc_remove(greeter_proc_file);
	printk(KERN_INFO, "infoshare: removed /proc/%s\n", PROC_NAME);
    printk(KERN_INFO "infoshare: module unloaded\n");
}
 
module_init(proc_init);
module_exit(proc_cleanup);
