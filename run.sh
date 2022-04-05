#!/bin/bash

# all experiments follow the same execution format of
#  ./program N id other_args ...

program=$1
n=$2
m=$3

usage () {
    echo $@
    echo "usage: $0 [program name] [N] [number of inputs/mults/checks]"
    exit 0
}

if [ -z "${program}" ]; then
    usage "missing program argument"
fi

if ! [[ $n =~ ^[0-9]+$ ]]; then
    usage "not a number:" $n
fi

echo "running: $program $n i $m"

logext=$(basename $program .x)

logdir="logs/logs_${logext}_${n}_${m}_$(date +%s)"

mkdir -p $logdir

echo -n "starting "
for i in $(seq 0 $(($n - 1))); do
    echo -n "$i "
    ( ./$program $n $i $m &>"${logdir}/party_${i}.log" ) &
    pid=$!
done
echo

echo "waiting for experiment to finish ..."
wait $pid
