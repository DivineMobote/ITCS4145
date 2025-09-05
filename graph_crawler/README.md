# BFS Web Crawler – C++

This project is a BFS (Breadth-First Search) program that works with the Bridges Hollywood Actor/Movie graph. The program talks to the Bridges web server, pulls neighbors through API calls, and shows all the nodes you can reach up to a certain depth.  

---

## Author
Divine Mobote
Professor Erik Saule  
UNC Charlotte – ITCS 4145 (Parallel Programming)
Date: 09/05/2025

---

## What the Program Does
- Traverses the Hollywood graph using BFS  
- Each node is an actor or movie
- Uses cURL to make requests and RapidJSON to parse the responses  
- Outputs which nodes were reached and at what distance (depth level)  

---

## Files in This Project
- `src/bfs_web.cpp` – main C++ code for BFS  
- `Makefile` – compile instructions  
- `script/bfs_job.sh` – job script for Centaurus (runs depth 1–3)  
- `logs_centaurus/` – outputs from running on Centaurus (`.out`, `.err`, and `.txt` files)  
- `local_outputs/` – sample runs I did locally  

---

## How to Compile

### Local (Mac/Linux):
```bash
make clean
make
```

### On Centaurus
```bash
module load gcc
make clean
make
```

---

## How to Run it

```bash
./bfs_web "<start node>" <depth> [-o output.txt]
```

Example: 
```bash
./bfs_web "Tom Hanks" 2 -o logs/tom_hanks_depth2.txt
```

### Running on Centaurus(SLURM):
```bash
sbatch script/bfs_job.sh
```

Check job status:
```bash
squeue -u $USER
```

Sample Output:

```sql
Start: "Tom Hanks"  depth: 1  reached: 40  time: 172.567 ms
node	dist
Tom Hanks	0
A Beautiful Day in the Neighborhood	1
A Hologram for the King	1
A Man Called Otto	1
Angels & Demons	1
... (20 more)
```

---

## Notes:
- Nodes are URL encoded, so spaces and special characters work fine.
- If the server gives an error (like HTTP 400), the program just skips that node.
- Bigger depths return a lot of results, so expect longer run times and larger files.


