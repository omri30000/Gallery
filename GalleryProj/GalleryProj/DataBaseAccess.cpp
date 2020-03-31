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
			m_name = "\"";
			m_name += argv[i];
			m_name += "\"";
		}
		if (std::string(azColName[i])._Equal("LOCATION"))
		{
			m_pathOnDisk = "\"";
			m_pathOnDisk += argv[i];
			m_pathOnDisk += "\"";
		}
		if (std::string(azColName[i])._Equal("CREATION_DATE"))
		{
			m_creationDate = "\"";
			m_creationDate += argv[i];
			m_creationDate += "\"";
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
The function will get the data and cast it into list<pair<string, string>>
input: the char** base arr, number of fields, strings with the data, strings with fields names
output: 0 if succeeded
*/
int DataBaseAccess::callbackGetData(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		((list<pair<string, string>>*)data)->push_back(std::make_pair("\'" + string(azColName[i])+ "\'", "\'" + string(argv[i]) + "\'"));
	}

	return 0;
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
The function will print all the users exist in database
input: none
output: none
*/
void DataBaseAccess::printUsers()
{
	list<pair<string, string>> data;
	string username = "";

	std::string sqlStatement = "SELECT * FROM Users ;";

	if (!executeCommand(sqlStatement.c_str(), callbackGetData, &data))
	{
		//throw ItemNotFoundException(userId);
	}

	//expected value of data: list<pair<col_name, value>>

	std::cout << "Users list:" << std::endl;
	std::cout << "-----------" << std::endl;

	for (list<pair<string, string>>::iterator it = data.begin(); it != data.end(); it++) 
	{
		if (it->first._Equal("ID"))
		{
			std::cout << std::setw(5) << "   + @" << it->second << " - ";
		}
		else if (it->first._Equal("NAME"))
		{
			std::cout << it->second << std::endl;
		}
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
The function will check if a user appears in the database by it's id
input: user id
output: true or false if the album exists or not
*/
bool DataBaseAccess::doesUserExists(int userId)
{
	int rValue = false;

	std::string sqlStatement = "SELECT * FROM Users "
		"WHERE USER_ID = " + std::to_string(userId) + ";";

	executeCommand(sqlStatement.c_str(), callbackCheckExistence, &rValue);

	return rValue;
}

/*
the function will return a user from database by it's ID
input: none
output: none
*/
User DataBaseAccess::getUser(int userId)
{
	list<pair<string, string>> data;
	string username = "";

	std::string sqlStatement = "SELECT NAME FROM Users "
		"WHERE ID = " + std::to_string(userId) + ";";

	if (!executeCommand(sqlStatement.c_str(), callbackGetData, &data))
	{
		//throw ItemNotFoundException(userId);
	}
	
	//expected value of data: list<pair<col_name, value>>, the list supposed to have 1 value 
	username = data.begin()->second; // get the name of the user
	return User(userId, username);
}

/*
The function will count how many album are owned by a user
input: user object
output: amount of albums
*/
int DataBaseAccess::countAlbumsOwnedOfUser(const User& user)
{
	list<pair<string, string>> data;
	int amountOfAlbums = 0;

	std::string sqlStatement = "SELECT COUNT(NAME) AS amountOfRecords FROM Albums"
		"WHERE USER_ID = " +std::to_string(user.getId()) + ";";

	if (!executeCommand(sqlStatement.c_str(), callbackGetData, &data))
	{
		throw ItemNotFoundException(user.getName(), user.getId());
	}

	//expected value of data: list<pair<col_name, value>>, the list supposed to have 1 value 
	amountOfAlbums = atoi(data.begin()->second.c_str()); // get the amount of albums of the user
	return amountOfAlbums;
}

/*
The function will count in how many albums the user tagged
input: user object
output: amount of albums that the user is tagged in
*/
int DataBaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	//1. get list of photos' ids where the user is tagged in from Tags table
	//2. get the amount of album that these pictures are taken from

	list<pair<string, string>> data;
	string pictureIdsString = "(";
	int amountOfAlbums = 0;

	std::string sqlStatement = "SELECT DISTINCT Tags.PICTURE_ID FROM Tags"
		"WHERE USER_ID = " + std::to_string(user.getId()) + ";";

	if (!executeCommand(sqlStatement.c_str(), callbackGetData, &data))
	{
		throw ItemNotFoundException(user.getName(), user.getId());
	}
	
	//expected value of data variable currently: list<pair<"picture_id", value>>, the list supposed to have 1 value 
	list<pair<string, string>>::iterator it = data.begin();
	pictureIdsString += it->second;
	it++;
	for (it; it != data.end(); it++)
	{
		pictureIdsString += ( ", " + it->second);
		//should look like this at the end (1, 2, 3, 4) - picture ids in brackets
	}
	pictureIdsString += ")";

	sqlStatement = "SELECT COUNT(DISTINCT PICTURES.ALBUM_ID) as amountOfAlbums FROM PICTURES"
		"WHERE PICTURES.ID in " + pictureIdsString + ";";

	if (!executeCommand(sqlStatement.c_str(), callbackGetData, &data))
	{
		throw ItemNotFoundException(user.getName(), user.getId());
	}
	//expected value of data variable currently: list<pair<"amountOfAlbums", value>>, the list supposed to have 1 value 
	amountOfAlbums = atoi(data.begin()->second.c_str()); // get the amount of tags of the user
	return amountOfAlbums;
}

/*
The function count how many tags the user has in all the albums' pictures
input: user object
output: amount of tags
*/
int DataBaseAccess::countTagsOfUser(const User& user)
{
	list<pair<string, string>> data;
	int amountOfTags = 0;

	std::string sqlStatement = "SELECT COUNT(USER_ID) AS amountOfRecords FROM Tags"
		"WHERE USER_ID = " + std::to_string(user.getId()) + ";";

	if (!executeCommand(sqlStatement.c_str(), callbackGetData, &data))
	{
		throw ItemNotFoundException(user.getName(), user.getId());
	}

	//expected value of data: list<pair<col_name, value>>, the list supposed to have 1 value 
	amountOfTags = atoi(data.begin()->second.c_str()); // get the amount of tags of the user
	return amountOfTags;
}

/*
The function will calculate the average amount of tags a user has
input: user object
output: average tags of user
*/
float DataBaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	float avg = 0;


	//TODO: fill this function
	return 0.0f;
}

/*
The function will return the the user that has the maximum amount of tags
(if there are more than 1, it return randomly- add a feature to return more than one)
*/
User DataBaseAccess::getTopTaggedUser()
{
	list<pair<string, string>> data;
	int topTaggedUserId = 0;

	std::string sqlStatement = "SELECT USER_ID FROM TAGS"
		"GROUP BY USER_ID"
		"ORDER BY COUNT(USER_ID) DESC"
		"LIMIT 1;";

	if (!executeCommand(sqlStatement.c_str(), callbackGetData, &data))
	{
		throw MyException("Failed to find top tagged user");
	}

	//expected value of data: list<pair<"USER_ID", value/top_taged_user_id>>, the list supposed to have 1 value 
	topTaggedUserId = atoi(data.begin()->second.c_str()); // get the amount of tags of the user
	return getUser(topTaggedUserId);
}

/*
The function will return the the user that has the maximum amount of tags
(if there are more than 1, it return randomly- add a feature to return more than one)
*/
Picture DataBaseAccess::getTopTaggedPicture()
{
	list<pair<string, string>> data;
	int topTaggedPictureId = 0;
	string topTaggedPictureName = "";
	string topTaggedPictureCreationDate = "";
	string topTaggedPictureLocation = "";
	string sqlStatement = "SELECT * FROM PICTURES "
		"INNER JOIN TAGS "
		"ON TAGS.PICTURE_ID = PICTURES.ID "
		"GROUP BY PICTURE_ID "
		"ORDER BY COUNT(PICTURE_ID) DESC "
		"LIMIT 1;";

	if (!executeCommand(sqlStatement.c_str(), callbackGetData, &data))
	{
		throw MyException("Failed to find top tagged picture");
	}

	//expected value of data: list<pair<column_name, value/top_taged_picture>>, the list supposed to have 1 record in it 
	for (list<pair<string, string>>::iterator it = data.begin(); it != data.end(); it++)
	{
		if (it->first._Equal("ID"))
			topTaggedPictureId = atoi(it->second.c_str()); // get the ID of the top tagged picture
		else if (it->first._Equal("NAME"))
			topTaggedPictureName = it->second;
		else if (it->first._Equal("CREATION_DATE"))
			topTaggedPictureCreationDate = it->second;
		else if (it->first._Equal("LOCATION"))
			topTaggedPictureLocation = it->second;
	}

	//TODO: relate tags to picture object before returning it

	return Picture(topTaggedPictureId, topTaggedPictureName, topTaggedPictureLocation, topTaggedPictureCreationDate);
}

/*
The function will find and return the pictures that a user is tagged in
input: a user
output: a list of pictures that the user is tagged in
*/
std::list<Picture> DataBaseAccess::getTaggedPicturesOfUser(const User& user)
{
	list<Picture> pictures; // the list to return
	queue<int> pictureIds;
	queue<string> pictureNames;
	queue<string> pictureCreationDates;
	queue<string> pictureLocations;
	list<pair<string, string>> data;

	string sqlStatement = "SELECT * FROM Pictures "
		"INNER JOIN Tags "
		"ON Pictures.ID = Tags.PICTURE_ID "
		"WHERE Tags.USER_ID = 1;";

	if (!executeCommand(sqlStatement.c_str(), callbackGetData, &data))
	{
		throw ItemNotFoundException(user.getName(), user.getId());
	}

	//expected value of data: list<pair<column_name, value/top_taged_picture>>
	for (list<pair<string, string>>::iterator it = data.begin(); it != data.end(); it++)
	{
		if (it->first._Equal("ID"))
			pictureIds.push(atoi(it->second.c_str())); // get the ID of the top tagged picture
		else if (it->first._Equal("NAME"))
			pictureNames.push(it->second);
		else if (it->first._Equal("CREATION_DATE"))
			pictureCreationDates.push(it->second);
		else if (it->first._Equal("LOCATION"))
			pictureLocations.push(it->second);
	}

	while (!pictureIds.empty())
	{
		pictures.push_back(Picture(pictureIds.front(), pictureNames.front(), pictureCreationDates.front(), pictureLocations.front()));

		//remove one line
		pictureIds.pop();
		pictureNames.pop();
		pictureCreationDates.pop();
		pictureLocations.pop();
	}

	return pictures;
}
