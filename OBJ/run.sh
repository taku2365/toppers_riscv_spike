#!/bin/bash

perl ../configure -T hifive1_gcc
make depend
make

rm result.txt
rm result.dump

spike -m0x80000000:0x50000000 -pc=80000000 -d -l --isa=rv32imac asp 2>&1 | tee result.txt
riscv32-hifive1-elf-objdump -D asp >& result.dump

