#include "DataBaseAccess.h"

/*
The function is a callback function that will cast the sql results to an album list
input: the list to set, number of fields, strings with the data, strings with fields names
output: 0 if succeeded
*/
int DataBaseAccess::callbackDataToAlbumList(void* data, int argc, char** argv, char** azColName)
{
	int m_ownerId{ 0 };
	int albumID{ 0 };
	std::string m_name;
	std::string m_creationDate;
	
	//get all details except for pictures.
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i])._Equal("USER_ID"))
		{
			m_ownerId = atoi(argv[i]);
		}
		else if (std::string(azColName[i])._Equal("NAME"))
		{
			m_name = argv[i];
		}
		else if (std::string(azColName[i])._Equal("CREATION_DATE"))
		{
			m_creationDate = argv[i];
		}
		else if (std::string(azColName[i])._Equal("ID"))
		{
			albumID = atoi(argv[i]);
		}
	}

	((std::list<std::pair<Album, int>>*)data)->push_back(std::make_pair(Album(m_ownerId, m_name, m_creationDate), albumID));
	
	return 0;
}

/*
The function is a callback function that will cast the sql results to a pictures list
input: the list to set, number of fields, strings with the data, strings with fields names
output: 0 if succeeded
*/
int DataBaseAccess::callbackDataToPictureList(void* data, int argc, char** argv, char** azColName)
{
	int m_pictureId = 0;
	std::string m_name;
	std::string m_pathOnDisk;
	std::string m_creationDate;
	int albumID = -1;

	//get all details except for tags.
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i])._Equal("ID"))
		{
			m_pictureId = atoi(argv[i]);
		}
		if (std::string(azColName[i])._Equal("NAME"))
		{
			m_name = argv[i];
		}
		if (std::string(azColName[i])._Equal("LOCATION"))
		{
			m_pathOnDisk = argv[i];
		}
		if (std::string(azColName[i])._Equal("CREATION_DATE"))
		{
			m_creationDate = argv[i];
		}
		if (std::string(azColName[i])._Equal("ALBUM_ID"))
		{
			albumID = atoi(argv[i]);
		}
	}
	//std::queue<std::pair<Picture, std::pair<int,int>>>
	((std::list<std::pair<Picture, int>>*)data)->push_back(std::make_pair(Picture(m_pictureId, m_name, m_pathOnDisk, m_creationDate), albumID));

	return 0;
}

/*
The function is a callback function that will cast the sql results to a tags list
input: the list to set, number of fields, strings with the data, strings with fields names
output: 0 if succeeded
*/
int DataBaseAccess::callbackDataToTagList(void* data, int argc, char** argv, char** azColName)
{
	int pictureID = 0;
	int userID = 0;

	//get all details except for tags.
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i])._Equal("PICTURE_ID"))
		{
			pictureID = atoi(argv[i]);
		}
		if (std::string(azColName[i])._Equal("USER_ID"))
		{
			userID = atoi(argv[i]);
		}
	}

	((std::list<std::pair<int, int>>*)data)->push_back(std::make_pair(pictureID, userID));

	return 0;
}

/*
The function will check if a sql statement returns 1 value or more
input: the list to set, number of fields, strings with the data, strings with fields names
output: 0 if succeeded
*/
int DataBaseAccess::callbackCheckExistence(void* data, int argc, char** argv, char** azColName)
{
	//if this function was called it means that there are 1 or more records
	*(bool*)data = true;
}

/*
the function is the constructor of dataBaseAccess object
input: none
output: none
*/
DataBaseAccess::DataBaseAccess()
{
	this->m_albums.clear();
	this->m_users.clear();
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
The function will clear all the memory that is saved in the class
input: none
output: none
*/
void DataBaseAccess::clear()
{
	this->m_albums.clear();
	this->m_users.clear();
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
The function will get sql statement and execute it on the db of the class
input: sql statement
output: true or false if everything went fine
*/
bool DataBaseAccess::executeCommand(const char* statement, int (*callback)(void*, int, char**, char**), void* arg)
{
	int res = 0;

	char** errMessage = nullptr;
	res = sqlite3_exec(this->_db, statement, callback, arg, errMessage);

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
			"ID INTEGER PRIMARY KEY NOT NULL, "
			"NAME TEXT NOT NULL);";

		if (!executeCommand(sqlStatement))
		{
			return false;
		}
	}
	{//Create Albums table
		const char* sqlStatement = "CREATE TABLE Albums("
			"ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
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
			"FOREIGN KEY(USER_ID) REFERENCES Users(ID) ON DELETE CASCADE"
			");";

		if (!executeCommand(sqlStatement))
		{
			return false;
		}
	}

	return true;
}

/*
The function will set the list of albums of the class with data from the database and return it
input: none
output: all albums list
*/
const std::list<Album> DataBaseAccess::getAlbums()
{
	std::list<std::pair<Album, int>> tempAlbums;
	std::queue<std::pair<Picture, int>> tempPictures;
	std::list<std::pair<int, int>> tempTags; //<pictureID, USER_ID>
	std::string sqlStatement = "SELECT * FROM Albums;";
	
	this->m_albums.clear();// clear previous data

	executeCommand(sqlStatement.c_str(), callbackDataToAlbumList, &tempAlbums);
	// here tempAlbums should contain the details of the album without the pictures

	sqlStatement = "SELECT * FROM Pictures;";
	executeCommand(sqlStatement.c_str(), callbackDataToPictureList, &tempPictures);

	sqlStatement = "SELECT * FROM Tags;";
	executeCommand(sqlStatement.c_str(), callbackDataToTagList, &tempTags);


	// relate pictures to albums
	while (!tempPictures.empty())
	{
		int pictureAlbumID = tempPictures.front().second; // the albumID in the picture object
		
														  //realte pictures and tags
		for (std::list<std::pair<int, int>>::iterator ite = tempTags.begin(); ite != tempTags.end(); ite++)
		{
			if (ite->first == tempPictures.front().first.getId()) // the picture id is the picture id in the tag
			{
				tempPictures.front().first.tagUser(ite->second);
			}
		}

		//run on the albums
		for (std::list<std::pair<Album, int>>::iterator it = tempAlbums.begin(); it != tempAlbums.end(); it++)
		{
			if (pictureAlbumID == it->second) // the picture belongs to this album
			{ 
				it->first.addPicture(tempPictures.front().first);//add picture to album			
			}
		}

		tempPictures.pop();
	}

	//cast std::list<std::pair<Album, int>> to std::list<Album>
	{
		for (std::list<std::pair<Album, int>>::iterator it = tempAlbums.begin(); it != tempAlbums.end(); it++)
		{
			this->m_albums.push_back(it->first);
		}
	}

	return this->m_albums;
}

/*
The function will set the list of albums of the class with data from the database and return it
input: none
output: all albums list
*/
const std::list<Album> DataBaseAccess::getAlbumsOfUser(const User& user)
{
	std::list<std::pair<Album, int>> tempAlbums;
	std::queue<std::pair<Picture, int>> tempPictures;
	std::list<std::pair<int, int>> tempTags; //<pictureID, USER_ID>
	std::string sqlStatement = "SELECT * FROM Albums WHERE USER_ID = " + std::to_string(user.getId()) + ";";

	this->m_albums.clear();// clear previous data

	executeCommand(sqlStatement.c_str(), callbackDataToAlbumList, &tempAlbums);
	// here tempAlbums should contain the details of the album without the pictures

	sqlStatement = "SELECT * FROM Pictures WHERE ALBUM_ID = " + std::to_string(tempAlbums.begin()->second) + ";";
	executeCommand(sqlStatement.c_str(), callbackDataToPictureList, &tempPictures);

	sqlStatement = "SELECT * FROM Tags WHERE USER_ID = " + std::to_string(user.getId()) + ";";
	executeCommand(sqlStatement.c_str(), callbackDataToTagList, &tempTags);


	// relate pictures to albums
	while (!tempPictures.empty())
	{
		int pictureAlbumID = tempPictures.front().second; // the albumID in the picture object

														  //realte pictures and tags
		for (std::list<std::pair<int, int>>::iterator ite = tempTags.begin(); ite != tempTags.end(); ite++)
		{
			if (ite->first == tempPictures.front().first.getId()) // the picture id is the picture id in the tag
			{
				tempPictures.front().first.tagUser(ite->second);
			}
		}

		//run on the albums
		for (std::list<std::pair<Album, int>>::iterator it = tempAlbums.begin(); it != tempAlbums.end(); it++)
		{
			if (pictureAlbumID == it->second) // the picture belongs to this album
			{
				it->first.addPicture(tempPictures.front().first);//add picture to album			
			}
		}

		tempPictures.pop();
	}

	//cast std::list<std::pair<Album, int>> to std::list<Album>
	for (std::list<std::pair<Album, int>>::iterator it = tempAlbums.begin(); it != tempAlbums.end(); it++)
	{
		this->m_albums.push_back(it->first);
	}

	return this->m_albums;
}

/*
The function will insert a new album to the database
input: an album to insert
output: none
*/
void DataBaseAccess::createAlbum(const Album& album)
{
	std::string sqlStatement = "INSERT INTO Albums (NAME, CREATION_DATE, USER_ID)"
		"VALUES (" + album.getName() + ", " + album.getCreationDate() + ", " + std::to_string(album.getOwnerId()) + ");";

	executeCommand(sqlStatement.c_str());
	//TODO: throw exception if it doesn't work
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
The function will check if an album appears in the database by it's name
input: album name, user id
output: true or false if the album exists or not
*/
bool DataBaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	int rValue = false;

	std::string sqlStatement = "SELECT * FROM Albums "
		"WHERE USER_ID = " + std::to_string(userId) + " AND NAME = " + albumName + ";";

	executeCommand(sqlStatement.c_str(), callbackCheckExistence, &rValue);

	return rValue;
}

/*
The function will load an album from database and open it
!for now, the function will open the first album that appears in the DB because we don't know which user is connected
input: album name
output: album object
*/
Album DataBaseAccess::openAlbum(const std::string& albumName)
{
	this->m_albums.clear();
	auto a = getAlbums();
	
	for (std::list<Album>::iterator it = this->m_albums.begin(); it != this->m_albums.end(); it++)
	{
		if(it->getName()._Equal(albumName))
		{
			return *it;
		}
	}
	throw MyException("Invalid Album Name");
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
The function will print all the albums exists
input: none
output: none
*/
void DataBaseAccess::printAlbums()
{
	this->m_albums.clear();
	auto a = getAlbums();

	if (m_albums.empty()) {
		throw MyException("There are no existing albums.");
	}
	std::cout << "Album list:" << std::endl;
	std::cout << "-----------" << std::endl;
	for (const Album& album : m_albums) {
		std::cout << std::setw(5) << "* " << album;
	}
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

/*
The function will add a tag to the tags table
input: picture name and user id
output: none
*/
void DataBaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	//insert tag to tags and relate it to Pictures and Users
	std::string sqlStatement = "INSERT INTO Tags (PICTURE_ID, USER_ID) "
		"VALUES (SELECT ID FROM Pictures WHERE NAME = "+ pictureName +", "+ std::to_string(userId) +");";
	//TODO: fix statement so it will work when there are more than 1 pictures with the same name
	executeCommand(sqlStatement.c_str());
	//TODO: if executeCommand fails, throw "PictureAddingException"
}

/*
the function will remove a tag record from tags table in database
input: an album name, picture name, and user ID
output: none
*/
void DataBaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	//delete picture
	std::string sqlStatement = "DELETE FROM Tags "
		"WHERE PICTURE_ID = SELECT ID FROM Pictures WHERE NAME = " + pictureName + " AND USER_ID = " + std::to_string(userId) + ";";

	if (!executeCommand(sqlStatement.c_str()))
	{
		throw ItemNotFoundException(pictureName, -1);
	}
}

/*
The function inserts user to Users table in database
input: user object
output: none
*/
void DataBaseAccess::createUser(User& user)
{
	//insert user to Users
	std::string sqlStatement = "INSERT INTO Users (ID, NAME) "
		"VALUES (" + std::to_string(user.getId()) + ", " + user.getName() + ");";

	executeCommand(sqlStatement.c_str());
	//TODO: if executeCommand fails, throw "UserAddingException"
}

/*
The function will delete user record from Users table in database
input: user object
output: none
*/
void DataBaseAccess::deleteUser(const User& user)
{
	//delete picture
	std::string sqlStatement = "DELETE FROM Users "
		"WHERE ID = " + std::to_string(user.getId()) + ";";

	if (!executeCommand(sqlStatement.c_str()))
	{
		throw ItemNotFoundException(user.getName(), user.getId());
	}
}

/*
The function will calculate the average amount of tags a user has
input: user object
output: average tags of user
*/
float DataBaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	return 0.0f;
}
