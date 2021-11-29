#!/usr/bin/sh

pushd build/

rm -rf *

if [ "$1" = "d" ]; then
	cmake ../ -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_CXX_FLAGS="-fsanitize=thread"
else
	cmake ../ -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_CXX_FLAGS="-fsanitize=thread"
fi

make

popd
