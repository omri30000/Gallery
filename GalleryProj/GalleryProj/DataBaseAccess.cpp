#include "DataBaseAccess.h"

/*
The function will open a database and set it up with tables if it's a new one
input: none
output: true or false id the operation succeeded
*/
bool DataBaseAccess::open()
{
	int fileExist = _access(this->dataBaseName.c_str(), 0);
	int res = sqlite3_open(this->dataBaseName.c_str(), &this->_db);

	if (res != SQLITE_OK)
	{
		this->_db = nullptr;
		std::cout << "Failed to open DB" << std::endl;
		return false;
	}

	if (fileExist != 0) // file doesn't exist, need to be initialized
	{
		{//Create Users table
			const char* sqlStatement = "CREATE TABLE Users("
				"ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
				"NAME TEXT NOT NULL);";

			if (!executeCommand(sqlStatement))
			{
				return false;
			}
		}
		{//Create Albums table
			const char* sqlStatement = "CREATE TABLE Albums("
				"ID INTEGER PRIMARY KEY NOT NULL, "
				"NAME TEXT NOT NULL, "
				"CREATION_DATE TEXT NOT NULL, "
				"USER_ID INTEGER, "
				"FOREIGN KEY(USER_ID) REFERENCES Users(ID)"
				");";
			if (!executeCommand(sqlStatement))
			{
				return false;
			}
		}
		{//Create Pictures table
			const char* sqlStatement = "CREATE TABLE Pictures("
				"ID INTEGER PRIMARY KEY NOT NULL, "
				"NAME TEXT NOT NULL, "
				"LOCATION TEXT NOT NULL, "
				"CREATION_DATE TEXT NOT NULL, "
				"ALBUM_ID INTEGER, "
				"FOREIGN KEY(ALBUM_ID) REFERENCES Albums(ID)"
				");";

			if (!executeCommand(sqlStatement))
			{
				return false;
			}
		}
		{//Create Tags table
			const char* sqlStatement = "CREATE TABLE Tags("
				"ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
				"PICTURE_ID INTEGER, "
				"USER_ID INTEGER, "
				"FOREIGN KEY(PICTURE_ID) REFERENCES Pictures(ID)"
				"FOREIGN KEY(USER_ID) REFERENCES Users(ID)"
				");";

			if (!executeCommand(sqlStatement))
			{
				return false;
			}
		}
	}

	std::cout << "Opened Successfully" << std::endl;
	return true;
}

/*
the function will close the data base
input: none
output: none
*/
void DataBaseAccess::close()
{
	sqlite3_close(this->_db);
	this->_db = nullptr;
}

bool DataBaseAccess::executeCommand(const char* statement)
{
	int res = 0;

	char** errMessage = nullptr;
	res = sqlite3_exec(this->_db, statement, nullptr, nullptr, errMessage);

	if (res != SQLITE_OK)
	{
		//std::cout << "Error in creating Users table" << std::endl;
		//system("pause");
		return false;
	}

	return true;
}

/*
the function is the constructor of dataBaseAccess object
input: none
output: none
*/
DataBaseAccess::DataBaseAccess()
{
	this->_db = nullptr;
}
