#include "RPCProg.h"
#include <stdarg.h>
#include <stdio.h>

CRPCProg::CRPCProg()
{
    m_bLogOn = true;
}

CRPCProg::~CRPCProg()
{
}

void CRPCProg::SetLogOn(bool bLogOn)
{
    m_bLogOn = bLogOn;
}

int CRPCProg::PrintLog(const char *format, ...)
{
    va_list vargs;
    int nResult;

    nResult = 0;

    if (m_bLogOn) {
        va_start(vargs, format);
        nResult = vprintf(format, vargs);
        va_end(vargs);
    }

    return nResult;
}
