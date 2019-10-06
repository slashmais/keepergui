
#include "keepergui.h"
#include <utilfuncs/utilfuncs.h>
#include <compression/compression.h>
#include <algorithm>


BupData::BupData(const std::string &B)
{
	bdir=B;
	Title(spf("Backups: ", bdir).c_str());
	SetRect(0,0,900,700);
	Sizeable();
	
	HSplit.Horz(tree, doc);
	HSplit.SetPos(2500);
	VSplit.Vert(HSplit, pan);
	VSplit.SetPos(9250);
	Add(VSplit);

	pan.btnClose.WhenPush << [&]{ Close(); };
	pan.btnCopyFile.WhenPush << [&]{ panCopyFile(); };
	pan.btnCopyFile.Enable(false);
	doc.SetEditable(false);//.SetReadOnly();
	
	DirTree dt;
	dt.Read(bdir);
	tree.NoRoot();
	root=tree.Add(0, Image(), bdir.c_str(), true);
	FillTree(root, dt);
	tree.Open(root);
	
	tree.WhenSel << [&]{ ShowFile(); };
}

void BupData::FillTree(int P, const DirTree &dt)
{
	for (auto f:dt.content) tree.Add(P, Image(), f.first.c_str(), false);
	for (auto p:dt)
	{
		int N=tree.Add(P, Image(), p.first.c_str(), true);
		FillTree(N, p.second);
	}
}

void BupData::ShowFile()
{
	int n=tree.GetCursor();
	if (n>=0)
	{
		auto isbindata=[](const std::string &d) { for (size_t i=0; i<std::min(d.size(),size_t(2000)); i++) { if (!d[i]) return true; } return false; };
		std::string s{};
		int p;
		s=tree.Get(n).ToString().ToStd();
		p=tree.GetParent(n);
		while ((p>=0)&&(p!=root)) { s=path_append(tree.Get(p).ToString().ToStd(), s); p=tree.GetParent(p); }
		s=path_append(bdir, s);
		if (file_exist(s)) //if file & !dir
		{
			std::string t{};
			if (iszcompressed(s)) { decompress_file_string(s, t); }
			else { file_read(s, t); if (isbindata(t)) t="(not displayable data)"; } ///.........todo...use hex-out-module from fed
			doc.SetData(t.c_str());
			pan.btnCopyFile.Enable(true);
		}
		else { doc.Clear(); pan.btnCopyFile.Enable(false); }
	}
}


void BupData::panCopyFile()
{
	int n=tree.GetCursor();
	if (n>=0)
	{
		std::string s{};
		size_t p;
		s=tree.Get(n).ToString().ToStd();
		p=tree.GetParent(n);
		while ((p>=0)&&(p!=(size_t)root)) { s=path_append(tree.Get(p).ToString().ToStd(), s); p=tree.GetParent(p); }
		s=path_append(bdir, s);
		if (file_exist(s))
		{
			String T=SelectDirectory();
			std::string t{};
			if (!T.IsEmpty())
			{
				if (iszcompressed(s))
				{
					if ((p=s.find_last_of('/'))!=std::string::npos)
					{
						t=s.substr(p+1, s.size()-p-3);
						if ((p=t.find('_'))!=std::string::npos) t=t.substr(p+1); //~~should~~ be fine ...
						t=path_append(T.ToStd(), t);
						decompress_file(s, t);
					}
				}
				else file_copy(s, T.ToStd());
			}
		}
	}
}
