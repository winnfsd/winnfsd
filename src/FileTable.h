#ifndef _FILETABLE_H_
#define _FILETABLE_H_

#define TABLE_SIZE 1024

#include "tree.hh"

typedef struct
{
    char *path;
    unsigned int nPathLen;
    unsigned char *handle;
    bool bCached;
} FILE_ITEM;

typedef struct _FILE_TABLE
{
	 tree_node_<FILE_ITEM>* pItems[TABLE_SIZE];
    _FILE_TABLE *pNext;
} FILE_TABLE;

typedef struct _CACHE_LIST
{
    FILE_ITEM *pItem;
    _CACHE_LIST *pNext;
} CACHE_LIST;

class CFileTable
{
    public:
    CFileTable();
    ~CFileTable();
    unsigned long GetIDByPath(const char *path);
    unsigned char *GetHandleByPath(const char *path);
    bool GetPathByHandle(unsigned char *handle, std::string &path);
	tree_node_<FILE_ITEM>* FindItemByPath(const char *path);
    bool RemoveItem(const char *path);
	void RenameFile(const char *pathFrom, const char *pathTo);

    protected:
		tree_node_<FILE_ITEM>* AddItem(const char *path);

    private:
    FILE_TABLE *m_pFirstTable, *m_pLastTable;
    unsigned int m_nTableSize;
    CACHE_LIST *m_pCacheList;

	tree_node_<FILE_ITEM>* GetItemByID(unsigned int nID);
    void PutItemInCache(FILE_ITEM *pItem);

};

extern bool FileExists(const char *path);
extern unsigned long GetFileID(const char *path);
extern unsigned char *GetFileHandle(const char *path);
extern bool GetFilePath(unsigned char *handle, std::string &filePath);
extern int RenameFile(const char *pathFrom, const char *pathTo);
extern int RenameDirectory(const char *pathFrom, const char *pathTo);
extern int RemoveFolder(const char *path);
extern bool RemoveFile(const char *path);
#endif
