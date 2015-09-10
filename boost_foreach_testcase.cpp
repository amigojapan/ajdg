//boost foreach test case
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
typedef boost::unordered_map<int, std::string> Punct_Map_Decode;
typedef boost::unordered_map<std::string, int> Punct_Map_Encode;

int main(int argc, char *argv[]) {
	Punct_Map_Encode punct_map_encode;
	Punct_Map_Decode punct_map_decode;
	#define make_punct_map(symbol,index) punct_map_encode[symbol]=index; punct_map_decode[index]=symbol;
	make_punct_map("\\",0) //"back slash"
	make_punct_map("!",1) //"exclamation mark"
	make_punct_map("#",2) //"hash"
	make_punct_map("$",3) //"dollar"
	make_punct_map("%",4) //"percent"
	BOOST_FOREACH(Punct_Map_Encode::value_type pair, punct_map_encode) {
		cout << pair.first << endl;
	}
	string str="That's";
	cout << str.find("'s") << endl;
	cout << str.find("s") << endl;
	return 0;
}