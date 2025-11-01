#!/bin/bash
cd "$(dirname "$0")"
cd ../../..

mkdir build/
cmake -S . -B ./build/
cd ./build/
make && make Shaders && ./VRuuum
read -p "Press [Enter] to continue..."