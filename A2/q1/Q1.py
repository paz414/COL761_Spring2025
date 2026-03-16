import sys
import urllib.request, json
import numpy as np
from sklearn.cluster import KMeans
import matplotlib.pyplot as plt
def load_data(arg):
    if arg.isdigit():
        dataset_number= int(arg)
        url = f"http://10.208.23.248:3000/dataset?student_id=me1221438&dataset_num={dataset_number}"
        with urllib.request.urlopen(url) as response:
            raw = response.read().decode("utf-8")
            data = json.loads(raw)
        return np.array(data["X"])
    else:
        return np.load(arg)
def main():
    data = load_data(sys.argv[1])
    k_range = range(1,16)
    inertias = []
    for k in k_range:
        kmeans = KMeans(n_clusters=k,n_init=20)
        kmeans.fit(data)
        inertias.append(kmeans.inertia_)
    plt.plot(k_range, inertias, marker='o')
    plt.xlabel("k")
    plt.ylabel("Inertia")
    plt.savefig("plot.png")

    ks = np.array(list(k_range))
    inertias = np.array(inertias)
    p1 = np.array([ks[0], inertias[0]])
    p2 = np.array([ks[-1], inertias[-1]])

    distances = []
    for i in range(len(ks)):
        p = np.array([ks[i], inertias[i]])
        dist = abs((p2[0]-p1[0])*(p1[1]-p[1]) - (p1[0]-p[0])*(p2[1]-p1[1])) / np.linalg.norm(p2-p1)        
        distances.append(dist)

    optimal_k = ks[np.argmax(distances)]
    print(optimal_k)
if __name__ == "__main__":
    main()
