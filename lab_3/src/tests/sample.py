import os
import sys
import subprocess as sp

import numpy as np

# 10 muestras por caso para poder usar
# distribucion normal para armar intervalo de confianza
# en estimacion de medias.
sample_size = 15
num_threads = [2, 4, 8, 16]

def run_cmd(cmd):
    return sp.run(
        cmd,
        shell=True,
        stdout=sp.PIPE,
        stderr=sp.STDOUT,
        encoding="utf8")

#p = {
#        returncode : int
#        stdout : bytes
#        args : str
#    }
def run(exe_file):
    out_file = exe_file.split("/")[-1]
    with open(("samples_%s" % out_file), "w") as f:
        f.write("num_threads: mean, stdev\n")
        for n_thr in num_threads:
            count_samples = []
            time_us_samples = []
            for i in range(sample_size):
                print("n_threads: %d, iteracion: %d" % (n_thr, i))
                p = run_cmd("./%s %d"
                    % (exe_file, n_thr))
                lines = p.stdout.split("\n")
                count_samples.append(float(lines[0].split(" = ")[1]))
                time_us_samples.append(float(lines[1].split(" = ")[1].split(" ")[0]))
            f.write("%d: %f, %f\n" % (n_thr, np.mean(time_us_samples), np.std(time_us_samples)))

if __name__ == "__main__":
    print(sys.argv[1])
    run(sys.argv[1])
