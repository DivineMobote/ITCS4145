#!/bin/bash
#SBATCH -J nbody-bench
#SBATCH -p Centaurus
#SBATCH --time=06:00:00
#SBATCH --cpus-per-task=1
#SBATCH --mem=2G
#SBATCH -o logs/%x_%A_%a.out
#SBATCH -e logs/%x_%A_%a.err
#SBATCH --array=0-2

# Environment setup for results
module load gcc || true
cd "$SLURM_SUBMIT_DIR"
mkdir -p logs

# Job cases
case $SLURM_ARRAY_TASK_ID in
  # Solar system snapshot benchmar
  0) /usr/bin/time -v ./nbody solar.tsv 200 5000000 5000000 --out logs/solar_bench.tsv ;;
  # Random 100-particle system
  1) /usr/bin/time -v ./nbody 100 1 10000 10000 --out logs/rand100_bench.tsv ;;
  # Random 1000-particle system
  2) /usr/bin/time -v ./nbody 1000 1 10000 10000 --out logs/rand1000_bench.tsv ;;
esac
