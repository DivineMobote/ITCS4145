# Parallel Level-by-Level BFS over Web API - ITCS 4145
Author: Divine Mobote  
Semester: Fall 2025  
Course: Parallel Programming (ITCS 4145)
Date: 09/22/2025

---

## Description
This program implements a level-synchronous breadth-first search (BFS) of a web-hosted graph. At the same level of the BFS tree, nodes are expanded simultaneously and the number of threads per level is limited.

- API endpoint: `GET http://hollywood-graph-crawler.bridgesuncc.org/neighbors/{node}`
- JSON response shape: `{ "neighbors": ["..."], "node": "..." }`

---

## Files
- **level_client.cpp** – Sequential BFS implementation (provided starter code).
- **par_level_client.cpp** – Parallel BFS implementation.
- **Makefile** – Builds both versions.
- **logs/** – Example outputs and timings from runs on Centaurus.
- **README.md** – Instructions (this file).

---

## Build
From the `sequential/` folder:
- Compile: 
```bash
make
```

- Clean:
```bash
make clean
```

---

## Run
- Run with first argument (start node) and, Second argument (depth).
- Examples: 

```bash
./level_client "Tom Hanks" 3
```

```bash
./par_level_client "Tom Hanks" 3 8
```

- Output Format:
```bash
- Tom Hanks
1
- Forrest_Gump
- Saving_Private_Ryan
- Cast_Away
3
...
Time to crawl: 0.83s
```

---

## Benchmarking
- You must run benchmarks to compare the sequential and parallel performance.
- A job script `run_bench.slurm` is include.
- Submit job: 
```bash
sbatch run_bench.slurm
```

---

## Logs and Timings
- `logs/seq_times.log`:

SEQ,Tom Hanks,3,82.022s
SEQ,Leonardo DiCaprio,3,48.9797s
SEQ,Meryl Streep,2,3.8561s
SEQ,Tom Hanks,2,3.75213s

- `logs/par_times_t8.log`:
PAR8,Tom Hanks,3,10.6535s
PAR8,Leonardo DiCaprio,3,6.51218s
PAR8,Meryl Streep,2,0.748025s
PAR8,Tom Hanks,2,0.743866s

---

## Performance Evaluation
- BFS scales correctly with different depths and starting nodes.
- Parallel BFS shows large speedups: 
   - Tom Hanks/ depth 3 was 7.7 times faster 
   - Leonardo Dicaprio / depth 3 was 7.5 faster
- Shallow depths also benefits (3.7s to 0.74s)
- The results confirm that multhreading improves performance while producing the same traveersal output.

---