#include <linux/module.h>
#include "cdev.h"
#include "phc.h"

MODULE_LICENSE("GPL");

static int pphc_init(void) {
  if (phc_interface_init() < 0) {
    return -1;
  }
  if (pphc_cdev_init() < 0) {
    return -1;
  }
  return 0;
}

static void pphc_exit(void) {
  pphc_cdev_exit();
}

module_init(pphc_init);
module_exit(pphc_exit);

