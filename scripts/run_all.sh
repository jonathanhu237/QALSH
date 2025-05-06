#!bin/bash

set -e

# Compile
cd methods
make clean
make -j $(nproc)
cd ..

# Create logs directory
mkdir -p logs

# Run
bash scripts/run_sift.sh > logs/sift.log 2>&1 &
bash scripts/run_gist.sh > logs/gist.log 2>&1 &
bash scripts/run_p53.sh > logs/p53.log 2>&1 &
bash scripts/run_trevi.sh > logs/trevi.log 2>&1 &

# Wait for all jobs to finish
echo "Waiting for all jobs to finish..."
wait
echo "Done!"
