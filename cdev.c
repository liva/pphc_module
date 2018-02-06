#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include "cdev.h"

#define DEVNAME "pphc"

static int cdev_major;
static struct cdev cdev_st;
static struct file_operations depftom_fops;

int pphc_cdev_init(void) {
  int ret;
  dev_t cdev;

  // alloc decive number
  ret = alloc_chrdev_region(&cdev, 0, 1, DEVNAME);

  if (ret < 0) {
    pr_warn("cdev_init: alloc device number failed: %d\n", ret);
    return -1;
  }

  cdev_init(&cdev_st, &depftom_fops);
  cdev_st.owner = THIS_MODULE;
  cdev_major = MAJOR(cdev);
  if (cdev_add(&cdev_st, MKDEV(cdev_major, 0), 1)) {
    pr_warn("depftom_init: fail to add cdev\n");
    return -1;
  }

  pr_info("depftom_init: please run 'mknod /dev/pphc c %d 0'\n", cdev_major);
  return 0;
}

void pphc_cdev_exit(void)
{
  cdev_del(&cdev_st);
  unregister_chrdev_region(MKDEV(cdev_major, 0), 1);
}


static int pphc_cdev_open(struct inode *inode, struct file *filep)
{
  pr_info("depftom_init: open\n");
  return 0;
}

static int pphc_cdev_release(struct inode *inode, struct file *filep)
{
  return 0;
}

static ssize_t pphc_cdev_read(struct file *filep, char __user *buf,
				 size_t count, loff_t *offset)
{
  char tmp_buf[8];
  
  if (count > 8) {
    count = 8;
  }

  if(copy_to_user(buf, tmp_buf, count)) {
    kfree(tmp_buf);
    return -EFAULT;
  }

  kfree(tmp_buf);
  return count;
}

static struct file_operations depftom_fops = {
  .open      = pphc_cdev_open,
  .release   = pphc_cdev_release,
  .read      = pphc_cdev_read,
};
