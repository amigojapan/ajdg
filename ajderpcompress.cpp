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
	//ending punctuations
	typedef boost::unordered_map<int, std::string> Punct_Map_Decode;
	typedef boost::unordered_map<std::string, int> Punct_Map_Encode;
	Punct_Map_Encode punct_map_encode;
	Punct_Map_Decode punct_map_decode;
	#define make_punct_map(symbol,index) punct_map_encode[symbol]=index; punct_map_decode[index]=symbol;
	make_punct_map("\\",0) //"back slash"
	make_punct_map("!",1) //"exclamation mark"
	make_punct_map("#",2) //"hash"
	make_punct_map("$",3) //"dollar"
	make_punct_map("%",4) //"percent"
	make_punct_map("&",5) //"and sign"
	make_punct_map("'",6) //"single quote"
	make_punct_map("\"",7) //"quote"
	make_punct_map("(",8) //"open parenthesis"
	make_punct_map(")",9) //"close parenthesis"
	make_punct_map("-",10) //"minus"
	make_punct_map("=",11) //"equal"
	make_punct_map("^",12) //"carrot"
	make_punct_map("~",13) //"tilde"
	make_punct_map("¥",14) //"Yen"
	make_punct_map("|",15) //"or sign"
	make_punct_map("@",16) //"at"
	make_punct_map("`",17) //"back tick"
	make_punct_map("[",18) //"open bracket"
	make_punct_map("]",19) //"close bracket"
	make_punct_map("{",20) //"open curly brace"
	make_punct_map("}",21) //"close curly brace"
	make_punct_map(";",22) //"semi colon"
	make_punct_map(":",23) //"colon"
	make_punct_map("+",24) //"plus"
	make_punct_map("*",25) //"asterisk"
	make_punct_map(",",26) //"comma"
	make_punct_map(".",27) //"dot"
	make_punct_map(">",28) //"more than"
	make_punct_map("<",29) //"less than"
	make_punct_map("/",30) //"foward slash"
	make_punct_map("?",31) //"question mark"
	make_punct_map("_",32) //“underscore”
	make_punct_map("‘s",33) // posserive
	make_punct_map("s",34)  // plural	
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
		//begin compression
		//Find first compressable data, store offset(later)(save it in file already?)
		//Setp1:find next punctuation or space.
		//find a word in dictionary between next punct or space and beginning of string.
		//if not found, not compressible, goto Step1
		//find index of compressible word in dictionary
		//convert index to index_bitset
		//if the word ends in space, set first flag of flags_bitset to 0, otherwise set it to 1 bit0, and store the punctuation value binary in puct_bitset
		//see if the word is capital, store in flags_bitset bit1
		//if the word did not end in a space, check to see if the next character next to the "punctuation" is a space, if it is, store in flags_bitset bit2
		//check if the next word is compressible(should make a function to find out), if it is not, store in flags_bitset bit3 then seek the next compressible word, get the offset...
			//if offset is more than MAX_OFFSET then exit gracefully, else,covert offset to binary and store in offset_bitset
		//clip the file string to be the next compressible portion and what is left over
		//concatenate bitsets
		//define compressed_bitset as a dynamic bitset
		//append to compressed_bitset
		//repeat until end of file
		//convert compressed_bitset using dynamic_bitset_to_bytes(Bitset bitset)
		//save file
		
				
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