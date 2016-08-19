#include <linux/mm.h>

#include "deps.h"

static int (*shmem_zero_setup_ptr)(struct vm_area_struct *) = SHMEM_ZERO_SETUP;

int shmem_zero_setup(struct vm_area_struct *vma)
{
    return shmem_zero_setup_ptr(vma);
}
