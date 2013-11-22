#ifndef _OUTPUTSTREAM_H_
#define _OUTPUTSTREAM_H_

#include <stdio.h>

class IOutputStream
{
    public:
    virtual void Write(void *pData, unsigned int nSize) = 0;
    virtual void Write(unsigned long nValue) = 0;
    virtual void Write8(unsigned __int64 nValue) = 0;
    virtual void Seek(int nOffset, int nFrom) = 0;
    virtual int GetPosition(void) = 0;
};

#endif
