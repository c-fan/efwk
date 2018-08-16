#!/bin/bash

#declare -a PLTS=(DMC APB HOST)
declare -a PLTS=(APB HOST)
declare -A ARCHS=([DMC]=arm [APB]=arm64 [HOST]=x86_64)

failed_plt=
for PLT in "${PLTS[@]}"; do
    PLT=$PLT ARCH=${ARCHS[$PLT]} make clean all || failed_plt=$failed_plt,$PLT
done
[ -z "$failed_plt" ] && echo All Succeeded. || echo Failed PLT:`echo $failed_plt|cut -c2-`.

