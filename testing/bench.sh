#!/bin/sh

input=$(wc -c input.txt | cut -d " " -f 6)

echo "Compressed file size in bytes:"
echo "\t gzip \t bzip2 \t lzma"

#gzip -d input.txt.gz   
com_time=$((time gzip input.txt) 2>&1 | sed '2q;d' | cut -f 2)
dec_time=$((time gzip -d input.txt.gz) 2>&1| sed '2q;d' | cut -f 2)

printf "compression time: \t$com_time\n"
printf "decompression time: \t$dec_time\n"

echo "and again:"

com_time=$((time gzip input.txt) 2>&1 | sed '2q;d' | cut -f 2)
dec_time=$((time gzip -d input.txt.gz) 2>&1| sed '2q;d' | cut -f 2)
printf "compression time: \t$com_time\n"
printf "decompression time: \t$dec_time\n"

