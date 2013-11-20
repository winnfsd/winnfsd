#ifndef _INPUTSTREAM_H_
#define _INPUTSTREAM_H_

#include <stdio.h>

class IInputStream
{
    public:
    virtual unsigned int Read(void *pData, unsigned int nSize) = 0;
    virtual unsigned int Read(unsigned long *pnValue) = 0;
    virtual unsigned int Read8(unsigned __int64 *pnValue) = 0;
    virtual unsigned int Skip(unsigned int nSize) = 0;
    virtual unsigned int GetSize(void) = 0;
};

#endif
