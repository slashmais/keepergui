
#include "keepergui.h"
#include "dbk.h"
#include <utilfuncs/utilfuncs.h>
#include <HoM/HoM.h>


#include <string>


bool InitDBK()
{
	
	if (!HoMicile()) { telluser(spf("HoM: ", HoM_Message())); return false; }
	
	KEEPER_META_PATH=path_append(HoM_PATH, "keeper_meta");
	if (!dir_exist(KEEPER_META_PATH)) { telluser("Cannot find/access '", KEEPER_META_PATH, "'"); return false; }
	
	CONFIG_FILE=path_append(KEEPER_META_PATH, "keeper.conf");
	if (!BUP_CONFIG.Load(CONFIG_FILE)) { telluser("Cannot open '", CONFIG_FILE, "'"); return false; }
	
	std::string dbn{}; //database is created by keeper-app..
	dbn=path_append(KEEPER_META_PATH, "dbkeeper.sqlite3");
	if (!DBK.Open(dbn)) { telluser("Cannot open database '", dbn, "'"); return false; }
	
	return true;
}

void TermDBK() { DBK.Close(); }

GUI_APP_MAIN
{
	if (InitDBK())
	{
		KeeperGUI().Run();
		TermDBK();
	}
}
