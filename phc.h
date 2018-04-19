#pragma once

#include <linux/types.h>

extern int phc_interface_init(void);
extern uint64_t phc_interface_read(void);
