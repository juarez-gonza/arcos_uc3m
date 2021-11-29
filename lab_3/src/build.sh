#!/usr/bin/sh

pushd build/

rm -rf *

if [ "$1" = "d" ]; then
	cmake ../ -DCMAKE_BUILD_TYPE=DEBUG
else
	cmake ../ -DCMAKE_BUILD_TYPE=RELEASE
fi


make

popd
