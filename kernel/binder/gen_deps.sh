#!/bin/sh

SYMS="get_vm_area zap_page_range map_kernel_range_noflush unmap_kernel_range "\
"get_files_struct put_files_struct __lock_task_sighand "\
"__alloc_fd __fd_install __close_fd can_nice "\
"security_binder_set_context_mgr security_binder_transaction "\
"security_binder_transfer_binder security_binder_transfer_file"

for sym in $SYMS; do 
	addr=`cat /proc/kallsyms | grep -Ee '^[0-9a-f]+ T '$sym'$' | sed -e 's/\s.*$//g'`
	if [ a$addr = 'a' ]; then
		echo "Error: can't find symbol $sym"
		exit 1
	fi

	name=`echo $sym | tr '[:lower:]' '[:upper:]'`
	echo "#define $name\t(void *)0x$addr"
done
