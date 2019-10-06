
#include "keepergui.h"
#include <string>
#include <utilfuncs/utilfuncs.h>
#include "dbk.h"
#include "keepif.h"


#define IMAGEFILE <keepergui/keepgui.iml>
#define IMAGECLASS PICS
#include <Draw/iml.h>
//#include <Draw/iml_source.h>

#include <string>


//----------------------------------------------------------------------------------------------
QDConfig BUP_CONFIG;
std::string KEEPER_META_PATH;
std::string CONFIG_FILE;
std::string DEF_BUP_DIR;

//----------------------------------------------------------------------------------------------
BUPDLG::~BUPDLG() {}

BUPDLG::BUPDLG(size_t id)
{
	ID=id;
	CtrlLayout(*this, "File/Dir-excludes");
	CenterOwner(); b=false;
	ebDir.Enable(!ID);
	btnSelDir.Enable(!ID);
	btnSelDir.WhenPush=THISBACK(OnSelDir);
	btnOK.WhenPush=THISBACK(OnOK);
	btnCancel.WhenPush=THISBACK(OnCancel);
	if (ID) showdata();
}

void BUPDLG::showdata()
{
	if (ID)
	{
		Source S;
		if (DBK.GetSource(ID, S))
		{
			ebDir.SetData(S.sp.c_str());
			ebF.SetData(S.fex.c_str());
			ebD.SetData(S.dex.c_str());
		}
	}
	else { ebDir.Clear(); ebF.Clear(); ebD.Clear(); }
}

void BUPDLG::OnSelDir() { String S=SelectDirectory(); if (!S.IsEmpty()) ebDir.SetData(S); }
void BUPDLG::OnOK() { b=true; Close(); } //validation...todo...by caller?
void BUPDLG::OnCancel() { Close(); }

//----------------------------------------------------------------------------------------------
KeeperGUI::KeeperGUI()
{
	CtrlLayout(*this, "Keeper Gui");
	SetRect(0,0,1000,800);
	Sizeable();
	curid=0;
	//acBups.WhenSel = [&]{ OnSelectionChange(); };
	acBups.WhenBar = [&](Bar &bar){ OnACMenu(bar); };
	btnBupLocation.SetImage(PICS::PicDirSel());
	btnBupLocation.WhenAction << [&]{ ChangeBupLocation(); };
	DEF_BUP_DIR=BUP_CONFIG.getval("defbupdir");
	ebDBD.SetData(DEF_BUP_DIR.c_str());
	InitACBups();
}

//void KeeperGUI::OnSelectionChange()
//{
//	int idx=acBups.GetCursor();
//	if (idx>=0)
//	{
//		curid=stot<size_t>(acBups.Get(idx, 0).ToString().ToStd());
//	}
//}

void KeeperGUI::ChangeBupLocation()
{
	if (PromptOKCancel(DeQtf("All data from the current location will be sync'd to the new location\nand may take some time..\n\nAre you sure you want to change the Backup location?\n")))
	{
		String S=SelectDirectory();
		if (!S.IsEmpty())
		{
			std::string snew;
			snew=S.ToStd();
			if (dir_sync(DEF_BUP_DIR, snew))
			{
				BUP_CONFIG.setval("defbupdir", snew);
				BUP_CONFIG.Save();
				DEF_BUP_DIR=snew;
				ebDBD.SetData(DEF_BUP_DIR.c_str());
			}
			else telluser("FAILED to change backup location");
		}
	}
}

void KeeperGUI::editbup()
{
	BUPDLG dlg(curid);
	dlg.Execute();
	if (dlg.b)
	{
		std::string s{}, f{}, d{};
		s=dlg.ebDir.GetData().ToString().ToStd();
		f=dlg.ebF.GetData().ToString().ToStd();
		d=dlg.ebD.GetData().ToString().ToStd();
		if (dir_exist(s)&&KeepSave(curid, s, f, d)) { Sys("keeper restart"); FillACBups(); }
	}
}

void KeeperGUI::OnRemove()
{
	int idx=acBups.GetCursor();
	if (idx>=0)
	{
		std::string s{}, d{};
		d=acBups.Get(idx, 2).ToString().ToStd();
		if (PromptOKCancel(DeQtf(spf("keeper will stop monitoring directory '", d, "'\n\nAre you sure?\n").c_str())))
		{
			s=spf("keeper --remove ", d);
			Sys(s.c_str());
			FillACBups();
		}
	}
}

bool KeeperGUI::do_restore(const std::string &f, const DirEntries &md, const std::string &t)
{
	bool b=true;
	if (!md.empty())
	{
		std::string sf, st, sz;
		auto it=md.begin();
		while (b&&(it!=md.end()))
		{
			sf=path_append(f, it->first);
			if (!iszcompressed(sf)) //only current
			{
				st=path_append(t, it->first);
				if (isdirtype(it->second))
				{
					DirEntries me;
					if (dir_read(sf, me)) { if ((b=dir_create(st))) { b=do_restore(sf, me, st); }}
					else report_error(spf("cannot restore '", sf, "' -", filesys_error()), true); //and carry-on
				}
				else if (isfiletype(it->second)) { b=file_copy(sf, st); }
			}
			it++;
		}
	}
	return b;
}

void KeeperGUI::OnRestore()
{
	int idx=acBups.GetCursor();
	if (idx>=0)
	{
		std::string s{}, d{}, t{};
		d=acBups.Get(idx, 2).ToString().ToStd();
		String T=SelectDirectory();
		if (T.IsEmpty()) return;
		WaitCursor WC();
		t=T.ToStd();
		d=path_name(d);
		s=path_append(DEF_BUP_DIR, d);
		t=path_append(t, d);
		if (!path_realize(t)) { report_error(spf("cannot create restore-dir: '", t, "'"), true); return; }
		Sys("keeper stop"); //now OK if restoring to a monitored directory
		DirEntries md;
		dir_read(s, md);
		do_restore(s, md, t);
		Sys("keeper"); //get the show on the road again
	}
}

void KeeperGUI::ShowBackups()
{
	int idx=acBups.GetCursor();
	if (idx>=0)
	{
		std::string B{}, d{};
		d=acBups.Get(idx, 2).ToString().ToStd();
		B=path_append(DEF_BUP_DIR, path_name(d));
		ShowBupData(B);
	}
}

void KeeperGUI::InitACBups()
{
	acBups.Reset();
	acBups.AddColumn("#", 25);
	acBups.AddColumn("Status", 25);
	acBups.AddColumn("Source", 150);
	acBups.AddColumn("File-ex", 50);
	acBups.AddColumn("Dir-ex", 50);
	acBups.AddColumn("Last update", 70); //date-time of last backup
	acBups.AddColumn("#Files", 30).SetDisplay(StdRightDisplay()); //count of files backed-up
	FillACBups();
}

size_t count_entries(const DirTree &dt)
{
	size_t n=0;//dt.size();
	n+=dt.content.size();
	for (auto p:dt) n+=count_entries(p.second);
	return n;
}

void KeeperGUI::GetLastTotal(std::string sbupdir, std::string &sL, std::string &sT)
{
	sL=path_time_h(sbupdir);
	size_t t=0;
	DirTree DT; //'tis a bad way to do this..
	DT.Read(sbupdir);
	t=count_entries(DT);
	sT=ttos<size_t>(t);
}

void KeeperGUI::FillACBups()
{
	acBups.Clear();
	std::string s{};
	Sources SS;
	if (DBK.GetSources(SS))
	{
		for (auto S:SS)
		{
			std::string status{"OK"}, sB{}, sL{}, sT{};
			if (!dir_exist(S.sp)) status="?";
			sB=path_append(DEF_BUP_DIR, path_name(S.sp));
			GetLastTotal(sB, sL, sT);
			acBups.Add(spf(S.id).c_str(), status.c_str(), S.sp.c_str(), S.fex.c_str(), S.dex.c_str(), sL.c_str(), sT.c_str());
		}
	}
}

void KeeperGUI::OnACMenu(Bar &bar)
{
	int idx=acBups.GetCursor();
	if (idx>=0) { curid=stot<size_t>(acBups.Get(idx, 0).ToString().ToStd()); }
	bar.Add("Add..", [&]{ curid=0; editbup(); });
	bar.Separator();
	bar.Add((curid>0), "Edit..", [&]{ editbup(); });
	bar.Add((curid>0), "Show backups..", [&]{ ShowBackups(); });
	bar.Add((curid>0), "Restore", [&]{ OnRestore(); });
	bar.Separator();
	bar.Add((curid>0), "Remove", [&]{ OnRemove(); });
	bar.Separator();
	bar.Add("About keeper", [&]{ OnAbout(); });
	bar.Separator();
	bar.Add("Exit", [&]{ Ctrl::Close(); });
}

void KeeperGUI::OnAbout()
{
	std::string s;
	s="[* [@(10.70.10) keeper]] is a tool to make real-time backups of directories.&"
		"&[* [@(10.70.10) KeeperGui]] (this app) provide methods to specify and recover real-time backups,&"
		"while keeper itself runs as a daemon to do the work. (Can also be done via CLI)&"
		"&&You supply the directory that you want to backup as changes occur within it,&"
		"and wildcard-templates (using * and ?) for files/sub-directories to be excluded&"
		"&The database, config, and logs for Keeper is located at:&    [!monospace! [+64 \1";
	s+=KEEPER_META_PATH; s+="\1]]&";
	s+="&Detailed information can be obtained by executing:&    [!monospace! [+64 keeper \1--\1help]]&in a terminal.&";
	MsgOK(s.c_str(), "About keeper..");
}

void ShowBupData(const std::string &B)
{
	BupData dlg(B);
	dlg.Execute();
}
