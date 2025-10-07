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

cd templates

for x in *.html; do
    ../../../htcc/htcc $x $x.hpp
done

cd ..

g++ -g -ggdb -std=c++17 ../../http.cpp demo.cpp ../MiniJson/Source/libJson.a -Itemplates -I../../htcc -I../.. -I ../MiniJson/Source/include -pthread -o tinyhttp_demo
