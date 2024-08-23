/*  This file is based on file_list.h */

#ifndef CHEAT_LIST_H
#define CHEAT_LIST_H

#include <vector>
#include <string>

#include "../../../types.h"
#include "../../../cheat.h"
#include "../../../driver.h"

struct CheatData {
	std::string name;
	std::string codeStr;
	uint16 addr;
	uint8 val;
	int compare;	/* -1 for no compare. */
	int type;	/* 0 for replace, 1 for substitute(GG). */
	int status;
};

class CheatList
{
    public:
        CheatList();
        ~CheatList() {}

	const char *GetLabel(int index);
        const char *GetName(int index)  { return cheats[index].name.c_str(); }
	const char *GetCode(int index)  { return cheats[index].codeStr.c_str(); }
	int LoadCheats(const char *filename, bool importing=false);
        int Size() { return cheats.size(); }
	int Add(char *name, uint32 a, uint8 v, int c, int s, int type, void *data);
	bool IsActive(int index) { return cheats[index].status ? true : false; }
	int ToggleCheat(uint32 which);
	int DelCheat(uint32 which);
	int DelAllCheats(void);

    protected:
	std::vector<CheatData> cheats;
        const char **file_types;
};

#endif
