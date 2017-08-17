#!/bin/bash

###############################################################################
#                               Command line                                  #
###############################################################################

if [ "$#" -ne 2 ]; then
    printf "Usage: $0 input_sam p\n"
    exit -1
fi

input_sam=$1
printf "Input SAM file: $input_sam\n"
p=$1
printf "p: $p\n" # p={1,2,4,8,16,32}

printf "Checking input SAM file $input_sam ... "
if [ ! -f $input_sam ]; then printf "did not find input SAM file: $input_sam\n"; exit -1; fi
printf "OK\n"

###############################################################################
#                                Executables                                  #
###############################################################################

# Binaries
prblock_compress="/project/dna/prog/libCSAM-da36a12/CompressQual"
prblock_decompress="/project/dna/prog/libCSAM-da36a12/DecompressQual"
pblock_string="rblock"

# Python scripts
#

printf "Checking executables ... "
if [ ! -x $prblock_compress ]; then printf "did not find $prblock_compress\n"; exit -1; fi
if [ ! -x $prblock_decompress ]; then printf "did not find $prblock_decompress\n"; exit -1; fi
printf "OK\n"

###############################################################################
#                                  Compress                                   #
###############################################################################

printf "Extracting quality values from FASTQ file ... "
$python $xtract_qual_fastq_py $input_fastq 2> $input_fastq.qual
printf "OK\n"

printf "Compressing quality values ... "
$prblock_compress $input_sam -q 1 -m 1 -l $p
printf "OK\n";

printf "Decompressing quality values ... "
$prblock_decompress $input_sam.cqual
printf "OK\n";

mv $input_sam.cqual $input_sam.$pblock_string
mv $input_sam.cqual.qual $input_sam.$pblock_string.qual
wc -c $input_sam.$pblock_string > $input_sam.$pblock_string.log

###############################################################################
#                                   Cleanup                                   #
###############################################################################

printf "Cleanup ... "
#
printf "OK\n";

