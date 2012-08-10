#!/bin/sh

X='./map/'

for filename in ./map/*.map
do
  ./bdragon -pre map/${filename#$X} scene/${filename#$X}.scen
done;
