#include "DataBaseAccess.h"

/*
the function is the constructor of dataBaseAccess object
input: none
output: none
*/
DataBaseAccess::DataBaseAccess()
{
	this->_db = nullptr;
}

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
		createTables();
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

/*
The function will get sql statement and execute it on the db of the class
input: sql statement
output: true or false if everything went fine
*/
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
The function will create the tables of the gallery in the database
input: none
output: true or false if the tables created successfuly
*/
bool DataBaseAccess::createTables()
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
			"FOREIGN KEY(ALBUM_ID) REFERENCES Albums(ID) ON DELETE CASCADE"
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
			"FOREIGN KEY(PICTURE_ID) REFERENCES Pictures(ID) ON DELETE CASCADE"
			"FOREIGN KEY(USER_ID) REFERENCES Users(ID)"
			");";

		if (!executeCommand(sqlStatement))
		{
			return false;
		}
	}

	return true;
}

/*
The function will delete an existing album from database
input: album name and a user ID
output: none
*/
void DataBaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	//TODO: check "ON DELETE CASCADE" works

	//delete album
	std::string sqlStatement = "DELETE FROM Albums "
		"WHERE USER_ID = " +std::to_string(userId) + " AND NAME = " + albumName + ";";

	executeCommand(sqlStatement.c_str());
	//TODO: if executeCommand fails, throw "albumDeletionException"
}

/*
the function will get an album to close and remove it from storage
input: an album to close
output: none
*/
void DataBaseAccess::closeAlbum(Album& pAlbum)
{
	//this function do nothing for real
}

/*
The function will relate a picture to an album in the database
input: album name and picture object
output: none
*/
void DataBaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
	//insert picture to pictures and relate it to albums
	std::string sqlStatement = "INSERT INTO Pictures (ID, NAME, LOCATION, CREATION_DATE, ALBUM_ID) "
		"VALUES (" + std::to_string(picture.getId()) + ", " + picture.getName() + ", " + picture.getPath() + ", "
		"" + picture.getCreationDate() + ", SELECT ID FROM Albums WHERE NAME = " + albumName + ");";

	executeCommand(sqlStatement.c_str());
	//TODO: if executeCommand fails, throw "PictureAddingException"
}


/*
The function will remove a picture record from database
input: album name, picture name
output: none
*/
void DataBaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	//delete picture
	std::string sqlStatement = "DELETE FROM Pictures "
		"WHERE NAME = " + pictureName + " AND ALBUM_ID = SELECT ID FROM ALBUMS WHERE NAME = " + albumName + ";";

	if (!executeCommand(sqlStatement.c_str()))
	{
		throw ItemNotFoundException(pictureName, -1);
	}
	
}
