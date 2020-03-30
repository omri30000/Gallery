#pragma once
#include "sqlite3.h"
#include "IDataAccess.h"
#include "ItemNotFoundException.h"
#include <io.h>


class DataBaseAccess : public IDataAccess
{
private:
	const std::string dataBaseName = "MyDB.sqlite";
	sqlite3* _db;

public:
	DataBaseAccess();
	virtual ~DataBaseAccess() = default;

	// album related
	const std::list<Album> getAlbums() override;
	const std::list<Album> getAlbumsOfUser(const User & user) override;
	void createAlbum(const Album & album) override;
	void deleteAlbum(const std::string & albumName, int userId) override;
	bool doesAlbumExists(const std::string & albumName, int userId) override;
	Album openAlbum(const std::string & albumName) override;
	void closeAlbum(Album & pAlbum) override;
	void printAlbums() override;

	// picture related
	void addPictureToAlbumByName(const std::string & albumName, const Picture & picture) override;
	void removePictureFromAlbumByName(const std::string & albumName, const std::string & pictureName) override;
	void tagUserInPicture(const std::string & albumName, const std::string & pictureName, int userId) override;
	void untagUserInPicture(const std::string & albumName, const std::string & pictureName, int userId) override;

	// user related
	void printUsers() override;
	void createUser(User & user) override;
	void deleteUser(const User & user) override;
	bool doesUserExists(int userId) override;
	User getUser(int userId) override;

	// user statistics
	int countAlbumsOwnedOfUser(const User & user) override;
	int countAlbumsTaggedOfUser(const User & user) override;
	int countTagsOfUser(const User & user) override;
	float averageTagsPerAlbumOfUser(const User & user) override;

	// queries
	User getTopTaggedUser() override;
	Picture getTopTaggedPicture() override;
	std::list<Picture> getTaggedPicturesOfUser(const User & user) override;

	bool open() override;
	void close() override;
	void clear() override {}; // this function doesn't have to clear anything 

	//temp functions
	bool executeCommand(const char* statement);
	bool createTables();
};