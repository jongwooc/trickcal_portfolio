#!/bin/bash

export CGEN_PATH=`pwd`

if [ $# -lt 1 ]; then
    echo "please input the name of the solver.prototxt you want to generate."
    exit 0
fi

python caffe_parser/caffe_parser.py $1

python main.py -c -p caffe_parser/pd_output/parsed_pd.data -n caffe_parser/cfg_output/parsed_cfg.cfg && make && time ./run_dnn
