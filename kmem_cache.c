#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/mm.h>

MODULE_AUTHOR("hiroya");
MODULE_DESCRIPTION("kmem_cache test");
MODULE_LICENSE("GPL");

static LIST_HEAD(caches);
static unsigned long current_id;

struct foo_bar {
	unsigned long id;
};

static struct kmem_cache *cachep;
struct dentry *dentry_kmem_cache;
static int value;

static void init_once(void *p)
{
	struct foo_bar *foo = (struct foo_bar *)p;
	foo->id = current_id++;
}

static ssize_t cachep_write(struct file *file, const char __user *buf,
			 size_t count, loff_t *pos)
{
	struct foo_bar *foo;

	foo = (struct foo_bar *)kmem_cache_alloc(cachep, GFP_KERNEL);
	if (!foo)
		return -ENOMEM;

	kmem_cache_free(cachep, foo);

	return count;
}

static const struct file_operations fops = {
	.write = cachep_write,
};

static int __init cachep_init(void)
{
	cachep = kmem_cache_create("@debugslub",
			  sizeof(struct foo_bar),
			  0, SLAB_ACCOUNT|SLAB_PANIC, init_once);
	if (!cachep)
		return -ENOMEM;

	dentry_kmem_cache = debugfs_create_file("debugslub", 0666,
						NULL, &value, &fops);
	if (!dentry_kmem_cache) {
		kmem_cache_destroy(cachep);
		return -ENODEV;
	}
	return 0;
}

static void __exit cachep_exit(void)
{
	kmem_cache_destroy(cachep);
	debugfs_remove(dentry_kmem_cache);
}

module_init(cachep_init);
module_exit(cachep_exit);
