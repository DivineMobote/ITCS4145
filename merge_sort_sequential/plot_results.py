import matplotlib.pyplot as plt
import csv

x = []
y = []

with open('results.csv', 'r') as file:
    reader = csv.reader(file)
    for row in reader:
        size, time = row
        x.append(int(size))
        y.append(float(time))

plt.figure(figsize=(8, 6))
plt.plot(x, y, marker='o', linestyle='-', color='purple')
plt.xlabel('Input Size (n)', fontsize=12)
plt.ylabel('Execution Time (seconds)', fontsize=12)
plt.title('Merge Sort Execution Time vs Input Size', fontsize=14)
plt.xscale('log')
plt.yscale('log')
plt.grid(True, which="both", ls="--")
plt.tight_layout()
plt.savefig("merge_sort_plot.png", dpi=300)
plt.show()
