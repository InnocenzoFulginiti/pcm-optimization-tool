#!/bin/bash

echo "On File $1"

file=$1

gateType_query="select a2 as type, a4==\"-1\" as wasRemoved, a5.count(\",\") as beforeCount, a6.count(\",\") as afterCount, COUNT(NR) as count group by a2,a5.count(\",\"),a6.count(\",\"),a4==\"-1\""
gateType_output=${file%.csv}_gateType.csv

set -x

if test -f "$gateType_output"; then
    echo "$gateType_output already exists - skipped"
else
    rbql-py --with-headers --delim ";" --output $gateType_output --query "$gateType_query" < $1
fi

