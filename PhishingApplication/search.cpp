#define MOD "Module"

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main(){
	string str;
	ifstream modfile;
	size_t pos;
	
	system("lsmod>mod.txt");
	modfile.open("mod.txt");
	cout<< "file opened\n";
	
	if(modfile.is_open()){
		while(getline(modfile,str)){
		pos=str.find(MOD);
		if(pos!=string::npos){
			cout<<"found!\n";
			break;
			}
		}
	}
	modfile.close();
	system("rm -r -f mod.txt");
	return 0;
}


