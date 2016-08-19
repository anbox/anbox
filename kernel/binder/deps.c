#include <linux/sched.h>
#include <linux/file.h>
#include <linux/fdtable.h>
#include <linux/atomic.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "deps.h"

static struct vm_struct *(*get_vm_area_ptr)(unsigned long, unsigned long) = GET_VM_AREA;
static void (*zap_page_range_ptr)(struct vm_area_struct *, unsigned long, unsigned long, struct zap_details *) = ZAP_PAGE_RANGE;
static int (*map_kernel_range_noflush_ptr)(unsigned long start, unsigned long size, pgprot_t prot, struct page **pages) = MAP_KERNEL_RANGE_NOFLUSH;
static void (*unmap_kernel_range_ptr)(unsigned long, unsigned long) = UNMAP_KERNEL_RANGE;
static struct files_struct *(*get_files_struct_ptr)(struct task_struct *) = GET_FILES_STRUCT;
static void (*put_files_struct_ptr)(struct files_struct *) = PUT_FILES_STRUCT;
static struct sighand_struct *(*__lock_task_sighand_ptr)(struct task_struct *, unsigned long *) = __LOCK_TASK_SIGHAND;
static int (*__alloc_fd_ptr)(struct files_struct *files, unsigned start, unsigned end, unsigned flags) = __ALLOC_FD;
static void (*__fd_install_ptr)(struct files_struct *files, unsigned int fd, struct file *file) = __FD_INSTALL;
static int (*__close_fd_ptr)(struct files_struct *files, unsigned int fd) = __CLOSE_FD;
static int (*can_nice_ptr)(const struct task_struct *, const int) = CAN_NICE;
static int (*security_binder_set_context_mgr_ptr)(struct task_struct *mgr) = SECURITY_BINDER_SET_CONTEXT_MGR;
static int (*security_binder_transaction_ptr)(struct task_struct *from, struct task_struct *to) = SECURITY_BINDER_TRANSACTION;
static int (*security_binder_transfer_binder_ptr)(struct task_struct *from, struct task_struct *to) = SECURITY_BINDER_TRANSFER_BINDER;
static int (*security_binder_transfer_file_ptr)(struct task_struct *from, struct task_struct *to, struct file *file) = SECURITY_BINDER_TRANSFER_FILE;

struct vm_struct *get_vm_area(unsigned long size, unsigned long flags)
{
	return get_vm_area_ptr(size, flags);
}

void zap_page_range(struct vm_area_struct *vma, unsigned long address, unsigned long size, struct zap_details *details)
{
	zap_page_range_ptr(vma, address, size, details);
}

int map_kernel_range_noflush(unsigned long start, unsigned long size, pgprot_t prot, struct page **pages)
{
	return map_kernel_range_noflush_ptr(start, size, prot, pages);
}

void unmap_kernel_range(unsigned long addr, unsigned long size)
{
	unmap_kernel_range_ptr(addr, size);
}

struct files_struct *get_files_struct(struct task_struct *task)
{
	return get_files_struct_ptr(task);
}

void put_files_struct(struct files_struct *files)
{
	put_files_struct_ptr(files);
}

struct sighand_struct *__lock_task_sighand(struct task_struct *tsk, unsigned long *flags)
{
	return __lock_task_sighand_ptr(tsk, flags);
}

int __alloc_fd(struct files_struct *files, unsigned start, unsigned end, unsigned flags)
{
	return __alloc_fd_ptr(files, start, end, flags);
}

void __fd_install(struct files_struct *files, unsigned int fd, struct file *file)
{
	__fd_install_ptr(files, fd, file);
}

int __close_fd(struct files_struct *files, unsigned int fd)
{
	return __close_fd_ptr(files, fd);
}

int can_nice(const struct task_struct *p, const int nice)
{
	return can_nice_ptr(p, nice);
}

int security_binder_set_context_mgr(struct task_struct *mgr)
{
	return security_binder_set_context_mgr_ptr(mgr);
}

int security_binder_transaction(struct task_struct *from, struct task_struct *to)
{
	return security_binder_transaction_ptr(from, to);
}

int security_binder_transfer_binder(struct task_struct *from, struct task_struct *to)
{
	return security_binder_transfer_binder_ptr(from, to);
}

int security_binder_transfer_file(struct task_struct *from, struct task_struct *to, struct file *file)
{
	return security_binder_transfer_file_ptr(from, to, file);
}
