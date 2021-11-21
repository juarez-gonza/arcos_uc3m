#!/usr/bin/sh

sim_args=()

pushd ./build > /dev/null

rm -rf *

cmake ../
make

popd > /dev/null
