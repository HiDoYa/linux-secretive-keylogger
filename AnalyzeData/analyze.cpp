#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream> 
#include <vector>
#include <list>

using namespace std;

char numarr[] = {
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
};

string states_initial[] = {
	"al",
	"al",
	"ak",
	"az",
	"az",
	"ar",
	"ar",
	"ca",
	"ca",
	"ca",
	"ca",
	"ca",
	"ca",
	"co",
	"co",
	"ct",
	"de",
	"fl",
	"fl",
	"fl",
	"ga",
	"ga",
	"ga",
	"hi",
	"id",
	"il",
	"il",
	"il",
	"in",
	"in",
	"ia",
	"ia",
	"ia",
	"ks",
	"ks",
	"ks",
	"ks",
	"ky",
	"ky",
	"ky",
	"la",
	"la",
	"me",
	"me",
	"md",
	"md",
	"ma",
	"ma",
	"ma",
	"mi",
	"mi",
	"mn",
	"mn",
	"ms",
	"ms",
	"mo",
	"mo",
	"mo",
	"mt",
	"ne",
	"ne",
	"nv",
	"nh",
	"nj",
	"nj",
	"nm",
	"nm",
	"ny",
	"ny",
	"ny",
	"ny",
	"ny",
	"nc",
	"nc",
	"nd",
	"oh",
	"oh",
	"oh",
	"ok",
	"ok",
	"or",
	"pa",
	"pa",
	"pa",
	"pa",
	"pa",
	"ri",
	"sc",
	"sd",
	"tn",
	"tn",
	"tx",
	"tx",
	"tx",
	"tx",
	"tx",
	"tx",
	"ut",
	"vt",
	"va",
	"va",
	"va",
	"va",
	"va",
	"va",
	"wa",
	"wa",
	"wv",
	"wv",
	"wv",
	"wi",
	"wi",
	"wy",
	"wy",
};

string states_zip[] = {
	"35",
	"36",
	"99",
	"85",
	"86",
	"71",
	"72",
	"90",
	"91",
	"92",
	"93",
	"94",
	"95",
	"80",
	"81",
	"06",
	"19",
	"32",
	"33",
	"34",
	"30",
	"31",
	"39",
	"96",
	"83",
	"60",
	"61",
	"62",
	"46",
	"47",
	"50",
	"51",
	"52",
	"66",
	"61",
	"71",
	"72",
	"40",
	"41",
	"42",
	"70",
	"71",
	"03",
	"04",
	"20",
	"21",
	"01",
	"02",
	"05",
	"48",
	"49",
	"55",
	"56",
	"38",
	"39",
	"63",
	"64",
	"65",
	"59",
	"68",
	"69",
	"89",
	"03",
	"07",
	"08",
	"87",
	"88",
	"10",
	"11",
	"12",
	"13",
	"14",
	"27",
	"28",
	"58",
	"43",
	"44",
	"45",
	"73",
	"74",
	"77",
	"15",
	"16",
	"17",
	"18",
	"19",
	"02",
	"29",
	"57",
	"37",
	"38",
	"75",
	"76",
	"77",
	"78",
	"79",
	"88",
	"84",
	"05",
	"20",
	"22",
	"23",
	"24",
	"25",
	"26",
	"98",
	"99",
	"24",
	"25",
	"26",
	"53",
	"54",
	"82",
	"83",
};

vector<long long> GetStringByNumArr(string str) {
	vector<long long> numlist;
	int startIndex = 0;
	while (startIndex != -1) {
		vector<char> tempnum;
		startIndex = -1;
		for (size_t i = 0; i < str.length(); i++) {
			for (size_t j = 0; j < 10; j++) {
				if (str[i] == numarr[j]) {
					startIndex = i;
					break;
				}
			}
			if (startIndex != -1) {
				tempnum.push_back(str[startIndex]);
				int tempindex = 0;
				char tempchar = str[startIndex + (tempindex += 1)];
				while (int(tempchar - 48) >= 0 && int(tempchar - 48) <= 9) {
					tempnum.push_back(tempchar);
					tempchar = str[startIndex + (tempindex += 1)];
				}
				str.erase(startIndex, tempindex);
				break;
			}
		}
		if (!tempnum.empty()) {
			numlist.push_back(stoll(string(tempnum.begin(), tempnum.end())));
		}
	}
	return numlist;
}

int main() {
	//read in file into buffer calls "content"
	ifstream ifs("log_file.txt");
	string content((istreambuf_iterator<char>(ifs)),(istreambuf_iterator<char>()));
	ifs.close();
	

	//1. Email address
	string email = "namchen@ucdavis.edu";
	size_t found = 0;
	vector<int> foundEmailList;
	while (found != string::npos) {
		found = content.find(email, found+1);
		if (found != -1) {
			foundEmailList.push_back(found);
		} else {
			break;
		}
	}
	ofstream myfile;
	myfile.open("log_file_analyzed.txt");
	if (foundEmailList.size() != 0) {
		myfile << "###1:possible email address related data:\n" << endl; 
		for (int i = 0; i < foundEmailList.size(); i++) {
			myfile << content.substr(foundEmailList[i], 40) << endl;
			myfile << endl;
		}
	} else {
		myfile << "###1: no email found" << endl;
	}


	//2. Credit card number
	vector<long long> foundNumberList = GetStringByNumArr(content);
	if (foundNumberList.size() != 0) {
		myfile << "###2:possible credit card number related data:\n" << endl; 
		for (int i = 0; i < foundNumberList.size(); i++) {
			if (to_string(foundNumberList[i]).length() == 16) {
				myfile << foundNumberList[i] << endl;
				myfile << foundNumberList[i+1] << endl;
				myfile << foundNumberList[i+2] << endl;
				myfile << endl;
			}
		}
	} else {
		myfile << "###2: no credit card found" << endl;
	}
	

	//3. Living adress and phone number
	int state_number = 114;
	vector<int> foundStateList;
	for (int i = 0; i < state_number; i++) {
		size_t found = 0;
		while (found != string::npos) {
			found = content.find(states_initial[i], found+1);
			if (found != -1) {
				string temp1 = content.substr(found+2, 2);
				string temp2 = content.substr(found+3, 2);
				if (states_zip[i] == temp1 || states_zip[i] == temp2) {
					foundStateList.push_back(found);
				}
			} else {
				break;
			}
		}
	}
	if (foundStateList.size() != 0) {
		myfile << "###3: possible living address related data:\n" << endl; 
		for (int i = 0; i < foundStateList.size(); i++) {
			myfile << content.substr(foundStateList[i]-30,38) << endl;
			myfile <<content.substr(foundStateList[i]+9,10) << endl;
			myfile << endl;
		}
	} else {
		myfile << "###3: no states found" << endl;
	}


	//4. Server protocol password
	string protocol = "ssh";
	found = 0;
	vector<int> foundConnectionList;
	while (found != string::npos) {
		found = content.find(protocol, found+1);
		if (found != -1) {
			foundConnectionList.push_back(found);
		} else {
			break;
		}
	}
	if (foundConnectionList.size() != 0) {
		myfile << "###4:possible connection login related data:\n" << endl; 
		for (int i = 0; i < foundConnectionList.size(); i++) {
			myfile << content.substr(foundConnectionList[i], 50) << endl;
			myfile << endl;
		}
	} else {
		myfile << "###4: no connection login found" << endl;
	}

	myfile.close();
}