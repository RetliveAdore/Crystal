#include "Demo.h"

char command[128];
CRUINT8 sub = 0;

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
	if (
		Compare(command, "quit") ||
		Compare(command, "exit") ||
		Compare(command, "close")
		)
		return CRFALSE;
	else if (Compare(command, "Demo1"))
	{
		printf("return value: %d\n", Demo1(argc, argv));
	}
	else
		printf("invalid command: %s\n", command);
	return CRTRUE;
}
