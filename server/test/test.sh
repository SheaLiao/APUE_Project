#!/bin/bash 

make

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:`pwd`/../lib/ 

./server -p 12345
