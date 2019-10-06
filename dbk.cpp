
#include "dbk.h"
#include <utilfuncs/utilfuncs.h>


DBKeep DBK;


DBKeep::DBKeep() {}

DBKeep::~DBKeep() {}

bool DBKeep::GetSources(Sources &SS)
{
	DBResult RS;
	bool b=false;
	size_t i=0, n;
	SS.clear();
	n=ExecSQL(&RS, "SELECT id, sourcedir, direxcludes, fileexcludes FROM sources ORDER BY id ASC");
	if (NoError(this))
	{
		while (i<n)
		{
			Source S;
			S.id=stot<size_t>(RS.GetVal("id", i));
			S.sp=SQLRestore(RS.GetVal("sourcedir", i));
			S.fex=SQLRestore(RS.GetVal("fileexcludes", i));
			S.dex=SQLRestore(RS.GetVal("direxcludes", i));
			SS.push_back(S);
			i++;
		}
		b=true;
	}
	return b;
}

bool DBKeep::GetSource(size_t id, Source &S)
{
	DBResult RS;
	ExecSQL(&RS, spf("SELECT * FROM sources WHERE id=", id));
	if (NoError(this))
	{
		S.id=stot<size_t>(RS.GetVal("id", 0));
		S.sp=SQLRestore(RS.GetVal("sourcedir", 0));
		S.fex=SQLRestore(RS.GetVal("fileexcludes", 0));
		S.dex=SQLRestore(RS.GetVal("direxcludes", 0));
		return true;
	}
	return false;
}

bool DBKeep::Save(Source &S)
{
	std::string sSQL;
	DBResult RS;
	if (S.id)
	{
		sSQL=spf("UPDATE sources SET",
				" sourcedir = ", SQLSafeQ(S.sp),
				", direxcludes = ", (S.dex.empty()?"''":SQLSafeQ(S.dex)),
				", fileexcludes = ", (S.fex.empty()?"''":SQLSafeQ(S.fex)),
				" WHERE id = ", S.id);
		ExecSQL(&RS, sSQL);
	}
	else
	{
		S.id=new_id("sources");
		sSQL=spf("INSERT INTO sources(id, sourcedir, direxcludes, fileexcludes) VALUES(",
					S.id,
					", ", SQLSafeQ(S.sp),
					", ", SQLSafeQ(S.dex),
					", ", SQLSafeQ(S.fex), ")");
		ExecSQL(&RS, sSQL);
	}
	return (NoError(this));
}

bool DBKeep::ExistSource(const std::string &s)
{
	DBResult RS;
	std::string SQL;
	size_t n;
	SQL=spf("SELECT * FROM sources WHERE sourcedir = ", SQLSafeQ(s));
	n=ExecSQL(&RS, SQL);
	return (NoError(this)&&(n>0));
}


