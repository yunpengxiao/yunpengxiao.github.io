#!/bin/bash

echo dynamic_iostream.cc
clang++ dynamic_iostream.cc -std=c++14 -Wall -o d_iostream && ./d_iostream && rm d_iostream
echo

echo static_iostream.cc
clang++ static_iostream.cc -std=c++14 -Wall -o s_iostream && ./s_iostream && rm s_iostream
echo

echo raw_json.cc
clang++ raw_json.cc -std=c++14 -o raw_json && ./raw_json && rm raw_json
echo

echo reflection_json.cc
clang++ reflection_json.cc -std=c++14 -o ref_json && ./ref_json && rm ref_json

