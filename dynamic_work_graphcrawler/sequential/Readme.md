# Parallel BFS with Blocking Queue – Hollywood Graph Crawler

Author: Divine Mobote  
Semester: Fall 2025  
Course: Parallel Programming (ITCS 4145)
Date: 10/01/2025

---

## Description
This project implements a BFS  a traversal over the Hollywood actor–movie dataset using a web-based API.

Two versions are provided:
- Sequential BFS (`client_seq.cpp`): baseline implementation.
- Parallel BFS (`client_par.cpp`): uses a block queue and multiple threads for traversal

---

## API Information
- Service endpoint: http://hollywood-graph-crawler.bridgesuncc.org/neighbors/{node} 
- Example: curl -s http://hollywood-graph-crawler.bridgesuncc.org/neighbors/Tom%20Hanks

---

## File Structure
- logs/ 
    - bfs-bench_3521.out
    - bfs-bench_3521.err.
- results/
    - par_d4_t1.txt / .time
    - par_d4_t2.txt / .time
    - par_d4_t4.txt / .time
    - par_d4_t8.txt / .time
    - par_maincase_d4_t8.txt / .time
    - seq_d4.txt / .time
    - timings.txt
    - slurm_bench_output.txt
- client_par.cpp # Parallel BFS (Blocking Queue)
- client_seq.cpp # Sequential BFS
- Makefile # Builds both versions
- Readme.md # This file
- run_bench.sh # SLURM batch script to benchmark

---

## Build 
- Build:
```bash
make
```

- clean files:
```bash
make clean
```

---

## Usage:
- sequential:
```bash
./bfs_seq "Tom Hanks" 4
```

- Parallel BFS:
```bash
./bfs_par "Tom Hanks" 4 8
```

---

## SLURM BATCH Job (Benchmarking):
- Submit the job:
```bash
sbatch run_bench.sh
```

- Check job status:
```bash
squeue -u $USER
```

- View logs:
```bash
less logs/bfs-bench_<JOBID>.out
```

---

Results and Performance:
- Sample results: `results/timings.txt`
- Observations:
    - Correctness verified: Visited count = 29790 for all runs.
    - Speedup: Linear scaling as threads increase
    - Demonstrations of parallel BFS with blocking queue

