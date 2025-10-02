#!/bin/bash
#SBATCH -J bfs-bench
#SBATCH -p Centaurus
#SBATCH --time=01:00:00
#SBATCH --cpus-per-task=8
#SBATCH --mem=2G
#SBATCH -o logs/%x_%j.out
#SBATCH -e logs/%x_%j.err
set -euo pipefail

cd "$SLURM_SUBMIT_DIR"

# Help: to ensure job is runs in the same folder as binaries
if [[ ! -x ./bfs_par || ! -x ./bfs_seq ]]; then
  echo "ERROR: run from the folder that has bfs_par and bfs_seq"
  echo "Current: $SLURM_SUBMIT_DIR"
  exit 1
fi

# Help: Unique output dir each run, so files never clobber each other 
RUNID=$(date +%Y%m%d-%H%M%S)
OUTDIR="results/$RUNID"
mkdir -p "$OUTDIR" logs

echo "Running Parallel benchmarks..."
for t in 1 2 4 8; do
  echo "---- Threads: $t ----"
  /usr/bin/time -f "threads=$t elapsed %E" -o "$OUTDIR/par_d4_t${t}.time" \
    ./bfs_par "Tom Hanks" 4 $t > "$OUTDIR/par_d4_t${t}.txt" 2>> "$OUTDIR/par_d4_t${t}.time"

  # Help: quick sanity append (show last 3 lines so I know the file is complete) 
  { echo "--- tail par_d4_t${t}.txt ---"; tail -n 3 "$OUTDIR/par_d4_t${t}.txt"; } >> "$OUTDIR/check.log"
done

# Save “main case” aliases
cp "$OUTDIR/par_d4_t8.txt"  "$OUTDIR/par_maincase_d4_t8.txt"
cp "$OUTDIR/par_d4_t8.time" "$OUTDIR/par_maincase_d4_t8.time"

echo "Running Sequential benchmark..."
/usr/bin/time -f "seq elapsed %E" -o "$OUTDIR/seq_d4.time" \
  ./bfs_seq "Tom Hanks" 4 > "$OUTDIR/seq_d4.txt" 2>> "$OUTDIR/seq_d4.time"
{ echo "--- tail seq_d4.txt ---"; tail -n 3 "$OUTDIR/seq_d4.txt"; } >> "$OUTDIR/check.log"

echo "Building timing summary..."
{
  echo 'Start="Tom Hanks", Depth=4'
  echo '-----------------------------------'
  echo 'Parallel Runs'
  cat "$OUTDIR/par_d4_t1.time"
  cat "$OUTDIR/par_d4_t2.time"
  cat "$OUTDIR/par_d4_t4.time"
  cat "$OUTDIR/par_d4_t8.time"
  echo
  echo 'Sequential Run'
  cat "$OUTDIR/seq_d4.time"
  echo
  echo 'Visited Counts'
  grep -m1 "Visited:" "$OUTDIR/par_d4_t1.txt" || true
  grep -m1 "Visited:" "$OUTDIR/par_d4_t2.txt" || true
  grep -m1 "Visited:" "$OUTDIR/par_d4_t4.txt" || true
  grep -m1 "Visited:" "$OUTDIR/par_d4_t8.txt" || true
  grep -m1 "Visited:" "$OUTDIR/seq_d4.txt"    || true
} > "$OUTDIR/timings.txt"

# Copy SLURM stdout into the same run folder
cp "logs/${SLURM_JOB_NAME}_${SLURM_JOB_ID}.out" "$OUTDIR/slurm_bench_output.txt" 2>/dev/null || true

echo

