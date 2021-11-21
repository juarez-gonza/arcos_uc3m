#!/usr/bin/sh

argc=`expr $# + 1`
sim_args=()

pushd ./build > /dev/null

for val in $@
do
	if [ $val != $0 ]
	then
		sim_args+="$val "
	fi
done

./sim-soa $sim_args

popd > /dev/null
