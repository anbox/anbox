#!/bin/sh

SYMS="shmem_zero_setup"

for sym in $SYMS; do 
	addr=`cat /proc/kallsyms | grep -Ee '^[0-9a-f]+ T '$sym'$' | sed -e 's/\s.*$//g'`
	if [ a$addr = 'a' ]; then
		echo "Error: can't find symbol $sym"
		exit 1
	fi

	name=`echo $sym | tr '[:lower:]' '[:upper:]'`
	echo "#define $name\t(void *)0x$addr"
done
