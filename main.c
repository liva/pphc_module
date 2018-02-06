#include <linux/module.h>
#include "cdev.h"

MODULE_LICENSE("GPL");

struct phc_driver {
  const char name[10];
  uint64_t (*read)(void *arg);
  void *arg;
};

struct phc_driver_container {
  struct phc_driver *phc;
  uint64_t tsc_init;
  uint64_t phc_init;
};

struct time_container {
  uint64_t time;
  uint32_t pid;
};

static inline struct time_container pphc_rdtsc(void) {
  uint32_t a,c,d;
  struct time_container container;

  asm volatile("rdtscp" : "=a" (a), "=d" (d), "=c"(c));
  container.time = ((((uint64_t)a) | (((uint64_t)d) << 32)) * 10) / 33;
  container.pid = c;
  return container;
}

#define DRIVER_ENTRY_NUM 10

static struct phc_driver_container phc_drivers[DRIVER_ENTRY_NUM];
static int current_driver;

int register_phc(struct phc_driver *ops) {
  int i;
  printk(KERN_DEBUG "calibrator: try to register phc(%s)\n", ops->name);
  for (i = 0; i < DRIVER_ENTRY_NUM; i++) {
    if (phc_drivers[i].phc == NULL) {
      phc_drivers[i].phc = ops;
      phc_drivers[i].phc_init = ops->read(ops->arg);
      phc_drivers[i].tsc_init = pphc_rdtsc().time;
      if (current_driver == -1) {
        current_driver = i;
      }
      printk(KERN_DEBUG "calibrator: register phc(%s)\n", ops->name);
      printk(KERN_DEBUG "ptp: count: %llx\n", ops->read(ops->arg));
      return 0;
    }
  }
  return -ENOMEM;
}

int unregister_phc(struct phc_driver *ops) {
  int i, j;
  for (i = 0; i < DRIVER_ENTRY_NUM; i++) {
    if (phc_drivers[i].phc == ops) {
      phc_drivers[i].phc = NULL;
      if (current_driver == i) {
        current_driver = -1;
        printk(KERN_DEBUG "calibrator: unregister phc(%s)\n", ops->name);
        for (j = 0; j < DRIVER_ENTRY_NUM; j++) {
          if (phc_drivers[j].phc != NULL) {
            current_driver = j;
          }
        }
      }
      return 0;
    }
  }
  return -ENOENT;
}

/* static int proc_do_count(struct ctl_table *table, int write, */
/*                            void __user *buffer, size_t *lenp, loff_t *ppos) */
/* { */
/*         struct ctl_table fake_table; */
/* 	uint64_t count1; */
/* 	if (current_driver == -1 || phc_drivers[current_driver].phc == NULL) { */
/* 	  count1 = 0; */
/* 	} else { */
/* 	  count1 = phc_drivers[current_driver].phc->read(phc_drivers[current_driver].phc->arg); */
/* 	} */
/*         fake_table.maxlen = sizeof(uint64_t); */
/*         fake_table.data = &count1; */

/*         return proc_doulongvec_minmax(&fake_table, write, buffer, lenp, ppos); */
/* } */

/* static struct ctl_table pphc_table[] = { */
/*         { */
/*          .procname = "count", */
/*          .data = NULL, */
/*          .maxlen = sizeof(uint64_t), */
/*          .mode = 0444, */
/*          .proc_handler = proc_do_count, */
/*          }, */
/* }; */

/* static struct ctl_table pphc_root[] = { */
/*         { */
/*          .procname = "pphc", */
/*          .maxlen = 0, */
/*          .mode = 0555, */
/*          .child = pphc_table, */
/*          }, */
/*         {} */
/* }; */

/* static struct ctl_table dev_root[] = { */
/*         { */
/*          .procname = "dev", */
/*          .maxlen = 0, */
/*          .mode = 0555, */
/*          .child = pphc_root, */
/*          }, */
/*         {} */
/* }; */

/* static struct ctl_table_header *sysctl_header; */

static int calibrator_init(void) {
  int i;
  current_driver = -1;
  for (i = 0; i < DRIVER_ENTRY_NUM; i++) {
    phc_drivers[i].phc = NULL;
  }
  if (pphc_cdev_init() < 0) {
    return -1;
  }
  return 0;
}

static void calibrator_exit(void) {
  pphc_cdev_exit();
}

EXPORT_SYMBOL(register_phc);
EXPORT_SYMBOL(unregister_phc);

module_init(calibrator_init);
module_exit(calibrator_exit);

