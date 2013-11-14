#include "RPCServer.h"
#include "PortmapProg.h"
#include "NFSProg.h"
#include "MountProg.h"
#include "ServerSocket.h"
#include "DatagramSocket.h"
//#include <unistd.h>
#include <stdio.h>
#include <direct.h>

#define SOCKET_NUM 3
enum
{
	PORTMAP_PORT = 111,
	MOUNT_PORT = 1058,
	NFS_PORT = 2049
};
enum
{
	PROG_PORTMAP = 100000,
	PROG_NFS = 100003,
	PROG_MOUNT = 100005
};

static unsigned int g_nUID, g_nGID;
static bool g_bLogOn;
static CRPCServer g_RPCServer;
static CPortmapProg g_PortmapProg;
static CNFSProg g_NFSProg;
static CMountProg g_MountProg;

static void printUsage(char *pExe)
{
	printf("\n");
	printf("Usage: %s [-id <uid> <gid>] [-log on | off] <export path> [alias path]\n\n", pExe);
	printf("For example:\n");
	printf("On Windows> %s d:\\work\n", pExe);
	printf("On Linux> mount -t nfs 192.168.12.34:/d/work mount\n\n");
	printf("For another example:\n");
	printf("On Windows> %s d:\\work /exports\n", pExe);
	printf("On Linux> mount -t nfs 192.168.12.34:/exports\n\n");
	printf("Use \".\" to export the current directory:\n");
	printf("On Windows> %s . /exports\n", pExe);
}

static void printLine(void)
{
	printf("=====================================================\n");
}

static void printAbout(void)
{
	printLine();
	printf("WinNFSd v2.0\n");
	printf("Network File System server for Windows\n");
	printf("Copyright (C) 2005 Ming-Yang Kao\n");
	printf("Edited in 2011 by ZeWaren\n");
	printLine();
}

static void printHelp(void)
{
	printLine();
	printf("Commands:\n");
	printf("about: display messages about this program\n");
	printf("help: display help\n");
	printf("log on/off: display log messages or not\n");
	printf("list: list mounted clients\n");
	printf("quit: quit this program\n");
	printLine();
}

static void printCount(void)
{
	int nNum;

	nNum = g_MountProg.GetMountNumber();
	if (nNum == 0)
		printf("There is no client mounted.\n");
	else if (nNum == 1)
		printf("There is 1 client mounted.\n");
	else
		printf("There are %d clients mounted.\n", nNum);
}

static void printList(void)
{
	int i, nNum;

	printLine();
	nNum = g_MountProg.GetMountNumber();
	for (i = 0; i < nNum; i++)
		printf("%s\n", g_MountProg.GetClientAddr(i));
	printCount();
	printLine();
}

static void printConfirmQuit(void)
{
	printf("\n");
	printCount();
	printf("Are you sure to quit? (y/N): ");
}

static void inputCommand(void)
{
	char command[20];

	printf("Type 'help' to see help\n\n");
	while (true)
	{
		fgets(command, 20, stdin);
		if (command[strlen(command) - 1] == '\n')
			command[strlen(command) - 1] = '\0';

		if (_stricmp(command, "about") == 0)
			printAbout();
		else if (_stricmp(command, "help") == 0)
			printHelp();
		else if (_stricmp(command, "log on") == 0)
			g_RPCServer.SetLogOn(true);
		else if (_stricmp(command, "log off") == 0)
			g_RPCServer.SetLogOn(false);
		else if (_stricmp(command, "list") == 0)
			printList();
		else if (_stricmp(command, "quit") == 0)
		{
			if (g_MountProg.GetMountNumber() == 0)
				break;
			else
			{
				printConfirmQuit();
				fgets(command, 20, stdin);
				if (command[0] == 'y' || command[0] == 'Y')
					break;
			}
		}
		else if (_stricmp(command, "reset") == 0)
			g_RPCServer.Set(PROG_NFS, NULL);
		else if (strcmp(command, "") != 0)
		{
			printf("Unknown command: '%s'\n", command);
			printf("Type 'help' to see help\n");
		}
	}
}

static void start(char *path, char *pathAlias)
{
	int i;
	CDatagramSocket DatagramSockets[SOCKET_NUM];
	CServerSocket ServerSockets[SOCKET_NUM];
	bool bSuccess;
	hostent *localHost;

	g_PortmapProg.Set(PROG_MOUNT, MOUNT_PORT);  //map port for mount
	g_PortmapProg.Set(PROG_NFS, NFS_PORT);  //map port for nfs
	g_NFSProg.SetUserID(g_nUID, g_nGID);  //set uid and gid of files
	g_MountProg.Export(path, pathAlias);  //export path for mount
	g_RPCServer.Set(PROG_PORTMAP, &g_PortmapProg);  //program for portmap
	g_RPCServer.Set(PROG_NFS, &g_NFSProg);  //program for nfs
	g_RPCServer.Set(PROG_MOUNT, &g_MountProg);  //program for mount
	g_RPCServer.SetLogOn(g_bLogOn);

	for (i = 0; i < SOCKET_NUM; i++)
	{
		DatagramSockets[i].SetListener(&g_RPCServer);
		ServerSockets[i].SetListener(&g_RPCServer);
	}

	bSuccess = false;
	if (ServerSockets[0].Open(PORTMAP_PORT, 3) && DatagramSockets[0].Open(PORTMAP_PORT))  //start portmap daemon
	{
		printf("Portmap daemon started\n");
		if (ServerSockets[1].Open(NFS_PORT, 10) && DatagramSockets[1].Open(NFS_PORT))  //start nfs daemon
		{
			printf("NFS daemon started\n");
			if (ServerSockets[2].Open(MOUNT_PORT, 3) && DatagramSockets[2].Open(MOUNT_PORT))  //start mount daemon
			{
				printf("Mount daemon started\n");
				bSuccess = true;  //all daemon started
			}
			else
				printf("Mount daemon starts failed.\n");
		}
		else
			printf("NFS daemon starts failed.\n");
	}
	else
		printf("Portmap daemon starts failed.\n");

	if (bSuccess)
	{
		localHost = gethostbyname("");
		printf("Local IP = %s\n", inet_ntoa (*(struct in_addr *)*localHost->h_addr_list));  //local address
		inputCommand();  //wait for commands
	}

	for (i = 0; i < SOCKET_NUM; i++)
	{
		DatagramSockets[i].Close();
		ServerSockets[i].Close();
	}
}

int main(int argc, char *argv[])
{
	char *pPath = NULL;
	char m_pPathAlias[MAXPATHLEN];
	char *pPathAlias = NULL;
	WSADATA wsaData;
	
	printAbout();
	if (argc < 2)
	{
		pPath = strrchr(argv[0], '\\');
		pPath = pPath == NULL? argv[0] : pPath + 1;
		printUsage(pPath);
		return 1;
	}

	g_nUID = g_nGID = 0;
	g_bLogOn = true;
	for (int i = 1; i < argc; i++)  //parse parameters
	{
		if (_stricmp(argv[i], "-id") == 0)
		{
			g_nUID = atoi(argv[++i]);
			g_nGID = atoi(argv[++i]);
		}
		else if (_stricmp(argv[i], "-log") == 0)
		{
			g_bLogOn = _stricmp(argv[++i], "off") != 0;
		}
		else if (i==argc - 2) {
			pPath = argv[argc - 2];  //path is before the last parameter
			if (*pPath == '"')
				++pPath;  //remove head "
			if (*(pPath + strlen(pPath) - 1) == '"')
				*(pPath + strlen(pPath) - 1) = '\0';  //remove tail "
			if (pPath[0] == '.' && pPath[1] == '\0') {
				static char path1[MAXPATHLEN];
				_getcwd(path1, MAXPATHLEN);
				pPath = path1;
			}
			else if (pPath[1] != ':' || !((pPath[0] >= 'A' && pPath[0] <= 'Z') || (pPath[0] >= 'a' && pPath[0] <= 'z')))  //check path format
			{
				printf("Path format is incorrect.\n");
				printf("Please use a full path such as C:\\work");
				return 1;
			}

			pPathAlias = argv[argc - 1]; //path alias is the last parameter
			if (*pPathAlias == '"')
				++pPathAlias;  //remove head "
			if (*(pPathAlias + strlen(pPathAlias) - 1) == '"')
				*(pPathAlias + strlen(pPathAlias) - 1) = '\0';  //remove tail "
			if (pPathAlias[0] != '/')  //check path alias format
			{
				printf("Path alias format is incorrect.\n");
				printf("Please use a path like /exports\n");
				return 1;
			}
			break;
		}
		else if (i==argc - 1) {
			pPath = argv[argc - 1];  //path is the last parameter
			if (*pPath == '"')
				++pPath;  //remove head "
			if (*(pPath + strlen(pPath) - 1) == '"')
				*(pPath + strlen(pPath) - 1) = '\0';  //remove tail "
			if (pPath[0] == '.' && pPath[1] == '\0') {
				static char path1[MAXPATHLEN];
				_getcwd(path1, MAXPATHLEN);
				pPath = path1;
			}
			else if (pPath[1] != ':' || !((pPath[0] >= 'A' && pPath[0] <= 'Z') || (pPath[0] >= 'a' && pPath[0] <= 'z')))  //check path format
			{
				printf("Path format is incorrect.\n");
				printf("Please use a full path such as C:\\work");
				return 1;
			}
			strncpy_s(m_pPathAlias, pPath, sizeof(m_pPathAlias) - 1);
			m_pPathAlias[1] = m_pPathAlias[0];  //transform mount path to Windows format
			m_pPathAlias[0] = '/';
			for (size_t i = 2; i < strlen(pPath); i++)
				if (m_pPathAlias[i] == '\\')
					m_pPathAlias[i] = '/';
			m_pPathAlias[strlen(pPath)] = '\0';
			pPathAlias = m_pPathAlias;
			break;
		}
	}

	WSAStartup(0x0101, &wsaData);

	if (pPath != NULL && pPathAlias != NULL) {
		printf("Starting, path is: %s, path alias is: %s\n", pPath, pPathAlias);
		start(pPath, pPathAlias);
	}

	WSACleanup();
	return 0;
}
