//Copyright 2015 Usmar Padow (amigojapan) usmpadow@gmail.com

#include <ostream>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
//#include<stdio.h>

#include <cstdlib>
#include <iterator>
#include <vector>
#include <stdint.h>
#include <boost/dynamic_bitset.hpp>
#include <boost/unordered_map.hpp>

// Make the block size one byte
typedef boost::dynamic_bitset<unsigned char> Bitset;
typedef std::vector<unsigned char> bytesArray;
typedef boost::unordered_map<std::string, uint32_t> CompressionHash;

bytesArray dynamic_bitset_to_bytes(Bitset bitset){
    bytesArray bytes;
    boost::to_block_range(bitset, std::back_inserter(bytes));	
	return bytes;
}

Bitset convert_bytes_to_dynamic_bitset(int bitset_size, bytesArray bytes){
	//revert values from bytes to dynamic bitset
	Bitset bitset(bitset_size);
	int bitcount=0;
	for(int byteCount=0; byteCount<bytes.size(); ++byteCount) {
		int mask=0x80;
		for(int individualBit=8; individualBit>0;--individualBit) {
			bitset[(byteCount*8) + individualBit-1]=bytes[byteCount] & mask ? 1 : 0;
			mask >>= 1;
		}
		
	}
	return bitset;
}

Bitset uint32_t_to_bitset(uint32_t number){
	//revert values from bytes to dynamic bitset
	Bitset bitset(32);
	int bitcount=0;
	uint32_t mask=0x80000000;
	for(int individualBit=32; individualBit>0;--individualBit) {
		bitset[individualBit-1]= number & mask ? 1 : 0;
		mask >>= 1;
	}
	return bitset;
}

char version[4]="0.1";

void print_version() {
	std::cout << std::endl << "ajdg text compressor/decompressor Copyright 2015 Usmar Padow (amigojapan) usmpadow@gmail.com"  << std::endl;
	std::cout << "Version: "  << version << std::endl;
}
void print_help() {
	std::cout << std::endl << "Usage: ajdg [-v} version [-h] help" << std::endl;
	std::cout << std::endl << "To compress:" << std::endl; 
	std::cout << std::endl << "ajdg -c -b bits -d path to dictionary -i input file -o outpupt file.ajdg" << std::endl;
	std::cout << std::endl << "To extract" << std::endl;
	std::cout << std::endl << "ajdg -x -b bits -d path to dictionary -i input file.ajdg -o outpupt file" << std::endl;
}
/*
void try_this(CompressionHash *compressionhash, string str){
    try {
      std::cout << str << " at line:" << compressionhash.at(str) << std::endl;
    }
	catch(std::exception const&  ex){
	    //std::cout << "Exception:" << ex.what();
		std::cout << str << " not found!" << std::endl;
	}
}
*/
#define try_this(str) try{std::cout << str << " at line:" << compressionhash.at(str) << std::endl;}catch(std::exception const&  ex){std::cout << str << " not found!" << std::endl;}
#include<string.h>
int main(int argc, char *argv[]) {
	char bits_param[3];
	char dict_file_path[100];
	char input_file_path[100];	
	char output_file_path[100];	
	
	bool compress_mode=false;
	bool extract_mode=false;
	for (int nArg=0; nArg < argc; nArg++) {
		if(strcmp(argv[nArg],"-b")== 0) {
			sprintf(bits_param,"%s", argv[nArg+1]);
		}
		if(strcmp(argv[nArg],"-d")== 0) {
			sprintf(dict_file_path,"%s", argv[nArg+1]);
		}
		if(strcmp(argv[nArg],"-i")== 0) {
			sprintf(input_file_path,"%s", argv[nArg+1]);
		} 
		if(strcmp(argv[nArg],"-o")== 0) {
			sprintf(output_file_path,"%s", argv[nArg+1]);
		} 
		if(strcmp(argv[nArg],"-c")== 0) {
			compress_mode=true;
		} 
		if(strcmp(argv[nArg],"-x")== 0) {
			extract_mode=true;
		}
		if(strcmp(argv[nArg],"-v")== 0) {
			print_version();
			return 0;
		}
		if(strcmp(argv[nArg],"-h") == 0) {
			print_version();
			print_help();
			return 0;
		}		
	}

	if(strcmp(bits_param,"") == 0||strcmp(dict_file_path,"") == 0||strcmp(input_file_path,"") == 0||strcmp(output_file_path,"") == 0||!(compress_mode||extract_mode)) {
		std::cout << std::endl << "Error in command parameters!!" << std::endl;
		print_version();
		print_help();
		return 0;
	}
	if(compress_mode) {
		std::cout << "Reading dictionary.." << std::endl;
		string line;
		CompressionHash compressionhash;
		uint32_t line_number=1;
		
		ifstream myfile (dict_file_path);
		
		if (myfile.is_open()){
			while ( getline (myfile,line) ){
				compressionhash[line] = line_number;				
				//std::cout << line << '\n';
				line_number++;
			}
			myfile.close();
			} else {
				std::cout << "Error: Unable to open dictionary file!!";
				return 0;
		}
		Bitset max_bitset(32);
		max_bitset = uint32_t_to_bitset(line_number);
		std::cout <<  "Lines: " << line_number << " Maxmimum address value in bits: " << max_bitset << std::endl;
		int bits=0;
		for(int counter=32;counter>0;counter--) {
			if(max_bitset[counter]==1) {
				bits=counter+1;
				break;
			}
		}
		std::cout <<  "Bits per word in dictionary: " << bits << std::endl;	
		std::cout << "Opening input file..." << std::endl;
		//this is where the work needs to be done
		//next probably need to come up with all the bit scheme to encode pucntuation and offsets if uncompressable
		
		//Google is great. That’s a great cup of tea. Usmar’s compression algorithm is good! +*}+>?>+*+}*>?>}}*}*>?>}*} Plurals are also interesting.
		try_this("google")
		try_this("is")
		try_this("great")
		try_this("that")
		try_this("a")
		try_this("great")	
		try_this("cup")
		try_this("of")
		try_this("tea")
		try_this("compression")
		try_this("algorithm")
		try_this("is")	
		try_this("good")
		try_this("plural")
		try_this("are")
		try_this("also")
		try_this("interesting")
		
		std::cout << "Compression complete!" << std::endl;
		return 0;
	}

/*	
    Bitset bitset(40); // 40 bits

    // Assign random bits
    for (int i=0; i<40; ++i)
    {
        bitset[i] = std::rand() % 2;
    }

    // Copy bytes to buffer
	bytesArray bytes;
	bytes = dynamic_bitset_to_bytes(bitset);
	
	Bitset bitset2(40);
	
	bitset2 = convert_bytes_to_dynamic_bitset(40, bytes);
	
	
	std::cout << "bitset1: " << bitset << std::endl;
	std::cout << "bitset2: " << bitset2 << std::endl;
*/
}