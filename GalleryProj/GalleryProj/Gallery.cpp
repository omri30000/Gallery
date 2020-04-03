#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include "MemoryAccess.h"
#include "DataBaseAccess.h"
#include "AlbumManager.h"

/*
The function will be called when Ctrl+C is pressed
*/
BOOL WINAPI consoleHandler(DWORD signal, std::string procName)
{
	if (signal == CTRL_C_EVENT)
	{
		exit(1);
	}
	return true;
}

int getCommandNumberFromUser()
{
	std::string message("\nPlease enter any command(use number): ");
	std::string numericStr("0123456789");
	
	std::cout << message << std::endl;
	std::string input;
	std::getline(std::cin, input);
	
	while (std::cin.fail() || std::cin.eof() || input.find_first_not_of(numericStr) != std::string::npos) {

		std::cout << "Please enter a number only!" << std::endl;

		if (input.find_first_not_of(numericStr) == std::string::npos) {
			std::cin.clear();
		}

		std::cout << std::endl << message << std::endl;
		std::getline(std::cin, input);
	}
	
	return std::atoi(input.c_str());
}

/*
the function will print the opening message
input: none
output: none
*/
void printOpeningMsg()
{
	//the time now
	std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	std::string albumName;
	std::cout << "Welcome to Gallery created by Omri Zaiman!" << std::endl;
	std::cout << "==========================================" << std::endl;
	std::cout << "The Time is: " << std::ctime(&time);
	std::cout << "==========================================" << std::endl;
	std::cout << "Type " << HELP << " to a list of all supported commands" << std::endl;
}

int main(void)
{
	// initialization data access
	DataBaseAccess dataAccess;

	// initialize album manager
	AlbumManager albumManager(dataAccess);

	printOpeningMsg();

	do {
		int commandNumber = getCommandNumberFromUser();
		try	{	
			albumManager.executeCommand(static_cast<CommandType>(commandNumber));
		} catch (std::exception& e) {	
			std::cout << e.what() << std::endl;
		}
	} 
	while (true);

	return 0;
}