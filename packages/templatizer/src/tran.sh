#!/bin/sh

for i in $*
do
	echo file $i
	mv $i ${i}.bak
	sed "s/rsl2cpp/templatizer/g" ${i}.bak > $i
done
