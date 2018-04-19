#include <linux/printk.h>
#include <uapi/asm-generic/errno-base.h>

#include "phc.h"


#define DRIVER_ENTRY_NUM 10

struct phc_driver {
  const char name[10];
  uint64_t (*read)(void *arg);
  void *arg;
};

static struct phc_driver *phc_drivers[DRIVER_ENTRY_NUM];
static int current_driver;

uint64_t phc_interface_read(void) {
  if (phc_drivers[current_driver] == NULL) {
    return 0;
  }
  return (*phc_drivers[current_driver]).read((*phc_drivers[current_driver]).arg);
}

int register_phc(struct phc_driver *ops) {
  int i;
  pr_info("calibrator: try to register phc(%s)\n", ops->name);
  for (i = 0; i < DRIVER_ENTRY_NUM; i++) {
    if (phc_drivers[i] == NULL) {
      phc_drivers[i] = ops;
      if (current_driver == -1) {
        current_driver = i;
      }
      pr_info("calibrator: register phc(%s)\n", ops->name);
      pr_info("ptp: count: %llx\n", ops->read(ops->arg));
      return 0;
    }
  }
  return -ENOMEM;
}

int unregister_phc(struct phc_driver *ops) {
  int i, j;
  for (i = 0; i < DRIVER_ENTRY_NUM; i++) {
    if (phc_drivers[i] == ops) {
      phc_drivers[i] = NULL;
      if (current_driver == i) {
        current_driver = -1;
	pr_info("calibrator: unregister phc(%s)\n", ops->name);
        for (j = 0; j < DRIVER_ENTRY_NUM; j++) {
          if (phc_drivers[j] != NULL) {
            current_driver = j;
          }
        }
      }
      return 0;
    }
  }
  return -ENOENT;
}

int phc_interface_init(void) {
  int i;
  current_driver = -1;
  for (i = 0; i < DRIVER_ENTRY_NUM; i++) {
    phc_drivers[i] = NULL;
  }
  return 0;
}

EXPORT_SYMBOL(register_phc);
EXPORT_SYMBOL(unregister_phc);
