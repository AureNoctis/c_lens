#ifndef _UTILS_
#define _UTILS_

#include <stdio.h>
#include <Windows.h>

#define _do_
#define _then_

void summonConsole() {
	if (AllocConsole()) {
		FILE* fDummy;
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONIN$", "r", stdin);

		// OUTPUT MODE (colors etc.)
		HANDLE hOut	   = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD  outMode = 0;
		GetConsoleMode(hOut, &outMode);
		outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOut, outMode);

		// INPUT MODE (FIX FREEZE)
		HANDLE hIn	  = GetStdHandle(STD_INPUT_HANDLE);
		DWORD  inMode = 0;
		GetConsoleMode(hIn, &inMode);

		inMode |= ENABLE_EXTENDED_FLAGS;   // required
		inMode &= ~ENABLE_QUICK_EDIT_MODE; // disable freeze
		inMode &= ~ENABLE_INSERT_MODE;	   // optional

		SetConsoleMode(hIn, inMode);
	}
}

#endif
