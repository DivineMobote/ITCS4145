#!/bin/bash
#SBATCH -J bfs-web
#SBATCH -p Centaurus
#SBATCH --time=00:15:00
#SBATCH --cpus-per-task=1
#SBATCH --mem=2G
#SBATCH -o logs/%x_%j.out
#SBATCH -e logs/%x_%j.err

module load gcc || true
cd "$SLURM_SUBMIT_DIR"
mkdir -p logs

# Build
make RAPIDJSON_INC=$HOME/rapidjson/include

# Run BFS (1,2,3) and save output + timing logs
/usr/bin/time -f "%E real  %M KB" ./bfs_web "Tom Hanks" 1 -o logs/out_d1.txt | tee logs/run_d1.log
/usr/bin/time -f "%E real  %M KB" ./bfs_web "Tom Hanks" 2 -o logs/out_d2.txt | tee logs/run_d2.log
/usr/bin/time -f "%E real  %M KB" ./bfs_web "Tom Hanks" 3 -o logs/out_d3.txt | tee logs/run_d3.log
