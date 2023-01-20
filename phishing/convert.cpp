#include <stdio.h>
#include <iostream>
#include <string>

void convert() {
	double usdToJpy = 107.61;
	double jpyToUsd = 0.0093;
	
	std::cout << "Available currencies: [USD] [JPY] [CNY]" << std::endl;
	
	std::cout << "Convert from:" << std::endl;
	std::string convert_from;
	std::getline(std::cin, convert_from);
	
	std::cout << "Amount?" << std::endl;
	std::string amountStr;
	std::getline(std::cin, amountStr);
	double amount = atof(amountStr.c_str());
	
	std::cout << "Convert to:" << std::endl;
	std::string convertTo;
	std::getline(std::cin, convertTo);
	
	double result=amount * usdToJpy;
	
	std::cout << "--------------------------" << std::endl;
	std::cout << "\nLatest exchange rate: " << usdToJpy << std::endl;
	printf("%.2f %s = %.2f %s \n", amount, convert_from.c_str(), result, convertTo.c_str());
	std::cout << "\n--------------------------" << std::endl;
}
