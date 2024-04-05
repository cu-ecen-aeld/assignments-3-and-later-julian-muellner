#!/usr/bin/env bash

if [[ $# -ne 2 ]]; then
    echo "Exactly two arguments expected"
    exit 1
fi

writefile=$1
writestr=$2

mkdir -p $(dirname $writefile) && touch ${writefile} && echo ${writestr} > ${writefile}

if [[ $? -eq 1 ]]; then
    echo "File could not be created"
    exit 1
fi



