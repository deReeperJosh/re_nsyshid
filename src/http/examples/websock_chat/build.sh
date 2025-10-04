#!/bin/bash

initialwd=$PWD

if [ ! -d ../MiniJson ]; then
    cd ..
    git clone https://github.com/zsmj2017/MiniJson
    cd MiniJson
    cmake .
    make -j
    cd $initialwd
fi

mkdir -p build
make
