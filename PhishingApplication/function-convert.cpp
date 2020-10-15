
#include <stdio.h>
#include <iostream>
#include <string>
using namespace std;

void convert(){
	
	double usdjpy=107.61;
	double jpyusd=0.0093;
	
	cout<<"Available currencies:[USD] [JPY] [CNY]"<<endl;
	
	cout<<"Convert from:"<<endl;
	string cf;
	getline(cin,cf);
	
	cout<<"Amount?"<<endl;
	string am;
	getline(cin,am);
	double amount = atof(am.c_str());
	
	cout<<"Convert to:"<<endl;
	string ct;
	getline(cin,ct);
	
	
	double result=amount * usdjpy;
	
	cout<<"--------------------------"<<endl;
	cout<<"\nLatest exchange rate: "<<usdjpy<<endl;
	printf("%.2f %s = %.2f %s \n",amount,cf.c_str(),result,ct.c_str());
	cout<<"\n--------------------------"<<endl;
}
