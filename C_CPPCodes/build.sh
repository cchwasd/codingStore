#!/bin/bash

sudo rm -rf ./build

cmake -S ./ -B ./build  -LH
make