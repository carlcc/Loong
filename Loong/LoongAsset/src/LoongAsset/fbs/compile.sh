#!/bin/bash

for i in $(ls *.fbs); do
    flatc --cpp ${i} --gen-mutable --gen-object-api
done