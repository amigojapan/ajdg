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
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp> 
//#include "dynamic_bitset_concatenators.h"
// Make the block size one byte
typedef boost::dynamic_bitset<unsigned char> Bitset;
typedef std::vector<unsigned char> bytesArray;
//#include <unordered_map>
//typedef std::unordered_map<std::string, uint32_t> CompressionHash;//http://en.cppreference.com/w/cpp/container/unordered_map
//typedef std::unordered_map<uint32_t, std::string> DeCompressionHash;
//typedef std::unordered_map<int, std::string> Punct_Map_Decode;
//typedef std::unordered_map<std::string, int> Punct_Map_Encode;

#include <boost/unordered_map.hpp>
//#include <map>
//typedef std::map<std::string, uint32_t> CompressionHash;
typedef boost::unordered_map<std::string, uint32_t> CompressionHash;//http://en.cppreference.com/w/cpp/container/unordered_map
typedef boost::unordered_map<uint32_t, std::string> DeCompressionHash;
typedef boost::unordered_map<int, std::string> Punct_Map_Decode;
typedef boost::unordered_map<std::string, int> Punct_Map_Encode;
#define MAX_OFFSET 254//this is the maximum non-text can be from the next compressible before it is not ocnsidered to be a pure text file anymore, note to expand this I would have to add more bytes to the offset, so for now I will chear and use only one byte, that is why the maximum is 254


namespace local {
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
    
    Bitset uint8_t_to_bitset(uint8_t number){
        //revert values from bytes to dynamic bitset
        Bitset bitset(8);
        uint32_t mask=0x80;
        for(int individualBit=8; individualBit>0;--individualBit) {
            bitset[individualBit-1]= number & mask ? 1 : 0;
            mask >>= 1;
        }
        return bitset;
    }
    
    uint8_t bitset_to_uint8_t(Bitset bitset,int byte_offset){
        //from bitset to byte
        uint32_t mybyte=0;
        for(int individualBit=8; individualBit>=0;--individualBit) {
            mybyte |= bitset[(byte_offset*8)+individualBit]<<individualBit;
        }
        return mybyte;
    }
    
    uint32_t bitset_to_uint32_t(Bitset bitset,int offset,int bits){
        //from bitset to byte
        uint32_t mybyte=0;
        for(int individualBit=bits-1; individualBit>=0;--individualBit) {//the -1 is a quickhack
            //cout<<"\ntest:"<<bitset[offset+individualBit];
            mybyte |= bitset[offset+individualBit]<<individualBit;
        }
        return mybyte;
    }
    
    Bitset concatenate_dynamic_bitsets(Bitset left_bitset, Bitset right_bitset) {
        Bitset concated(right_bitset.size()+left_bitset.size());
        double offset=0;
        for(int counter=0;counter<right_bitset.size();counter++){
            concated[counter]=right_bitset[counter];
            offset++;
        }
        for(int counter=0;counter<left_bitset.size();counter++){
            concated[offset+counter]=left_bitset[counter];
        }
        return concated;
    }

    Bitset append_bool_to_dynamic_bitset_right(Bitset bitset, bool Boolean_value) {
        Bitset concated(bitset.size()+1);
        concated[0]=Boolean_value;
        for(int counter=0;counter<bitset.size();counter++){
            concated[counter+1]=bitset[counter];
        }
        return concated;
    }
    
    Bitset append_bool_to_dynamic_bitset_left(Bitset bitset, bool Boolean_value) {
        Bitset concated(bitset.size()+1);
        int offset=0;
        for(int counter=0;counter<bitset.size();counter++){
            concated[counter]=bitset[counter];
            offset++;
        }
        concated[offset]=Boolean_value;
        return concated;
    }
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

typedef struct compressed_structure {
    uint32_t index;
    bool punctuation;
    bool capital;
    string punctuation_string="none";
    bool punctuation_bit0=0;
    bool punctuation_bit1=0;
    bool punctuation_bit2=0;
    bool punctuation_bit3=0;
    bool punctuation_bit4=0;
    bool punctuation_bit5=0;
    bool space;
    bool next_compressible=true;
    uint8_t offset=0;
    string word;//for debugging purposes
} struct_compressed;
std::vector<struct_compressed> compressed_data_array;

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
//#define try_this(str) try{std::cout << str << " at line:" << compressionhash.at(str) << std::endl;}catch(std::exception const&  ex){std::cout << str << " not found!" << std::endl;}
//#define find_hash(str) try{line_number=compressionhash.at(str);found=true;}catch(std::exception const&  ex){line_number=999999;found=false;}//weird, when this executes it modifyies the value of a variable that is not mentioned in this macro... why?
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
	make_punct_map("\%",4) //"percent"
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
	make_punct_map("\n",36)  // newline
	if(compress_mode) {
		std::cout << "Reading dictionary.." << std::endl;
		string line="";
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
		max_bitset = local::uint32_t_to_bitset(line_number);
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
                //derpson: ifstream file; file.open(filename); std::string word; file >> word;
                //[5:10pm] derpson: std::ifstream xD sorry
				//std::cout << line << '\n';
			}
            input_file_string =  input_file_string +"quickhack\n";//***quickhack to get the file to parse until the last word,  in the file, this quick hack also appeds an element to the end of the array, which needs a second quick hack to remove it
			myfile.close();
			} else {
				std::cout << "Error: Unable to open input file!!" << std::endl;
				return 1;
			}
			std::cout << input_file_string << '\n';
			//Find first compressable data, store first_offset(later)(save it in file already?)
			//find a word in dictionary between next punct or space and beginning of string.
			//find next punctuation or space.
			//step1:
            int uncompressible_offset=0;
            string uncompressable_characters_string="";
            //bool first_loop=true;
            while(true) {
                bool next_compressible=true;
                //if(!first_loop) {
                 //   unsigned long next_to_last = compressed_data_array.size()-1;
                
                //}
                //first_loop=false;
                //if end of file goto end_compression
                if(input_file_string=="") {
                    break;//return 2;//goto end_compression;
                }
                //***if the working string is a pucntuation mark(***or non ASCII), add to the offset until we find the next compressible
                obj_pos_punct.punctuation="";
                find_next_punctation(input_file_string,punct_map_encode);
                //find if the word is in dictionary,greg said to do this, sorry but I thoght it was easier to do a quick and ditry fix...
                char first_char;
                //**find next compressible
                first_char=input_file_string.at(0);
                if((obj_pos_punct.punctuation!=""&&obj_pos_punct.pos==0)||first_char=='\n') {//if first chat is punctation or newline(I think the newline is overriden by the one in the punctuation maks)
                    next_compressible=false;
                    uncompressible_offset++;
                    uncompressable_characters_string+=input_file_string.substr(0,1);
                    input_file_string=input_file_string.substr(1,input_file_string.length());
                    //set the previous element's offset and non compressible flag
                    unsigned long next_to_last = compressed_data_array.size()-1;
                    compressed_data_array.at(next_to_last).next_compressible=false;
                    compressed_data_array.at(next_to_last).offset=uncompressible_offset;
                    continue;
                }
                uncompressible_offset=0;
                //add a member to the compressed data array
                struct_compressed new_element;
                new_element.next_compressible=true;
                compressed_data_array.insert(compressed_data_array.end(), new_element);
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
                bool apostrophe_s=false;
                bool plural=false;
                if(obj_pos_punct.punctuation==" ") space=true;
                if(obj_pos_punct.punctuation=="s ") plural=true;
                if(obj_pos_punct.punctuation=="’s") apostrophe_s=true;
                std::cout << "space:" << space << std::endl;
                //check to see if next char in input_file_string is space, if so set next_space boolean to true
                bool next_space=false;
                if(obj_pos_punct.pos+1>=input_file_string.length()){
                    //end of file reached
                    break;
                }
                if(
                   obj_pos_punct.pos!=999999
                   &&
                   !space
                   &&
                   input_file_string.at(obj_pos_punct.pos+1)==' '
                ) next_space=true;
                if(apostrophe_s&&input_file_string.at(obj_pos_punct.pos+4)==' ') {
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
                if(apostrophe_s) stride=4;
                if(next_space) stride+=1;
                if(plural) stride=2;
                if(obj_pos_punct.pos==999999) {//no more punctuation
                    next_compressible=false;
                    //set the previous element's offset and non compressible flag
                    compressed_data_array.erase(compressed_data_array.end()-1);//erase last element
                    unsigned long next_to_last = compressed_data_array.size()-1;
                    compressed_data_array.at(next_to_last).next_compressible=false;
                    compressed_data_array.at(next_to_last).offset=input_file_string.length();
                    break;
                }
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
                //while(!found) {//***Also deal with words that are not in the dinctionaty!!!

                //std::string * input_file_string_backup= new std::string("Just a string.");string //=input_file_string;
                //memcpy ( &input_file_string_backup, &input_file_string, sizeof(input_file_string) );
                //find_hash(working_string)
                /* this takes too long and uses up lots of memory and crashed, also defeats the point of using a hash
                //try looping thru all the hash and finding the item?
                found=false;
                for ( auto it = compressionhash.begin(); it != compressionhash.end(); ++it ) {
                    std::cout << " first:" << it->first << " second:" << it->second;
                    if(it->first==working_string) {
                        found=true;
                        line_number=it->second;
                    } else {
                        line_number=999999;
                        found=false;
                    }
                }
                */
                if (working_string=="13th") {
                    
                    ;
                }
                next_word://I know this is a total dirty hack, hell! go with goto!
                try{
                    line_number=compressionhash.at(working_string);
                    found=true;
                }catch(
                    std::exception const&  ex){
                    line_number=999999;
                    found=false;
                }
                
                 //weird, when this executes it modifyies the value of a variable that is not mentioned in this macro... why?
                //input_file_string=input_file_string_backup;
                //memcpy ( &input_file_string, &input_file_string_backup, sizeof(input_file_string_backup) );
                //delete input_file_string_backup;
                
                if(!found) {
                        uncompressible_offset++;
                        uncompressable_characters_string+=input_file_string.substr(0,1);
                        input_file_string=input_file_string.substr(1,input_file_string.length());
                        unsigned long next_to_last = compressed_data_array.size()-1;
                        compressed_data_array.at(next_to_last).next_compressible=false;
                        compressed_data_array.at(next_to_last).offset=uncompressible_offset;
                        std::size_t found = input_file_string.find(" ");
                        if (found!=std::string::npos){
                            working_string=input_file_string.substr(0,found);
                        }else{ break;}
                        goto next_word;
                }
                if (working_string=="13th") {
                    ;
                }
                //next_compressible=false;
                //uncompressible_offset++;
                //uncompressable_characters_string+=input_file_string.substr(0,1);
                //set the previous element's offset and non compressible flag
                /*
                uncompressable_characters_string+=working_string.substr(0,1);//add the uncompresable character to the string
                working_string=working_string.substr(1,working_string.length());
                offset++;
                unsigned long next_to_last = compressed_data_array.size()-1;
                compressed_data_array.at(next_to_last).next_compressible=false;
                compressed_data_array.at(next_to_last).offset=offset;
                */
                //}
                if(offset>=MAX_OFFSET) {
                    std::cout << "ERROR: this file has more than a " << MAX_OFFSET << " stride of uncompressible characters, this file is probably not mostly text, ajdg cannot compress it!" << std::endl;
                    return 1;
                }
                std::cout << "found in dictionary:" << found << " line number in dictionary:" << line_number << " offset:" << offset <<std::endl;
                
                //look to see if the string was upper case, and store the result in a bool
                bool was_uppercase=false;
                char ch = possible_uppercase_copy.at(offset);//get the character that could be uppercase or not,store in ch
                //if(ch >= 'A' && ch <= 'Z') was_uppercase=true;//check to see if the character is uppercase
                if(isupper(ch)) was_uppercase=true;//check to see if the character is uppercase
                std::cout << "possible_uppercase_copy.at(offset):\"" << possible_uppercase_copy.at(offset) << "\" possible_uppercase_copy:\"" << possible_uppercase_copy << "\" was uppercase:" << was_uppercase << std::endl;
                //add the data we need to the array
                compressed_data_array.at(compressed_data_array.size()-1).index=line_number;
                compressed_data_array.at(compressed_data_array.size()-1).punctuation=obj_pos_punct.punctuation==" "?false:true;
                if(compressed_data_array.at(compressed_data_array.size()-1).punctuation) {
                    compressed_data_array.at(compressed_data_array.size()-1).punctuation_string=obj_pos_punct.punctuation==" "?"none":obj_pos_punct.punctuation;
                    //prepare the punctuation bits
                    Bitset punctuation_32bit(32);
                    punctuation_32bit=local::uint32_t_to_bitset((uint32_t) punct_map_encode[obj_pos_punct.punctuation]);
                    //maybe get rid of spaces from punctuation saving cause it is already saved in the space bit
                    compressed_data_array.at(compressed_data_array.size()-1).punctuation_bit0=punctuation_32bit[0];
                    compressed_data_array.at(compressed_data_array.size()-1).punctuation_bit1=punctuation_32bit[1];
                    compressed_data_array.at(compressed_data_array.size()-1).punctuation_bit2=punctuation_32bit[2];
                    compressed_data_array.at(compressed_data_array.size()-1).punctuation_bit3=punctuation_32bit[3];
                    compressed_data_array.at(compressed_data_array.size()-1).punctuation_bit4=punctuation_32bit[4];
                    compressed_data_array.at(compressed_data_array.size()-1).punctuation_bit5=punctuation_32bit[5];
                }
                compressed_data_array.at(compressed_data_array.size()-1).capital=was_uppercase;
                compressed_data_array.at(compressed_data_array.size()-1).space=space||next_space;//is this even valid? seems to be working
                compressed_data_array.at(compressed_data_array.size()-1).word=working_string;
			//goto step1;//next word!
            }
            //debug by itterating over array and printing otu the data in it
            printf("index\t\tpunctuation?\t\tpunct\t\tcapital\t\tspace\t\tnext_compressible\t\toffset\n");
        compressed_data_array.pop_back();//second quick hack to remove the element added by the first quick hack
        for(int element=0;element<compressed_data_array.size();element++) {
                cout << compressed_data_array.at(element).index << "\t\t" <<
                compressed_data_array.at(element).punctuation;
            
                if(compressed_data_array.at(element).punctuation) {
                    cout << "bits:"<<
                    compressed_data_array.at(element).punctuation_bit5 <<
                    compressed_data_array.at(element).punctuation_bit4 <<
                    compressed_data_array.at(element).punctuation_bit3 <<
                    compressed_data_array.at(element).punctuation_bit2 <<
                    compressed_data_array.at(element).punctuation_bit1 <<
                    compressed_data_array.at(element).punctuation_bit0;
                }
            
                if(compressed_data_array.at(element).punctuation_string=="\n")
                    cout << "nl" << "\t\t";
                else
                    cout << compressed_data_array.at(element).punctuation_string << "\t\t";
                cout << compressed_data_array.at(element).capital << "\t\t" <<
                compressed_data_array.at(element).space << "\t\t" <<
                compressed_data_array.at(element).next_compressible << "\t\t" <<
                (unsigned int) compressed_data_array.at(element).offset << "\t\t" <<
                " -> " << compressed_data_array.at(element).word << endl;
        }
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
        /*
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
		*/

        /*
        Bitset concated(right_bitset.size()+left_bitset.size());
        double offset=0;
        for(int counter=0;counter<right_bitset.size();counter++){
            concated[counter]=right_bitset[counter];
            offset++;
        }
        for(int counter=0;counter<left_bitset.size();counter++){
            concated[offset+counter]=left_bitset[counter];
        }
        */
        std::cout << "Compression complete!" << std::endl;
        //boost::dynamic_bitset<> left_bitset( std::string("10") );
        //boost::dynamic_bitset<> right_bitset( std::string("00") );
        /*
        Bitset left_bitset(2);
        Bitset right_bitset(2);
        left_bitset[0]=1;
        left_bitset[1]=1;
        right_bitset[0]=0;
        right_bitset[1]=0;
        Bitset concated;
        concated = local::concatenate_dynamic_bitsets(left_bitset,right_bitset);
        concated = local::append_bool_to_dynamic_bitset_right(concated,1);
        concated = local::append_bool_to_dynamic_bitset_left(concated,0);
        string outpupt;
        to_string(concated, outpupt);
        cout << concated;
        */
        cout<<"concatenating bitsets...";
        ofstream compresed_file;
        compresed_file.open ("compressed_file.ajdg");
        Bitset compressed_bitset;
        for(int element=0;element<compressed_data_array.size();element++) {
            Bitset tmp_bitset;
            tmp_bitset=local::uint32_t_to_bitset(compressed_data_array.at(element).index);
            string outpupt;
            to_string(tmp_bitset, outpupt);
            Bitset shortened_bitset(bits);
            for (int counter=0;counter<bits;counter++) {
                shortened_bitset[counter]=tmp_bitset[counter];
            }
            compressed_bitset=local::concatenate_dynamic_bitsets(compressed_bitset,shortened_bitset);
            cout << "\nindex" <<compressed_data_array.at(element).index << " " <<
            "bitset:" << outpupt <<
            "punct:" << compressed_data_array.at(element).punctuation;
            compressed_bitset = local::append_bool_to_dynamic_bitset_right(compressed_bitset,compressed_data_array.at(element).punctuation);
            if(compressed_data_array.at(element).punctuation) {
                cout << "puctuation bits:"<<
                compressed_data_array.at(element).punctuation_bit5 <<
                compressed_data_array.at(element).punctuation_bit4 <<
                compressed_data_array.at(element).punctuation_bit3 <<
                compressed_data_array.at(element).punctuation_bit2 <<
                compressed_data_array.at(element).punctuation_bit1 <<
                compressed_data_array.at(element).punctuation_bit0;
                compressed_bitset = local::append_bool_to_dynamic_bitset_right(compressed_bitset,compressed_data_array.at(element).punctuation_bit5);
                compressed_bitset = local::append_bool_to_dynamic_bitset_right(compressed_bitset,compressed_data_array.at(element).punctuation_bit4);
                compressed_bitset = local::append_bool_to_dynamic_bitset_right(compressed_bitset,compressed_data_array.at(element).punctuation_bit3);
                compressed_bitset = local::append_bool_to_dynamic_bitset_right(compressed_bitset,compressed_data_array.at(element).punctuation_bit2);
                compressed_bitset = local::append_bool_to_dynamic_bitset_right(compressed_bitset,compressed_data_array.at(element).punctuation_bit1);
                compressed_bitset = local::append_bool_to_dynamic_bitset_right(compressed_bitset,compressed_data_array.at(element).punctuation_bit0);
                
            }
            
            if(compressed_data_array.at(element).punctuation_string=="\n")
                cout << "nl" << "\t\t";
            else
                cout << compressed_data_array.at(element).punctuation_string << "\t\t";
            cout << compressed_data_array.at(element).capital << "\t\t" <<
            compressed_data_array.at(element).space << "\t\t" <<
            compressed_data_array.at(element).next_compressible << "\t\t" <<
            (unsigned int) compressed_data_array.at(element).offset << "\t\t" <<
            " -> " << compressed_data_array.at(element).word << endl;
            compressed_bitset = local::append_bool_to_dynamic_bitset_right(compressed_bitset,compressed_data_array.at(element).capital);
            compressed_bitset = local::append_bool_to_dynamic_bitset_right(compressed_bitset,compressed_data_array.at(element).space);
            compressed_bitset = local::append_bool_to_dynamic_bitset_right(compressed_bitset,compressed_data_array.at(element).next_compressible);
            if(!compressed_data_array.at(element).next_compressible) {
                Bitset offset_bitset;
                offset_bitset=local::uint8_t_to_bitset(compressed_data_array.at(element).offset);
                compressed_bitset = local::concatenate_dynamic_bitsets(compressed_bitset,offset_bitset);
                to_string(offset_bitset, outpupt);
                cout << "offset bits:" << outpupt;
                //pack uncompressable characters
                for (int counter=0; counter<compressed_data_array.at(element).offset; counter++) {
                    string one_char_string=uncompressable_characters_string.substr(0,1);
                    char uncompressable_character=one_char_string.c_str()[0];
                    cout<<"uncompressables caracter to pack:"<<uncompressable_character;//uncompressable_character;
                    uncompressable_characters_string=uncompressable_characters_string.substr(1,uncompressable_characters_string.length());
                     tmp_bitset=local::uint8_t_to_bitset((uint8_t)uncompressable_character);
                    compressed_bitset=local::concatenate_dynamic_bitsets(compressed_bitset,tmp_bitset);
                }
                
            }

            

        }
        //add trailing 0s to bitset so that it is dividable into 8 bits always
        int difference = compressed_bitset.size() % 8;
        cout << "difference1:" << difference;
        for(int counter=0;counter<(8-difference);counter++){//add all the extra bits so 8 - difference is the ammount of extra bits
            compressed_bitset = local::append_bool_to_dynamic_bitset_right(compressed_bitset,0);
        }
        difference = compressed_bitset.size() % 8;
        cout << "difference2:" << difference;
        //Write out to file
        //it seems I need to write this out byte by byte
        cout <<"\ncompressed bitset:"<<compressed_bitset<<endl;
        for(int byte_number=(int)compressed_bitset.size()/8-1;byte_number>=0;byte_number--) {//loop thru every byte of the bitset
            uint8_t byte=0;
            byte = local::bitset_to_uint8_t(compressed_bitset ,byte_number);
            printf("%x",byte);
            compresed_file.put(byte);
            //cout<<"bitset:"<<local::uint8_t_to_bitset(byte);
        }
        compresed_file.close();
        
        //uncompressor
        ifstream is ("compressed_file.ajdg", ios::in|ios::binary);
        is.is_open();
        //std::ifstream is("");     // open file
        
        char c;
        cout<<"\nopening archive...";
        Bitset input_byte_bitset;
        Bitset decompressioin_bitset;
        while (!is.eof()){          // loop getting single characters
            is.get(c);
            //printf("%x",c);
            input_byte_bitset= local::uint8_t_to_bitset((uint8_t)c);
            cout << input_byte_bitset;
            decompressioin_bitset=local::concatenate_dynamic_bitsets(decompressioin_bitset,input_byte_bitset);
        }
        cout<<endl<<"decompressioin_bitset:"<<decompressioin_bitset;
        //eliminate extra data
        //Bitset decompressioin_bitset(bitset_with_junk.size()-8);
        //copy data
        //for(int counter=0;counter<bitset_with_junk.size()-8;counter++) {
        //    decompressioin_bitset[counter]=bitset_with_junk[counter];
        //}
        //cout<<endl<<"clean bitset:"<<decompressioin_bitset;
        //cout<<endl<<"reversing bitset...";
        //Bitset reverse_bitset(decompressioin_bitset.size());
        
        
        //deserialize clean bitset
        std::cout << "Reading dictionary.." << std::endl;
        //string line;
        DeCompressionHash decompressionhash;
        //uint32_t line_number=1;
        
        ifstream DictFile (dict_file_path);
        line_number=0;
        if (DictFile.is_open()){
            while ( getline (DictFile,line) ){
                decompressionhash[line_number] = line;
                //std::cout << line << '\n';
                line_number++;
            }
            DictFile.close();
        }
        string output_string="";
        double offset=decompressioin_bitset.size();
        while(true) {
            uint32_t index;
            //index=local::bitset_to_uint32_t(decompressioin_bitset,(uint32_t)offset-32+8);//hmm, seems this is only 24 bits, strange it oviously is not(oh wait, did I pack it into 24 bits? I think that was my plan, I may need to fix this since this is important for compression....
            index=local::bitset_to_uint32_t(decompressioin_bitset,(uint32_t)offset-bits,bits);
            string word=decompressionhash[index-1];
            cout <<"word:"<< word;
            cout<<"punctuation:";
            offset=offset-bits;
            offset--;
            int puctuation_index=0;
            int jump_offset=0;
            if (decompressioin_bitset[offset]) {
                //do what is nessesary for puctuation
                cout<<"punctuation true";
                //decode puctuation
                puctuation_index=(int)local::bitset_to_uint32_t(decompressioin_bitset,(uint32_t)offset-6,6);//why is this 6 and not 5? probably an off by 1 thinggy(I am really bad at them) but it appears to be 6, eventhough it is 5 bits for the puctuation index

                cout<<"puctuation index:"<<puctuation_index;
                cout<<"punctuation mark:"<<punct_map_decode[puctuation_index];
                word+=punct_map_decode[puctuation_index];
                offset=offset-6;
            } else {
                cout<<"punctuation false";
            }
            offset--;
            cout<<"capital:"<<decompressioin_bitset[offset];
            if (decompressioin_bitset[offset]) {
                word[0] = toupper(word[0]);
            }
            offset--;
            cout<<"space:"<<decompressioin_bitset[offset];
            if (decompressioin_bitset[offset]) {
                word += " ";
            }
            output_string+=word;
            offset--;
            cout<<"nextcompressible:"<<decompressioin_bitset[offset];
            if (!decompressioin_bitset[offset]) {
                //do what is nessesary for puctuation
                //decode jump offset
                jump_offset=(int)local::bitset_to_uint32_t(decompressioin_bitset,(uint32_t)offset-8,8);//why is this 6 and not 5? probably an off by 1 thinggy(I am really bad at them) but it appears to be 6, eventhough it is 5 bits for the puctuation index
                
                cout<<"jump offset:"<<jump_offset;
                offset-=8;
                //offset-=jump_offset*8;//jump to character after jump(possibly need +1)
                //decode uncompressable characters
                
                char uncompressable_character;
                for (int counter=0; counter<jump_offset; counter++) {
                    uncompressable_character=(char)local::bitset_to_uint32_t(decompressioin_bitset,(uint32_t)offset-8,8);
                    cout<<"uncompressable character:"<<uncompressable_character;
                    output_string+=uncompressable_character;
                    offset-=8;//move one byte up
                }
                
            }
            if (offset<=20) {//if we reached the end of the bite(remember there may be trailing zeros
                break;
            }
        }
            //std::couindex	uint32_t	index	uint32_t	64483	64483index	uint32_t	128967	12896716507849	16507849t << c;
        cout<<"output string:"<<output_string;
        is.close();                // close file
        /*
        Bitset b(8*2);
        b[15]=1;
        b[14]=0;
        b[13]=0;
        b[12]=0;
        b[11]=0;
        b[10]=0;
        b[9]=0;
        b[8]=0;

        b[7]=0;
        b[6]=0;
        b[5]=0;
        b[4]=0;
        b[3]=0;
        b[2]=0;
        b[1]=1;
        b[0]=1;

        uint8_t byte;
        byte = local::bitset_to_uint8_t(b ,1);
        printf("byte int:%i",byte);
        */
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
//maybe use shorter bites like unicode does for ascii and longer for the more obscure words
//make a ducrionary using word count from enwiki8