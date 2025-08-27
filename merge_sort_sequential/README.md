# Merge Sort – Sequential (C++)

This project implements a sequential merge sort in C++ for ITCS 4145 course. The goal is to sort an array of integers and benchmark the performance on the Centaurus HPC cluster.

---

## Author 
Divine Mobote
Course: UNC Charlotte - ITCS 4145 Parallel Programming 

## Description

- Algorithm: Merge Sort
- Approach: Sequential
- Language: C++
- Array: Filled with random integers
- Input: Array size passed as a command-line argument
- Output: Time taken to sort (in seconds)

---

## Files (update later!)
- mergesort.cpp – Merge sort implementation
- Makefile - For compiling the program
- README - This documentation


---

## How to Compile
To build the program locally or on Centaurus:

```bash
make
```

To clean compile:
```bash
make clean
```

---

## How to Run
Run the program with desired array size:
```bash
./mergesort <array-size>
```
Example: 
```bash
./mergesort 20000
```
Output format:
20000,0.00512


---

## Running on Centaurus 

SSH into Centaurus:
```bash
ssh yourUNCCusername@centaurus.uncc.edu
```

Step 1: Clone the Git repo or transfer your files
```bash
git clone https://github.com/DivineMobote/ITCS4145.git
```

Step 2: Compile on Centaurus
cd merge_sort_sequential
```bash
make
```

Step 3: Run the SLURM job
Submit your SLURM script
```bash
sbatch run.slurm
```

---

## Benchmark Instructions
- The benchmark tests were run on input sizes of all powers of 10.
- Each run will print the size and time to output file
- Each output will be stored into result file as size,time

---

## Plotting the Results
Use Python + matplotlib to generate a log-log chart:
```python
import matplotlib.pyplot as plt
import pandas as pd

data = pd.read_csv('results.csv', header=None, names=['size', 'time'])

plt.figure(figsize=(10,6))
plt.plot(data['size'], data['time'], marker='o')
plt.xscale('log')
plt.yscale('log')
plt.xlabel('Array Size (log scale)')
plt.ylabel('Time (seconds, log scale)')
plt.title('Merge Sort Sequential Benchmark')
plt.grid(True)
plt.savefig('plot.png')
plt.show()
```

---

## Performance Reflection






