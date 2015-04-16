ajdg is a new kind of compression program for text which uses a seperate dictionary file, it can compress text better than other programs because it keeps only pointers to the dictionary file and keeps certain information about punctuation in a few bits.

ajdg uses the Unix philosophy of “Do One Thing and Do It Well” which is why you will need to compress an ajdg file with yet another compressor to get optimum compression, because ajdg does not deal with things like repetition of the same thing, but other compressors do.

Usage: ajdg [-v} version [-h] help

To compress:

ajdg -c -d path to dictionary -i input file -o outpupt file.ajdg

To extract

ajdg -x -d path to dictionary -i input file.ajdg -o outpupt file

please note that ajdg is as of yet a work in progress