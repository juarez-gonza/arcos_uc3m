import os
import sys
import subprocess as sp

random_seed = 666
size_enclosure = 1000.0
time_step = 0.1

# 30 muestras por caso para poder usar
# distribucion normal para armar intervalo de confianza
# en estimacion de medias.
sample_size = 30

def cleanup_dir():
    pwd = os.getcwd()
    os.remove(os.path.join(pwd, "final_config.txt"))
    os.remove(os.path.join(pwd, "init_config.txt"))

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
    if exe_file != "":
        with open("samples", "w") as f:
            for num_objects in [1000, 2000, 4000]:
                for num_iterations in [50, 100, 200]:
                    f.write("num_objects: %d, num_iterations: %d\n" % (num_objects, num_iterations))
                    for count in range(sample_size):
                        p = run_cmd("perf stat %s %d %d %d %f %f"
                            % (exe_file, num_objects, num_iterations,
                                random_seed, size_enclosure, time_step))

                        lines = p.stdout.split('\n')
                        systime = lines[-4].split(' ')[-3]
                        usertime = lines[-5].split(' ')[-3]
                        totaltime = lines[-7].split(' ')[-4]

                        f.write("%s\n" % totaltime)
    else:
        print("usage: %s [executable]" % exe_file)

if __name__ == "__main__":
    print(sys.argv[1])
    run(sys.argv[1])
    cleanup_dir()
