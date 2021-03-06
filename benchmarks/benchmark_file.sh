#!/bin/bash

# This script assumes that the working directory is benchmarks, since programs
# are specified using relative paths.
#
# To include gzippier in the benchmarks, ensure that it has been built and
# resides in the src directory.
#
# To include gzip 1.10 in the benchmarks, download and build it by running
# get_gzip.sh.
#
# To include pigz and/or bzip2 in the benchmarks, ensure that the system can
# locate them somewhere in its PATH.

possible_progs=(../src/gzip ./gzip-1.10 pigz)
possible_pretty_progs=("gzippier" "gzip" "pigz")

progs=()
pretty_progs=()

for i in $(seq 0 "${#possible_progs[@]}")
do
    if [[ $(command -v "${possible_progs[$i]}") ]]
    then
        progs+=("${possible_progs[$i]}")
        pretty_progs+=("${possible_pretty_progs[$i]}")
    fi
done

get_file_size () {
    local file=$1
    local file_size=$(wc -c $file | sed 's/^[ \t]*//g' | cut -d ' ' -f 1)
    echo "$file_size"
}

print_with_padding () {
    local str=$1
    printf "$str|"
}

print_header () {
    local row_label=$1
    print_with_padding $row_label
    for prog in "${pretty_progs[@]}"
    do
        print_with_padding $prog
    done
    printf "\n"
}

convert_to_seconds () {
    bc <<< $(echo "$1" | sed 's/m.*$//')*60+$(echo "$1" | sed 's/^.*m//' | sed 's/s$//')
}

convert_to_minutes_and_seconds () {
    echo "$(bc <<< $1/60)m$(bc <<< $1%60 | sed 's/0*$//')s" | sed 's/m\./m0\./' | sed 's/^0m//'
}

benchmark () {
    local file=$1
    local num_threads=$2
    local do_levels=$4
    local compr_times=()
    local decompr_times=()

    echo "Compressed file sizes:"
    local table1=$(print_header level)
    table1+="\n"
    max_level=9
    if [[ ! -z $4 ]]
    then
        echo "Custom level amount specified: $4"
        max_level=$4
    fi

    for level in $(seq 1 $max_level)
    do
        table1+=$(print_with_padding $level)
        for prog in "${progs[@]}"
        do
            jflag=
            # TODO: Uncomment this after merging with the parallelize branch.
            if [ "$prog" == "../src/gzip" ]
            then
                jflag="-j $num_threads"
            fi
            if [ "$prog" == "pigz" ]
            then
                jflag="-p $num_threads"
            fi
            compr_time_sum="0"
            decompr_time_sum="0"
            num_runs=$3
            ratio=
            for i in $(seq 1 $num_runs)
            do
                compr_time=$((time $prog $jflag -$level $file) 2>&1 | sed '2q;d' | \
                                 cut -f 2)
                compr_time=`convert_to_seconds $compr_time`
                compr_time_sum=`echo "$compr_time_sum + $compr_time" | bc -l`
                compr_file_size=$(get_file_size "$file*")
                ratio=`echo "scale=5; $compr_file_size/$uncompr_file_size" | bc -l | sed 's/^\./0\./' | sed 's/0*$//'`
                decompr_time=$((time $prog -d $file*) 2>&1 | sed '2q;d' | cut -f 2)
                decompr_time=`convert_to_seconds $decompr_time`
                decompr_time_sum=`echo "$decompr_time_sum + $decompr_time" | bc -l`
            done

            compr_time_avg=`echo "$compr_time_sum/10" | bc -l`
            decompr_time_avg=`echo "$decompr_time_sum/10" | bc -l`
            compr_times+=($compr_time_avg)
            decompr_times+=($decompr_time_avg)
            table1+=$(print_with_padding $ratio)
        done
        table1+="\n"
    done
    column -t -s "|" << EOF
$(printf $table1)
EOF
    printf "\n"

    echo "Compression times:"
    local table2=$(print_header level)
    table2+="\n"
    i=0
    for level in {1..9}
    do
        table2+=$(print_with_padding $level)
        for prog in "${progs[@]}"
        do
            table2+=$(print_with_padding "${compr_times[$i]}" | sed 's/^\./0\./' | sed 's/0*|/|/')
            i=$((i+1))
        done
        table2+="\n"
    done
    column -t -s "|" << EOF
$(printf $table2)
EOF
    printf "\n"

    echo "Decompression times:"
    local table3=$(print_header level)
    table3+="\n"
    i=0
    for level in {1..9}
    do
        table3+=$(print_with_padding "$level")
        for prog in "${progs[@]}"
        do
            table3+=$(print_with_padding "${decompr_times[$i]}" | sed 's/^\./0\./' | sed 's/0*|/|/')
            i=$((i+1))
        done
        table3+="\n"
    done
    column -t -s "|" << EOF
$(printf $table3)
EOF
}

if [ -z $1 ] || [ ! -z $5 ]
then
    echo "Usage: $0 <file> [<num_threads>]"
    exit 1
fi

file=$1
num_threads=1
if [ ! -z $2 ]
then
    num_threads=$2
fi

uncompr_file_size=$(get_file_size $file)

echo "Running benchmarks on $file..."
echo "Uncompressed file size: $uncompr_file_size"
echo "Number of threads for gzippier and pigz: $num_threads"
printf "\n"

benchmark $file $num_threads
