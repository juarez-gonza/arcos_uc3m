import scipy.stats as st
import numpy as np
import re

int_re = re.compile(r"[0-9]+")

if __name__ == "__main__":
    data_sample_a = {}
    dict_key = ""
    num_objects = 0
    num_iterations = 0

    with open("./aos/samples", "r") as f:
        for line in f:
            if "num_objects" in line:
                num_objects, num_iterations = int_re.findall(line)
                dict_key = "%s-%s" % (num_objects, num_iterations)
                data_sample_a[dict_key] = []
                continue
            data_sample_a[dict_key].append(float(line))

    data_sample_b = {}

    with open("./soa/samples", "r") as f:
        for line in f:
            if "num_objects" in line:
                num_objects, num_iterations = int_re.findall(line)
                dict_key = "%s-%s" % (num_objects, num_iterations)
                data_sample_b[dict_key] = []
                continue
            data_sample_b[dict_key].append(float(line))

    with open("kruskal_result", "w") as f:
        for key in data_sample_b.keys():
            mean_result = st.kruskal(data_sample_a[key], data_sample_b[key])
            f.write("para num_objects-num_iterations = %s:\n\tp_valor: %f, %s y %s es significativamente más rápido\n\t\tmedia aos: %f, media soa: %f\n"
                    % (key, mean_result.pvalue,
                        "no hay diferencia" if mean_result.pvalue > 0.05 else "si hay diferencia",
                        "aos" if np.mean(data_sample_b[key]) > np.mean(data_sample_a[key]) else "soa",
                        np.mean(data_sample_a[key]), np.mean(data_sample_b[key])
                        ))

    with open("anova_result", "w") as f:
        for key in data_sample_b.keys():
            mean_result = st.f_oneway(data_sample_a[key], data_sample_b[key])
            f.write("para num_objects-num_iterations = %s:\n\tp_valor: %f, %s y %s es significativamente más rápido\n\t\tmedia aos: %f, media soa: %f\n"
                    % (key, mean_result.pvalue,
                        "no hay diferencia" if mean_result.pvalue > 0.05 else "si hay diferencia",
                        "aos" if np.mean(data_sample_b[key]) > np.mean(data_sample_a[key]) else "soa",
                        np.mean(data_sample_a[key]), np.mean(data_sample_b[key])
                        ))
