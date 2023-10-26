#ifndef _INCLUDE_DENO_H_
#define _INCLUDE_DENO_H_

#include <Crystal.h>
#include <stdio.h>
#include <string.h>

//base

CRBOOL Compare(const char* str1, const char* str2);
void ClearCommand();
void GetCommand();
CRBOOL ProcessCommand(int argc, char** argv);

//demos

int Demo1(int argc, char** argv);
int Demo2(int argc, char** argv);

#endif //include
