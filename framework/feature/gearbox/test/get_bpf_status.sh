#!/bin/bash

rm bpf_status.txt

echo ">>>>>>>>>>>>>> bpf programs" >> bpf_status.txt
bpftool prog show >> bpf_status.txt
echo "" >> bpf_status.txt

echo ">>>>>>>>>>>>>> bpf maps" >> bpf_status.txt
bpftool map show >> bpf_status.txt
echo "" >> bpf_status.txt

echo ">>>>>>>>>>>>>> bpf links" >> bpf_status.txt
bpftool link show >> bpf_status.txt
echo "" >> bpf_status.txt
