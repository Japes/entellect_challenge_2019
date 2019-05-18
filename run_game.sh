#!/bin/bash

for file in $1/*/A*/Console/Console.txt
do
   clear
   cat "$file"
   sleep 0.1
done