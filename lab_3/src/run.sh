#!/usr/bin/bash

echo -e "counter:\n $(./build/counter $1)"
echo -e "counter_mutex:\n $(./build/counter_mutex $1)"
echo -e "counter_atomic:\n $(./build/counter_atomic $1)"
echo -e "counter_spin_ra:\n $(./build/counter_spin_ra $1)"
echo -e "counter_spin_ra_opt:\n $(./build/counter_spin_ra_opt $1)"
echo -e "counter_spin_seq:\n $(./build/counter_spin_seq $1)"
echo -e "counter_spin_seq_opt:\n $(./build/counter_spin_seq_opt $1)"
