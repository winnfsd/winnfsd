#ifndef _FILETABLE_H_
#define _FILETABLE_H_

#define TABLE_SIZE 1024

typedef struct
{
    char *path;
    unsigned int nPathLen;
    unsigned char *handle;
    bool bCached;
} FILE_ITEM;

typedef struct _FILE_TABLE
{
    FILE_ITEM pItems[TABLE_SIZE];
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
    unsigned long GetIDByPath(char *path);
    unsigned char *GetHandleByPath(char *path);
    char *GetPathByHandle(unsigned char *handle);
    FILE_ITEM *FindItemByPath(char *path);

    protected:
    FILE_ITEM *AddItem(char *path);

    private:
    FILE_TABLE *m_pFirstTable, *m_pLastTable;
    unsigned int m_nTableSize;
    CACHE_LIST *m_pCacheList;

    FILE_ITEM *GetItemByID(unsigned int nID);
    void PutItemInCache(FILE_ITEM *pItem);
};

extern bool FileExists(char *path);
extern unsigned long GetFileID(char *path);
extern unsigned char *GetFileHandle(char *path);
extern char *GetFilePath(unsigned char *handle);
extern int RenameFile(char *pathFrom, char *pathTo);
extern bool RemoveFile(char *path);

#endif
