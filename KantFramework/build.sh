#!/bin/bash

function rebuild() {
  rm -rf build  
  mkdir build
  cd build
  cmake .. -DKANT_MYSQL=ON
  make -j4
}

$@
