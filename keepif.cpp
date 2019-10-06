
#include "keepif.h"
#include <utilfuncs/utilfuncs.h>
#include "dbk.h"



bool KeepSave(size_t id, const std::string &s, const std::string &f, const std::string &d)
{
	bool b=false;
	Source S;
	S.id=id;
	S.sp=s;
	S.fex=f;
	S.dex=d;
	if (!id&&DBK.ExistSource(s)) b=tellerror("Directory '", s, "' already backed-up");
	else if (!(b=DBK.Save(S))) b=tellerror("Failed to add backup: ", DBK.GetLastError());
	return b;
}

