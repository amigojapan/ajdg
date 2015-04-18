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
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp> 
// Make the block size one byte
typedef boost::dynamic_bitset<unsigned char> Bitset;
typedef std::vector<unsigned char> bytesArray;
typedef boost::unordered_map<std::string, uint32_t> CompressionHash;
typedef boost::unordered_map<int, std::string> Punct_Map_Decode;
typedef boost::unordered_map<std::string, int> Punct_Map_Encode;
#define MAX_OFFSET 4095

bytesArray dynamic_bitset_to_bytes(Bitset bitset){
    bytesArray bytes;
    boost::to_block_range(bitset, std::back_inserter(bytes));	
	return bytes;
}

Bitset convert_bytes_to_dynamic_bitset(int bitset_size, bytesArray bytes){
	//revert values from bytes to dynamic bitset
	Bitset bitset(bitset_size);
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
	std::cout << std::endl << "ajdg -c -d path to dictionary -i input file -o outpupt file.ajdg" << std::endl;
	std::cout << std::endl << "To extract" << std::endl;
	std::cout << std::endl << "ajdg -x -d path to dictionary -i input file.ajdg -o outpupt file" << std::endl;
}
typedef struct pos_punct {
   uint32_t pos;
   string punctuation;
} struct_pos_punct;
struct_pos_punct obj_pos_punct;

void find_next_punctation(string search_string,Punct_Map_Encode punct_map_encode) {
	//find a word in dictionary between next punct or space and beginning of string.
		//find first pucntuation mark in file location
		uint32_t position_found=999999;
		uint32_t position_found_min=999999;
        bool found_plural=false;
        bool found_space=false;
		string punctuation_found="";
		BOOST_FOREACH(Punct_Map_Encode::value_type pair, punct_map_encode) {
			position_found = (uint32_t)search_string.find(pair.first);//Done:modify so it only finds the final "s" of a word! maybe change "s" to "s "
			if(position_found!=999999) {
                if(position_found==position_found_min) {
                    if(pair.first=="s ") found_plural=true;
                    if(pair.first=="s ") found_space=true;
                }
				if(position_found<position_found_min) {
					position_found_min=position_found;
					punctuation_found=pair.first;
				}
			}
		}
        obj_pos_punct.pos=position_found_min;
        if((found_plural&&found_space)&&(punctuation_found=="s "||punctuation_found==" ")) {
            obj_pos_punct.punctuation="s ";
            return;
        }
		obj_pos_punct.punctuation=punctuation_found;
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
#define find_hash(str) try{line_number=compressionhash.at(str);found=true;}catch(std::exception const&  ex){line_number=999999;found=false;}
#include<string.h>
int main(int argc, char *argv[]) {
	char dict_file_path[100];
	char input_file_path[100];	
	char output_file_path[100];	
	
	bool compress_mode=false;
	bool extract_mode=false;
	for (int nArg=0; nArg < argc; nArg++) {
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

	if(strcmp(dict_file_path,"") == 0||strcmp(input_file_path,"") == 0||strcmp(output_file_path,"") == 0||!(compress_mode||extract_mode)) {
		std::cout << std::endl << "Error in command parameters!!" << std::endl;
		print_version();
		print_help();
		return 0;
	}
	//ending punctuations
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
	make_punct_map("’s",33) // posserive
	make_punct_map("s ",34)  // plural
	make_punct_map(" ",35)  // space
		
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
				std::cout << "Error: Unable to open dictionary file!!" << std::endl;
				return 1;
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
		//Begin compression
		//Read in file to compress
		myfile.open(input_file_path);
		string input_file_string="";
		if (myfile.is_open()){
			while ( getline (myfile,line) ){
				input_file_string += line + "\n";				
				//std::cout << line << '\n';
			}
			myfile.close();
			} else {
				std::cout << "Error: Unable to open input file!!" << std::endl;
				return 1;
			}
			std::cout << input_file_string << '\n';
			//Find first compressable data, store first_offset(later)(save it in file already?)
			//find a word in dictionary between next punct or space and beginning of string.
			//find next punctuation or space.
			step1:
            //if end of file goto end_compression
            if(input_file_string=="") return 2;//goto end_compression;
            //***if the working string is a pucntuation mark(***or non ASCII), add to the offset until we find the next compressible
            obj_pos_punct.punctuation="";
            find_next_punctation(input_file_string,punct_map_encode);
            int uncompressible_offset=0;
            char first_char;
            //**find next compressible
            first_char=input_file_string.at(0);
            if((obj_pos_punct.punctuation!=""&&obj_pos_punct.pos==0)||first_char=='\n') {
                uncompressible_offset++;
                input_file_string=input_file_string.substr(1,input_file_string.length());
                goto step1;
            }

			find_next_punctation(input_file_string,punct_map_encode);
			std::cout << "pos:" << obj_pos_punct.pos << "punctuation mark:\"" << obj_pos_punct.punctuation << "\"" << std::endl;
				//find if the next letter is a space, store if so in boolean
			
			//deal with not found and end of file, or offset being too long
			//if not found, next not compressible, offset to end of file.
			uint32_t offset=0;
			if(obj_pos_punct.pos==999999) {
				offset=(uint32_t)input_file_string.length();//not found
				//***todo:terminate compression here goto end_of_compression;
			}
			if(offset>=MAX_OFFSET) {
				std::cout << "ERROR: this file has more than a " << MAX_OFFSET << " stride of uncompressible characters, this file is probably not mostly text, ajdg cannot compress it!" << std::endl;
				return 1;
			}
			
			bool space=false;
            bool apostophe_s=false;
            bool plural=false;
			if(obj_pos_punct.punctuation==" ") space=true;
            if(obj_pos_punct.punctuation=="s ") plural=true;
            if(obj_pos_punct.punctuation=="’s") apostophe_s=true;
			std::cout << "space:" << space << std::endl;
			//check to see if next char in input_file_string is space, if so set next_space boolean to true
			bool next_space=false;
			if(obj_pos_punct.pos!=999999&&!space&&input_file_string.at(obj_pos_punct.pos+1)==' ') next_space=true;
            if(apostophe_s&&input_file_string.at(obj_pos_punct.pos+4)==' ') {
                next_space=true;
            }
        
			std::cout << "next space:" << next_space << std::endl;
			//clip string up to location of punct or space and store in working_string
			string working_string;
			//find if there is a dictionary word between punct or space, if not this is all non compressible, save string length as offset_to_first_compressible_word
				//this should be achieved by feeding each combination of letters left in the string to the hashmap, cause this is much faster than looping thru the hashmap
			//find the offset of the beginning of the word, save the offset in offset_to_first_compressible_word
			working_string=input_file_string.substr(0,obj_pos_punct.pos);
            //clip input_file_string so that it no longer includes working_string, this is done for the next time around looking at the words
            int stride=1;
            if(space) stride=1;
            if(apostophe_s) stride=4;
            if(next_space) stride+=1;
            if(plural) stride=2;
            if(obj_pos_punct.pos==999999) return 3;//no more puctuation ***handle this
            input_file_string=input_file_string.substr(obj_pos_punct.pos+stride,input_file_string.length());
			//convert string to lower case, keep possible_uppercase_copy, so we can later chack if the word is upper case
			string possible_uppercase_copy;
			possible_uppercase_copy=working_string;
			boost::algorithm::to_lower(working_string);
			std::cout << "working string:" << working_string << std::endl;
			line_number=9999999;
			bool found=false;
			offset=0;
			//find index of compressible word in dictionary
			while(!found) {//***Also deal with words that are not in the dinctionaty!!!
				find_hash(working_string)
				if(found) break;
				working_string=working_string.substr(1,working_string.length());
				offset++;
			}
			if(offset>=MAX_OFFSET) {
				std::cout << "ERROR: this file has more than a " << MAX_OFFSET << " stride of uncompressible characters, this file is probably not mostly text, ajdg cannot compress it!" << std::endl;
				return 1;
			}
			std::cout << "found in dictionary:" << found << " line number in dictionary:" << line_number << " offset:" << offset <<std::endl;
			
			//**look to see if the string was upper case, and store the result in a bool
			bool was_uppercase=false;
			char ch = possible_uppercase_copy.at(offset);//get the character that could be uppercase or not,store in ch
			//if(ch >= 'A' && ch <= 'Z') was_uppercase=true;//check to see if the character is uppercase
			if(isupper(ch)) was_uppercase=true;//check to see if the character is uppercase
			std::cout << "possible_uppercase_copy.at(offset):\"" << possible_uppercase_copy.at(offset) << "\" possible_uppercase_copy:\"" << possible_uppercase_copy << "\" was uppercase:" << was_uppercase << std::endl;
			goto step1;//next word!
		//(possible improvement in the future),decide if leace a 1 2 or 3 letter word uncompressed and put an offset to the next compressible word
        
		//operations to generate bitset for current word
		//convert index to index_bitset
		//if the word ends in space, set first flag of flags_bitset to 0, otherwise set it to 1 bit0, and store the punctuation value binary in puct_bitset
		//see if the word is capital, store in flags_bitset bit1
		//if the word did not end in a space, check to see if the next character next to the "punctuation" is a space, if it is, store in flags_bitset bit2
		//check if the next word is compressible(should make a function to find out), if it is not, store in flags_bitset bit3 then seek the next compressible word, get the offset...
			//if offset is more than MAX_OFFSET then exit gracefully, else,covert offset to binary and store in offset_bitset
		//clip the file string to be the next compressible portion and what is left over
		//concatenate bitsets
		//define compressed_bitset as a dynamic bitset
		//convert offset_to_first_compressible_word to offset_to_first_compressible_word_bitset
		//append to compressed_bitset
		//repeat until end of file
		//convert compressed_bitset using dynamic_bitset_to_bytes(Bitset bitset)
         end_compression:
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