#!/usr/bin/env bash

if [[ $# -ne 2 ]]; then
    echo "Not accecpt exactly two parameters."
    exit 1
fi

filesdir=$1
searchstr=$2

if [[ ! -d "$filesdir" ]]; then
    echo "${filesdir} is not a directory!"
    exit 1
fi

num_lines=$(find "$filesdir" -type f | wc -l)
matching_lines=$(grep -ir "$searchstr" "$filesdir" | wc -l)
echo "The number of files are ${num_lines} and the number of matching lines are ${matching_lines}"
