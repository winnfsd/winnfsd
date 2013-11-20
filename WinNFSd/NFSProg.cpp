#include "NFSProg.h"

CNFSProg::CNFSProg() : CRPCProg()
{
    m_nUID = m_nGID = 0;
    m_pNFS2Prog = NULL;
    m_pNFS3Prog = NULL;
}

CNFSProg::~CNFSProg()
{
    delete m_pNFS2Prog;
    delete m_pNFS3Prog;
}

void CNFSProg::SetUserID(unsigned int nUID, unsigned int nGID)
{
    m_nUID = nUID;
    m_nGID = nGID;
}

int CNFSProg::Process(IInputStream *pInStream, IOutputStream *pOutStream, ProcessParam *pParam)
{
    if (pParam->nVersion == 2) {
        if (m_pNFS2Prog == NULL) {
            m_pNFS2Prog = new CNFS2Prog();
            m_pNFS2Prog->SetUserID(m_nUID, m_nGID);
            m_pNFS2Prog->SetLogOn(m_bLogOn);
        }

        return m_pNFS2Prog->Process(pInStream, pOutStream, pParam);
    } else {
        if (m_pNFS3Prog == NULL) {
            m_pNFS3Prog = new CNFS3Prog();
            m_pNFS3Prog->SetUserID(m_nUID, m_nGID);
            m_pNFS3Prog->SetLogOn(m_bLogOn);
        }

        return m_pNFS3Prog->Process(pInStream, pOutStream, pParam);
    }
}

void CNFSProg::SetLogOn(bool bLogOn)
{
    CRPCProg::SetLogOn(bLogOn);

    if (m_pNFS2Prog != NULL) {
        m_pNFS2Prog->SetLogOn(bLogOn);
    }

    if (m_pNFS3Prog != NULL) {
        m_pNFS3Prog->SetLogOn(bLogOn);
    }

}
