#!/bin/bash

folder=$1

if [ -d "$folder" ]; then
    infoFile="$folder/info.csv"
    echo "Info file: $infoFile" 

    runtimeFile="$folder/runtime.csv"
    echo "Runtime file: $runtimeFile"
    reductionFile="$folder/reduction.csv"
    echo "Reduction file: $reductionFile"

    gateType_query="select a2 as type, a4==\"-1\" as wasRemoved, a5.count(\",\") as beforeCount, a6.count(\",\") as afterCount, COUNT(NR) as count group by a2,a5.count(\",\"),a6.count(\",\"),a4==\"-1\""
    gateType_file="$folder/gateTypes.csv"

    set -x
    rbql-py --with-headers --delim ";" --output $gateType_file --query "$gateType_query" < $reductionFile

else
    echo "Argument is not a folder! - $folder"
fi
