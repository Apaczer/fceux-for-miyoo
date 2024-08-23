/*  This file is based on file_list.cpp */

#include <stdio.h>

#include "cheat_list.h"

static int listcallb(char *name, uint32 a, uint8 v, int compare, int s, int type, void *data) {
    CheatList *list;
    
    list = (CheatList *)data;
    return list->Add(name, a, v, compare, s, type, data);
}

using namespace std;

CheatList::CheatList() {
    FCEUI_ListCheats(listcallb, (void *)this);
}

int 
CheatList::Add(char *name, uint32 a, uint8 v, int c, int s, int type, void *data)
{
	CheatData cd;
	char codeStr[32];

	if (c >= 0)
	{
		sprintf(codeStr, "$%04X?%02X:%02X", a, c, v);
	}
	else
	{
		sprintf(codeStr, "$%04X:%02X   ", a, v);
	}

	cd.name = string(name);
	cd.codeStr = string(codeStr);
	cd.addr = a;
	cd.val = v;
	cd.compare = c;
	cd.status = s;
	cd.type = type;
	cheats.push_back(cd);

	return 1;
}

int 
CheatList::ToggleCheat(uint32 which) { 
    int status = FCEUI_ToggleCheat(which); 
    if (status >= 0) {
	cheats[which].status = status; 
	return (1);
    } else {
	return (0);
    }
}

int 
CheatList::DelCheat(uint32 which) {
    if (this->IsActive(which)) 
	this->ToggleCheat(which);
    int status = FCEUI_DelCheat(which); 
    if (status) 
	cheats.erase(cheats.begin() + which); 
    return (status); 
}

int 
CheatList::DelAllCheats(void) {
    FCEU_DisableAllCheats();
    FCEU_DeleteAllCheats();
    cheats.clear();
    return (1);
}

int 
CheatList::LoadCheats(const char *filename, bool importing) {
    FILE *fp = fopen (filename, "r");
    if (fp == NULL)
	return -1;

    FCEU_LoadGameCheats (fp, 1);
    fclose (fp);
    if (importing)
	savecheats = 1;
    
    FCEUI_ListCheats(listcallb, (void *)this);
    
    return 1;
}

const char*
CheatList::GetLabel(int index) {
    if (cheats[index].name.length() > 0)
	return this->GetName(index);
    else
	return this->GetCode(index);
}