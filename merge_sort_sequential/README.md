# Merge Sort – Sequential (C++)

This project implements a sequential merge sort in C++ for ITCS 4145 Parallel Programming course. The goal is to sort an array of integers and benchmark the performance on the Centaurus HPC cluster.

---

## Author 
**Divine Mobote**
Course: UNC Charlotte - ITCS 4145 Parallel Programming 

---

## Description

- Algorithm: Merge Sort
- Approach: Sequential
- Language: C++
- Array: Filled with random integers
- Input: Array size passed as a command-line argument
- Output: Time taken to sort

---

## Files 
- mergesort.cpp – C++ merge sort implementation
- Makefile - For compiling the program
- results.csv - Output of benchmarks 
- plot_results.py - Python script to create the plot chart
- merge_sort_plot.png - Saved chart from matplotlib
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
./mergesort 10000
```
Output format:
10000,0.00512

---

## Running on Centaurus 

SSH into Centaurus:
```bash
ssh yourUNCCusername@hpc-student.uncc.edu
```

Step 1: Clone the Git repo  or transfer your files
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
- The benchmark tests were run on input sizes in powers of 10.
- Each run will print the size and execution time.
- Results will be stored into results.csv in the format: size, time

---

## Plotting the Results
A log-log of input size vs. executiom time was created using Python matplotlib

```bash
python3 plot_results.py
```

This creates and saves the plot as merge_sort_plot.png

---

## Performance Reflection

The log-log plot shows that the execution time grows with the size of the input array. 

The shape of the graph reflect: 
- Increase as the size scales
- This makes sense and aligns with the time complexity of merge sort.






