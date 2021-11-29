#!/usr/bin/bash

file_folder="../build/"

while read f; do
	if [ "$f" != "counter" ]; then
		echo $f
		python3 sample.py "$file_folder/$f"
	fi
done < <(ls $file_folder | grep counter)
