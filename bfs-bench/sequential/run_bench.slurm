#!/bin/bash
#SBATCH -J bfs-bench
#SBATCH -p Centaurus
#SBATCH --time=00:20:00
#SBATCH --cpus-per-task=8
#SBATCH --mem=2G
#SBATCH -o %x_%j.out
#SBATCH -e %x_%j.err

set -euo pipefail
module load gcc || true
cd ~/bfs-bench/sequential
make -j
mkdir -p ../logs

# Define cases as node|depth 
CASES=(
  "Tom Hanks|3"
  "Leonardo DiCaprio|3"
  "Meryl Streep|2"
  "Tom Hanks|2"
)

# Sequential runs
: > ../logs/seq_times.log
for c in "${CASES[@]}"; do
  IFS="|" read -r name depth <<< "$c"
  ./level_client "$name" $depth | tee ../logs/seq_${name// /_}_$depth.log | \
    awk -v n="$name" -v d="$depth" '/Time to crawl:/{print "SEQ," n "," d "," $4}' \
    >> ../logs/seq_times.log
done

# Parallel runs (8 threads)
: > ../logs/par_times_t8.log
for c in "${CASES[@]}"; do
  IFS="|" read -r name depth <<< "$c"
  ./par_level_client "$name" $depth 8 | tee ../logs/par_t8_${name// /_}_$depth.log | \
    awk -v n="$name" -v d="$depth" '/Time to crawl:/{print "PAR8," n "," d "," $4}' \
    >> ../logs/par_times_t8.log
done
