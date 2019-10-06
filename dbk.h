#ifndef _keepgui_dbk_h_
#define _keepgui_dbk_h_

#include <dbsqlite3/dbsqlite3.h>

#include <vector>


struct Source
{
	size_t id;
	std::string sp;
	std::string fex;
	std::string dex;
};

typedef std::vector<Source> Sources;

struct DBKeep : public DBsqlite3
{
	DBKeep();
	virtual ~DBKeep();
	
	bool GetSources(Sources &SS);
	bool GetSource(size_t id, Source &S);
	bool Save(Source &S);
	bool ExistSource(const std::string &s);
	
};



extern DBKeep DBK;








#endif
