#!/bin/bash
#cd ~/ParlayANN/algorithms/vamana

# ./neighbors -R 64 -L 128 -data_type uint8 -dist_func Euclidian -base_path $P/base.1B.u8bin.crop_nb_1000000
# PARLAY_NUM_THREADS=1
# ./neighbors -R 64 -L 128 -alpha 1.2 -two_pass 1 -data_type uint8 -dist_func Euclidian -query_path $P/query.public.10K.u8bin -gt_path $P/bigann-1M -res_path test.csv -base_path $P/base.1B.u8bin.crop_nb_1000000
# ./neighbors -R 64 -L 128 -alpha 1.2 -data_type uint8 -dist_func Euclidian -graph_path $P/graph-10M -query_path $P/query.public.10K.u8bin -gt_path $P/bigann-10M -res_path test.csv -base_path $P/base.1B.u8bin.crop_nb_10000000
# ./neighbors -R 64 -L 128 -a 1.2 -data_type uint8 -dist_func Euclidian -query_path /ssd1/data/bigann/query.public.10K.u8bin -gt_path /ssd1/data/bigann/bigann-1M -res_path test.csv -base_path /ssd1/data/bigann/base.1B.u8bin.crop_nb_1000000

make

PROJECT_ROOT=/home/nawat/muic/senior/ParlayANN
DATA_ROOT=/home/nawat/muic/senior/ParlayANN/data/sift
DATA_TYPE=float
K=100

#echo $1 $2

if [ -n "$1" ]; then
  DATA_ROOT=/home/nawat/muic/senior/ParlayANN/data/$1
fi
if [ -n "$2" ]; then
  DATA_TYPE=$2
fi
if [ -n "$3" ]; then
  K=$3
fi
#echo "$DATA_ROOT"
#echo "$DATA_TYPE"

echo "Converting base to binary"
if [ ! -f "$DATA_ROOT/base.bin" ]; then
  $PROJECT_ROOT/data_tools/vec_to_bin "$DATA_TYPE" \
    "$DATA_ROOT"/base.fvecs \
    "$DATA_ROOT"/base.bin
fi
echo "Done"
echo ""

echo "Converting query to binary"
if [ ! -f "$DATA_ROOT/query.bin" ]; then
  $PROJECT_ROOT/data_tools/vec_to_bin "$DATA_TYPE" \
    "$DATA_ROOT"/query.fvecs \
    "$DATA_ROOT"/query.bin
fi
echo "Done"
echo ""

echo "Computing ground truth"
if [ ! -f "$DATA_ROOT"/gt.bin ]; then
  $PROJECT_ROOT/data_tools/compute_groundtruth \
    -base_path "$DATA_ROOT"/base.bin \
    -query_path "$DATA_ROOT"/query.bin \
    -data_type "$DATA_TYPE" \
    -k "$K" \
    -dist_func Euclidian \
    -gt_path "$DATA_ROOT"/gt.bin
fi
echo "Done"
echo ""

echo "Running neighbors"
./neighbors \
  -R 64 \
  -L 128 \
  -alpha 1.0 \
  -data_type float \
  -two_pass 0 \
  -dist_func Euclidian \
  -query_path "$DATA_ROOT"/query.bin \
  -gt_path "$DATA_ROOT"/gt.bin \
  -res_path test.csv \
  -base_path "$DATA_ROOT"/base.bin
