import os
import re

import numpy as np
import scipy.stats as st
import seaborn as sns
from matplotlib import pyplot as plt
from matplotlib.axes import Subplot

int_re = re.compile(r"[0-9]+")

if __name__ == "__main__":
    data = {}
    dict_key = ""
    num_objects = 0
    num_iterations = 0

    with open("samples", "r") as f:
        for line in f:
            if "num_objects" in line:
                num_objects, num_iterations = int_re.findall(line)
                dict_key = "%s-%s" % (num_objects, num_iterations)
                data[dict_key] = []
                continue
            data[dict_key].append(float(line))

    conf_interval = {}
    means = []
    for k in data.keys():
        means.append(np.mean(data[k]))
        conf_interval[k] = st.norm.interval(alpha=0.95,
                loc=means[-1], scale=st.sem(data[k]))

    num_obj_x = [int(k.split("-")[0]) for k in data.keys()]
    num_iter_y = [int(k.split("-")[1]) for k in data.keys()]
    fig = plt.figure(figsize=(6, 6))
    ax = fig.add_subplot(111, projection="3d")
    ax.set_title("Tiempo de ejecucion vs. Cantidad de objetos e iteraciones")
    ax.set_xlabel("num_objects")
    ax.set_ylabel("num_iterations")
    ax.set_zlabel("elapsed_time (seconds)")
    ax.scatter(num_obj_x, num_iter_y, means,
            linewidths=1, alpha=1, edgecolor='k', s=200)
    plt.savefig(os.path.join(os.getcwd(), "exe_vs_num_obj&iter.png"))

    fig, axes = plt.subplots(3, 3, figsize=(18,12), sharey=True, dpi=100)
    i = j = 0
    for k in data.keys():
        sns.histplot(data=data[k], kde=True, ax=axes[i, j])
        axes[i, j].set_title(k + " (num_obj, num_iter)")
        axes[i, j].axvline(conf_interval[k][0])
        axes[i, j].axvline(conf_interval[k][1])
        textstr = "mean: %f\nstdev: %f\ninterval: [%f, %f]"\
                % (np.mean(data[k]), np.std(data[k]), conf_interval[k][0], conf_interval[k][1])
        props = dict(boxstyle="round", facecolor="wheat", alpha=0.5)
        # los valorse para las coordenadas x e y del texto son obtenidos con prueba y error
        axes[i, j].text(0.63+(j*1.065) , 2.1 - 0.7*i, s=textstr, transform=ax.transAxes, fontsize=12,
                        verticalalignment='top', bbox=props)
        j = (j + 1) % 3
        if j == 0:
            i = (i + 1) % 3
    plt.savefig(os.path.join(os.getcwd(), "conf_interval.png"))
