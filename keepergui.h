#ifndef _keepgui_keepgui_h
#define _keepgui_keepgui_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define LAYOUTFILE <keepergui/keepgui.lay>
#include <CtrlCore/lay.h>

#include <string>
#include <map>
#include <utilfuncs/utilfuncs.h>
#include <qdconfig/qdconfig.h>


extern QDConfig BUP_CONFIG;
extern std::string KEEPER_META_PATH;
extern std::string CONFIG_FILE;
extern std::string DEF_BUP_DIR;

inline bool iszcompressed(const std::string &f) { return ((f[f.size()-2]=='.')&&((f[f.size()-1]|32)=='z')); }

struct KeeperGUI : public WithkeepguiLayout<TopWindow>
{
	typedef KeeperGUI CLASSNAME;

	size_t curid;
	
	KeeperGUI();
	
	//void OnSelectionChange();
	void ChangeBupLocation();

	void editbup();

	void OnRemove();
	bool do_restore(const std::string &f, const DirEntries &md, const std::string &t);
	void OnRestore();
	void ShowBackups();

	void InitACBups();
	void GetLastTotal(std::string sbupdir, std::string &sL, std::string &sT);
	void FillACBups();
	
	void OnACMenu(Bar &bar);
	void OnAbout();
};

struct BUPDLG : public WithbupdlgLayout<TopWindow>
{
	typedef BUPDLG CLASSNAME;
	bool b;
	size_t ID;
	virtual ~BUPDLG();
	BUPDLG(size_t id=0);
	void showdata();
	void OnSelDir();
	void OnOK();
	void OnCancel();
};


//#include <TreeGrid/treegrid.h> fucking shit does not work

struct Panel : public Ctrl
{
	typedef Panel CLASSNAME;
	Button btnClose;
	Button btnCopyFile;			// this tool is aimed mainly at decompressing files
	//Button btnCopyAllCurrent; --- can go & copy direct from bup-loc using file-manager
	Panel()
	{
		Add(btnClose.SetLabel(t_("Close")).RightPosZ(12, 60).BottomPosZ(12, 20));
		Add(btnCopyFile.SetLabel(t_("Copy/Extract ..")).RightPosZ(84, 90).BottomPosZ(12, 20));
	}
	virtual ~Panel(){}
	void Paint(Draw &drw) { drw.DrawRect(GetSize(), SColorFace()); }
};
struct BupData : public WithbupdataLayout<TopWindow>
{
	typedef BupData CLASSNAME;
	
	std::string bdir;
	int root;
	TreeCtrl tree;
	DocEdit doc;
	Panel pan;
	Splitter HSplit;
	Splitter VSplit;
	
	BupData(const std::string &B);
	virtual ~BupData() {}
	//void Layout();
	
	void FillTree(int P, const DirTree &dt);
	void ShowFile();
	
	void panCopyFile();
};

void ShowBupData(const std::string &B);

#endif
