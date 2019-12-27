#!/bin/bash

if [ "$1" == "" ]; then
    echo "Path to IDF not given"
else
    export IDF_PATH=$1
    source $1/export.sh
fi
