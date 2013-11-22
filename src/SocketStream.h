#ifndef _SOCKETSTREAM_H_
#define _SOCKETSTREAM_H_

#include "InputStream.h"
#include "OutputStream.h"

class CSocketStream : public IInputStream, public IOutputStream
{
    public:
    CSocketStream();
    virtual ~CSocketStream();
    unsigned char *GetInput(void);
    void SetInputSize(unsigned int nSize);
    unsigned char *GetOutput(void);
    unsigned int GetOutputSize(void);
    unsigned int GetBufferSize(void);
    unsigned int Read(void *pData, unsigned int nSize);
    unsigned int Read(unsigned long *pnValue);
    unsigned int Read8(unsigned __int64 *pnValue);
    unsigned int Skip(unsigned int nSize);
    unsigned int GetSize(void);
    void Write(void *pData, unsigned int nSize);
    void Write(unsigned long nValue);
    void Write8(unsigned __int64 nValue);
    void Seek(int nOffset, int nFrom);
    int GetPosition(void);
    void Reset(void);

    private:
    unsigned char *m_pInBuffer, *m_pOutBuffer;
    unsigned int m_nInBufferIndex, m_nInBufferSize, m_nOutBufferIndex, m_nOutBufferSize;
};

#endif
