#!/usr/bin/bash

# configuracion constante por consigna.
LINESIZE=64
NWAY=8

# tamaños en KiB
#L1I=$2 no especifica el enunciado
L1D=$((1024*${1}))
LL=$((1024*${2}))

# path absoluto de ejecutable target
target_path=$(realpath $3)

# path absoluto del directorio de salida
script_dir=$(echo ${0} | awk -F "/" '{for(i=1;i<NF;i++){printf "%s/", $i}}')
script_dir_abs=$(realpath ${script_dir})"/"
out_path_base=${script_dir_abs}"/"$(echo $target_path | awk -F "/" '{print $NF}')
mkdir ${out_path_base}

# path absoluto de archivos de salida (uno para cachegrind y otro para cg_annotate)
out_path_cg="${out_path_base}/${L1D}.cg"
out_path_anotate="${out_path_base}/${L1D}.acg"

# valgrind
valgrind --tool=cachegrind \
--LL=${LL},${NWAY},${LINESIZE} \
--D1=${L1D},${NWAY},${LINESIZE} \
--cachegrind-out-file=${out_path_cg} \
${target_path} && \
cg_annotate ${out_path_cg} --auto=yes > ${out_path_anotate}
