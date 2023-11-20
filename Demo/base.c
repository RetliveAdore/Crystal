﻿#include "Demo.h"

static char command[128];
static CRUINT8 sub = 0;

CRBOOL Compare(const char* str1, const char* str2)
{
	if (strlen(str1) != strlen(str2))
		return CRFALSE;
	for (int i = 0; i < strlen(str1); i++)
		if (str1[i] != str2[i]) return CRFALSE;
	return CRTRUE;
}

void ClearCommand()
{
	for (int i = 0; i < 128; i++) command[i] = '\0';
}

void GetCommand()
{
	printf("Demo$:");
	while (sub < 127)
	{
		command[sub] = getchar();
		if (command[sub] == '\n')
		{
			command[sub] = '\0';
			sub = 0;
			return;
		}
		sub++;
	}
	sub = 0;
}

CRBOOL ProcessCommand(int argc, char** argv)
{
	if (!strlen(command))
		return CRTRUE;
	if (
		Compare(command, "quit") ||
		Compare(command, "exit") ||
		Compare(command, "close")
		)
		return CRFALSE;
	else if (Compare(command, "Demo1"))
		printf("返回值：%d\n\n", Demo1(argc, argv));
	else if (Compare(command, "Demo2"))
		printf("返回值：%d\n\n", Demo2(argc, argv));
	else if (Compare(command, "Demo3"))
		printf("返回值：%d\n\n", Demo3(argc, argv));
	else if (Compare(command, "Demo4"))
		printf("返回值：%d\n\n", Demo4(argc, argv));
	else
		printf("invalid command: %s\n", command);
	return CRTRUE;
}
