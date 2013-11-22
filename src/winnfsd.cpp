#include "RPCServer.h"
#include "PortmapProg.h"
#include "NFSProg.h"
#include "MountProg.h"
#include "ServerSocket.h"
#include "DatagramSocket.h"
//#include <unistd.h>
#include <stdio.h>
#include <direct.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

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
enum pathFormats
{
    FORMAT_PATH = 1,
    FORMAT_PATHALIAS = 2
};

static unsigned int g_nUID, g_nGID;
static bool g_bLogOn;
static char *g_sFileName;
static CRPCServer g_RPCServer;
static CPortmapProg g_PortmapProg;
static CNFSProg g_NFSProg;
static CMountProg g_MountProg;

static void printUsage(char *pExe)
{
    printf("\n");
    printf("Usage: %s [-id <uid> <gid>] [-log on | off] [-pathFile <file>] [export path] [alias path]\n\n", pExe);
    printf("At least a file or a path is needed\n");
    printf("For example:\n");
    printf("On Windows> %s d:\\work\n", pExe);
    printf("On Linux> mount -t nfs 192.168.12.34:/d/work mount\n\n");
    printf("For another example:\n");
    printf("On Windows> %s d:\\work /exports\n", pExe);
    printf("On Linux> mount -t nfs 192.168.12.34:/exports\n\n");
    printf("Use \".\" to export the current directory (works also for -filePath):\n");
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
    printf("Edited in 2013 by Alexander Schneider (Jankowfsky AG)\n");
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

    if (nNum == 0) {
        printf("There is no client mounted.\n");
    } else if (nNum == 1) {
        printf("There is 1 client mounted.\n");
    } else {
        printf("There are %d clients mounted.\n", nNum);
    }
}

static void printList(void)
{
    int i, nNum;

    printLine();
    nNum = g_MountProg.GetMountNumber();

    for (i = 0; i < nNum; i++) {
        printf("%s\n", g_MountProg.GetClientAddr(i));
    }

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

    while (true) {
        fgets(command, 20, stdin);

        if (command[strlen(command) - 1] == '\n') {
            command[strlen(command) - 1] = '\0';
        }

        if (_stricmp(command, "about") == 0) {
            printAbout();
        } else if (_stricmp(command, "help") == 0) {
            printHelp();
        } else if (_stricmp(command, "log on") == 0) {
            g_RPCServer.SetLogOn(true);
        } else if (_stricmp(command, "log off") == 0) {
            g_RPCServer.SetLogOn(false);
        } else if (_stricmp(command, "list") == 0) {
            printList();
        } else if (_stricmp(command, "quit") == 0) {
            if (g_MountProg.GetMountNumber() == 0) {
                break;
            } else {
                printConfirmQuit();
                fgets(command, 20, stdin);

                if (command[0] == 'y' || command[0] == 'Y') {
                    break;
                }
            }
        } else if (_stricmp(command, "reset") == 0) {
            g_RPCServer.Set(PROG_NFS, NULL);
        } else if (strcmp(command, "") != 0) {
            printf("Unknown command: '%s'\n", command);
            printf("Type 'help' to see help\n");
        }
    }
}

static void start(std::vector<std::vector<std::string>> paths)
{
    int i;
    CDatagramSocket DatagramSockets[SOCKET_NUM];
    CServerSocket ServerSockets[SOCKET_NUM];
    bool bSuccess;
    hostent *localHost;

    g_PortmapProg.Set(PROG_MOUNT, MOUNT_PORT);  //map port for mount
    g_PortmapProg.Set(PROG_NFS, NFS_PORT);  //map port for nfs
    g_NFSProg.SetUserID(g_nUID, g_nGID);  //set uid and gid of files

    int numberOfElements = paths.size();
    printf("Mounting %i paths\n", numberOfElements);

    for (i = 0; i < numberOfElements; i++) {
        char *pPath = (char*)paths[i][0].c_str();
        char *pPathAlias = (char*)paths[i][1].c_str();

        printf("Path #%i is: %s, path alias is: %s\n", i + 1, pPath, pPathAlias);
        g_MountProg.Export(pPath, pPathAlias);  //export path for mount
    }
    
    g_RPCServer.Set(PROG_PORTMAP, &g_PortmapProg);  //program for portmap
    g_RPCServer.Set(PROG_NFS, &g_NFSProg);  //program for nfs
    g_RPCServer.Set(PROG_MOUNT, &g_MountProg);  //program for mount
    g_RPCServer.SetLogOn(g_bLogOn);

    for (i = 0; i < SOCKET_NUM; i++) {
        DatagramSockets[i].SetListener(&g_RPCServer);
        ServerSockets[i].SetListener(&g_RPCServer);
    }

    bSuccess = false;

    if (ServerSockets[0].Open(PORTMAP_PORT, 3) && DatagramSockets[0].Open(PORTMAP_PORT)) { //start portmap daemon
        printf("Portmap daemon started\n");

        if (ServerSockets[1].Open(NFS_PORT, 10) && DatagramSockets[1].Open(NFS_PORT)) { //start nfs daemon
            printf("NFS daemon started\n");

            if (ServerSockets[2].Open(MOUNT_PORT, 3) && DatagramSockets[2].Open(MOUNT_PORT)) { //start mount daemon
                printf("Mount daemon started\n");
                bSuccess = true;  //all daemon started
            } else {
                printf("Mount daemon starts failed.\n");
            }
        } else {
            printf("NFS daemon starts failed.\n");
        }
    } else {
        printf("Portmap daemon starts failed.\n");
    }


    if (bSuccess) {
        localHost = gethostbyname("");
        printf("Local IP = %s\n", inet_ntoa(*(struct in_addr *)*localHost->h_addr_list));  //local address
        inputCommand();  //wait for commands
    }

    for (i = 0; i < SOCKET_NUM; i++) {
        DatagramSockets[i].Close();
        ServerSockets[i].Close();
    }
}

char *formatPath(char *pPath, pathFormats format)
{
    //Remove head spaces
    while (*pPath == ' ') { 
        ++pPath;
    }

    //Remove tail spaces
    while (*(pPath + strlen(pPath) - 1) == ' ') {
        *(pPath + strlen(pPath) - 1) = '\0';
    }

    //Remove head "
    if (*pPath == '"') {
        ++pPath;  
    }

    //Remove tail "
    if (*(pPath + strlen(pPath) - 1) == '"') {
        *(pPath + strlen(pPath) - 1) = '\0';  
    }

    //Check for right path format
    if (format == FORMAT_PATH) {
        if (pPath[0] == '.') {
            static char path1[MAXPATHLEN];
            _getcwd(path1, MAXPATHLEN);

            if (pPath[1] == '\0') {
                pPath = path1;
            } else if (pPath[1] == '\\') {
                strcat_s(path1, pPath + 1);
                pPath = path1;
            }
            
        } else if (pPath[1] != ':' || !((pPath[0] >= 'A' && pPath[0] <= 'Z') || (pPath[0] >= 'a' && pPath[0] <= 'z'))) { //check path format
            printf("Path format is incorrect.\n");
            printf("Please use a full path such as C:\\work\n");

            return NULL;
        }

        for (size_t i = 0; i < strlen(pPath); i++) {
            if (pPath[i] == '/') {
                pPath[i] = '\\';
            }
        }
    } else if (format == FORMAT_PATHALIAS) {
        if (pPath[0] != '/') { //check path alias format
            printf("Path alias format is incorrect.\n");
            printf("Please use a path like /exports\n");

            return NULL;
        }
    }

    return pPath;
}

char *formatPathAlias(char *pPathAlias)
{
    pPathAlias[1] = pPathAlias[0]; //transform mount path to Windows format
    pPathAlias[0] = '/';

    for (size_t i = 2; i < strlen(pPathAlias); i++) {
        if (pPathAlias[i] == '\\') {
            pPathAlias[i] = '/';
        }
    }

    pPathAlias[strlen(pPathAlias)] = '\0';

    return pPathAlias;
}

int main(int argc, char *argv[])
{
    std::vector<std::vector<std::string>> pPaths;
    char *pPath = NULL;
    int numberOfPaths = 1;

    WSADATA wsaData;

    printAbout();

    if (argc < 2) {
        pPath = strrchr(argv[0], '\\');
        pPath = pPath == NULL ? argv[0] : pPath + 1;
        printUsage(pPath);
        return 1;
    }
  
    g_nUID = g_nGID = 0;
    g_bLogOn = true;
    g_sFileName = NULL;

    for (int i = 1; i < argc; i++) {//parse parameters
        if (_stricmp(argv[i], "-id") == 0) {
            g_nUID = atoi(argv[++i]);
            g_nGID = atoi(argv[++i]);
        } else if (_stricmp(argv[i], "-log") == 0) {
            g_bLogOn = _stricmp(argv[++i], "off") != 0;           
        } else if (_stricmp(argv[i], "-pathFile") == 0) {
            g_sFileName = argv[++i];
            int numberOfPathsFromFile = 0;

            g_sFileName = formatPath(g_sFileName, FORMAT_PATH);

            std::ifstream pathFile(g_sFileName);

            if (pathFile.is_open()) {
                std::string line;

                while (std::getline(pathFile, line)) {
                    char *pCurPath = (char*)malloc(line.size() + 1);
                    pCurPath = (char*)line.c_str();
                    pCurPath = formatPath(pCurPath, FORMAT_PATH);

                    if (pCurPath != NULL) {
                        char curPathAlias[MAXPATHLEN];
                        strcpy_s(curPathAlias, pCurPath);
                        char *pCurPathAlias = (char*)malloc(strlen(curPathAlias));
                        pCurPathAlias = curPathAlias;

                        pCurPathAlias = formatPathAlias(pCurPathAlias);

                        std::vector<std::string> pCurPaths;
                        pCurPaths.push_back(std::string(pCurPath));
                        pCurPaths.push_back(std::string(pCurPathAlias));
                        pPaths.push_back(pCurPaths);
                    }
                }
            } else {
                printf("Can't open file %s.\n", g_sFileName);
                return 1;
            }
        } else if (i == argc - 2) {
            pPath = argv[argc - 2];  //path is before the last parameter
            pPath = formatPath(pPath, FORMAT_PATH);

            char *pCurPathAlias = argv[argc - 1]; //path alias is the last parameter
            pCurPathAlias = formatPath(pCurPathAlias, FORMAT_PATHALIAS);

            if (pPath != NULL || pCurPathAlias != NULL) {
                std::vector<std::string> pCurPaths;
                pCurPaths.push_back(std::string(pPath));
                pCurPaths.push_back(std::string(pCurPathAlias));
                pPaths.push_back(pCurPaths);
            }

            break;
        } else if (i == argc - 1) {
            char *pPath = argv[argc - 1];  //path is the last parameter
            pPath = formatPath(pPath, FORMAT_PATH);

            if (pPath != NULL) {
                char curPathAlias[MAXPATHLEN];
                strcpy_s(curPathAlias, pPath);
                char *pCurPathAlias = curPathAlias;

                pCurPathAlias = formatPathAlias(pCurPathAlias);

                std::vector<std::string> pCurPaths;
                pCurPaths.push_back(std::string(pPath));
                pCurPaths.push_back(std::string(pCurPathAlias));
                pPaths.push_back(pCurPaths);
            }

            break;
        }
    }

    HWND console = FindWindow("ConsoleWindowClass", NULL);

    if (g_bLogOn == false && IsWindow(console)) {
        ShowWindow(console, SW_HIDE); // hides the window
    }

    if (pPaths.size() <= 0) {
        printf("No paths to mount\n");
        return 1;
    }

    WSAStartup(0x0101, &wsaData);
    start(pPaths);
    WSACleanup();

    return 0;
}
