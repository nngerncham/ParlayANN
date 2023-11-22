#!/bin/bash
#cd ~/ParlayANN/algorithms/vamana
make

# ./neighbors -R 64 -L 128 -data_type uint8 -dist_func Euclidian -base_path $P/base.1B.u8bin.crop_nb_1000000
# PARLAY_NUM_THREADS=1
# ./neighbors -R 64 -L 128 -alpha 1.2 -two_pass 1 -data_type uint8 -dist_func Euclidian -query_path $P/query.public.10K.u8bin -gt_path $P/bigann-1M -res_path test.csv -base_path $P/base.1B.u8bin.crop_nb_1000000
# ./neighbors -R 64 -L 128 -alpha 1.2 -data_type uint8 -dist_func Euclidian -graph_path $P/graph-10M -query_path $P/query.public.10K.u8bin -gt_path $P/bigann-10M -res_path test.csv -base_path $P/base.1B.u8bin.crop_nb_10000000
# ./neighbors -R 64 -L 128 -a 1.2 -data_type uint8 -dist_func Euclidian -query_path /ssd1/data/bigann/query.public.10K.u8bin -gt_path /ssd1/data/bigann/bigann-1M -res_path test.csv -base_path /ssd1/data/bigann/base.1B.u8bin.crop_nb_1000000

PROJECT_ROOT=/home/nawat/muic/senior/ParlayANN
DATA_ROOT=/home/nawat/muic/senior/ParlayANN/data/sift

echo "Converting base to binary"
if [ ! -f "$DATA_ROOT/sift_base.fbin" ]; then
  $PROJECT_ROOT/data_tools/vec_to_bin float \
    $DATA_ROOT/sift_base.fvecs \
    $DATA_ROOT/sift_base.fbin
fi
echo "Done"
echo ""

echo "Converting query to binary"
if [ ! -f "$DATA_ROOT/sift_query.fbin" ]; then
  $PROJECT_ROOT/data_tools/vec_to_bin float \
    $DATA_ROOT/sift_query.fvecs \
    $DATA_ROOT/sift_query.fbin
fi
echo "Done"
echo ""

echo "Computing ground truth"
if [ ! -f "$DATA_ROOT/sift_gt.ibin" ]; then
  $PROJECT_ROOT/data_tools/compute_groundtruth \
    -base_path $DATA_ROOT/sift_base.fbin \
    -query_path $DATA_ROOT/sift_query.fbin \
    -data_type float \
    -k 100 \
    -dist_func Euclidian \
    -gt_path $DATA_ROOT/sift_gt.ibin
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
  -query_path $DATA_ROOT/sift_query.fbin \
  -gt_path $DATA_ROOT/sift_gt.ibin \
  -res_path test.csv \
  -base_path $DATA_ROOT/sift_base.fbin
