import sys
from scipy import stats as st
import numpy

def get_data(filenames):
    file_data = {}
    num_objects = 0
    num_iterations = 0
    for filename in filenames:
        file_data[filename] = {}
        with open(filename) as f:
            for line in f:
                if "num_objects" in line:
                    num_objects = int(line.split(" ")[1][:-1])
                    num_iterations = int(line.split(" ")[-1][:-1])
                    if num_objects not in file_data[filename].keys():
                        file_data[filename][num_objects] = {}
                    file_data[filename][num_objects][num_iterations] = []
                    continue
                file_data[filename][num_objects][num_iterations].append(float(line))
    return file_data

def kruskal_wallis(filenames, file_data):
    tests = [(4000,250), (4000,500), (8000,250), (8000,500)]
    res = {}

    for t in tests:
        kruskal_args = []
        for f in filenames:
            kruskal_args.append(file_data[f][t[0]][t[1]])
        res[t] = st.kruskal(*kruskal_args)

    print("Resultados Kruskal-Wallis H-Test:")
    for k in res:
        print("\nConfiguración: num_iterations: %d, num_objects: %d, p_valor: %.12f" % (k[0], k[1], res[k].pvalue))
        if res[k].pvalue > 0.05:
            print("\tNo hay diferencia significativa en las medias de las muestras")
        else:
            print("\tSí hay diferencia significativa en las medias de las muestras")

def t_test(sample_a, sample_b, filename_a, filename_b):
    tests = [(4000,250), (4000,500), (8000,250), (8000,500)]
    res = {} #st.ttest_ind(a=sample_a,b=sample_b,equal_var=True)

    for t in tests:
        res[t] = st.ttest_ind(a=sample_a[t[0]][t[1]],b=sample_b[t[0]][t[1]],equal_var=True)

    print("Resultados T-Test:")
    for k in res:
        print("\nConfiguración: num_iterations: %d, num_objects: %d, p_valor: %.12f" % (k[0], k[1], res[k].pvalue))
        if res[k].pvalue > 0.05:
            print("\tNo hay diferencia significativa en las medias de las muestras")
        else:
            print("\tSí hay diferencia significativa en las medias de las muestras")
            mean_a = numpy.mean(sample_a[k[0]][k[1]])
            mean_b = numpy.mean(sample_b[k[0]][k[1]])
            print("\t\tmedia %s: %f, media %s: %f" % (filename_a, mean_a, filename_b, mean_b))
            if mean_a > mean_b:
                print("\t\t\tSpeedup = %f" % (mean_a / mean_b))
            else:
                print("\t\t\tSpeedup = %f" % (mean_b / mean_a))

if __name__ == "__main__":
    filenames = sys.argv[1:]
    file_data = get_data(filenames)
    kruskal_wallis(filenames, file_data)
    print("\n")
    t_test(file_data[filenames[0]], file_data[filenames[1]], filenames[0], filenames[1])
