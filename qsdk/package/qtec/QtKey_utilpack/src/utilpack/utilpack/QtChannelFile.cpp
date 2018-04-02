
#include "QtBase.h"
#include "QtChannelFile.h"
#include "QtHttpUrl.h"
#include "QtMessageBlock.h"

CQtChannelFile::CQtChannelFile()
	: m_pfGet(NULL)
	, m_pSink(NULL)
	, m_bSync(FALSE)
{
}

CQtChannelFile::~CQtChannelFile()
{
	Disconnect(QT_OK);
}

DWORD CQtChannelFile::AddReference()
{
	return CQtReferenceControlSingleThreadTimerDelete::AddReference();
}

DWORD CQtChannelFile::ReleaseReference()
{
	return CQtReferenceControlSingleThreadTimerDelete::ReleaseReference();
}

BOOL CQtChannelFile::Init(CQtHttpUrl *aUrl)
{
	QT_ASSERTE_RETURN(aUrl, FALSE);
	CQtString strScheme = aUrl->GetScheme();
	QT_ASSERTE_RETURN(strScheme == CQtHttpUrl::s_pszSchemeFile, FALSE);

	CQtString strPath = aUrl->GetPath();
	return Init(strPath);
}

BOOL CQtChannelFile::Init(const CQtString &aPath)
{
	BOOL bRet = FALSE;
	QT_ASSERTE_RETURN(!m_pfGet, bRet);

	if (aPath.empty())
		return bRet;
	m_strFileName = aPath;

	m_pfGet = fopen(m_strFileName.c_str(), "rb");
	if (!m_pfGet) {
		QT_ERROR_TRACE_THIS("CQtChannelFile::Init,"
			" m_strFileName=" << m_strFileName);
		return bRet;
	}
	setbuf(m_pfGet, NULL);
	return TRUE;
}

QtResult CQtChannelFile::AsyncOpen(IQtChannelSink *aSink)
{
	QT_ASSERTE_RETURN(m_pfGet, QT_ERROR_NOT_INITIALIZED);
	QT_ASSERTE_RETURN(aSink, QT_ERROR_INVALID_ARG);
	QT_ASSERTE(!m_pSink);
	m_pSink = aSink;

	if (m_bSync) {
		OnTimer(&m_TimerReadFile);
		return QT_OK;
	}
	else
		return m_TimerReadFile.Schedule(this, CQtTimeValue::s_tvZero, 1); 
}

void CQtChannelFile::OnTimer(CQtTimerWrapperID* aId)
{
	QT_ASSERTE(m_pSink);

	long lLen = 0;
	if (fseek(m_pfGet, 0, SEEK_END) || 
		(lLen = ftell(m_pfGet)) == -1 || 
		lLen > 1024 * 1024) 
	{
		// the file size must be less than 1M
		QT_ERROR_TRACE_THIS("CQtChannelFile::OnTimer,"
			" the file size must be less than 1M. lLen=" << lLen);
		m_pSink->OnConnect(QT_ERROR_NOT_AVAILABLE, this);
		return;
	}
	else {
		m_pSink->OnConnect(QT_OK, this);
		if (!m_pSink)
			return;
	}

	// TODO: use MessageBlock instead string
	CQtString strStreamBuf;
	strStreamBuf.resize(0);
	strStreamBuf.reserve(lLen);
	rewind(m_pfGet);

	char szBuf[1024 * 8];
	for ( ; ; ) {
		size_t dwRead = fread(szBuf, 1, sizeof(szBuf), m_pfGet);
		strStreamBuf.append(szBuf, dwRead);
		if (dwRead < sizeof(szBuf)) 
			break;
	}

	if (ferror(m_pfGet)) {
		m_pSink->OnDisconnect(QT_ERROR_FAILURE, this);
	}
	else {
		CQtMessageBlock mbOn(
			strStreamBuf.length(), 
			const_cast<char*>(strStreamBuf.c_str()),
			CQtMessageBlock::DONT_DELETE,
			strStreamBuf.length());
		m_pSink->OnReceive(mbOn, this);

		// may be Disconnect() in OnReceive().
		if (m_pSink)
			m_pSink->OnDisconnect(QT_OK, this);
	}
}

QtResult CQtChannelFile::OpenWithSink(IQtTransportSink *aSink)
{
	QT_ASSERTE(!"CQtChannelFile::OpenWithSink");
	return QT_ERROR_NOT_IMPLEMENTED;
}

IQtTransportSink* CQtChannelFile::GetSink()
{
	return m_pSink;
}

QtResult CQtChannelFile::GetUrl(CQtHttpUrl *&aUrl)
{
	QT_ASSERTE(!"CQtChannelFile::GetUrl");
	return QT_ERROR_NOT_IMPLEMENTED;
}

QtResult CQtChannelFile::
SendData(CQtMessageBlock &aData, CQtTransportParameter *aPara)
{
	QT_ASSERTE(!"CQtChannelFile::SendData");
	return QT_ERROR_NOT_IMPLEMENTED;
}

QtResult CQtChannelFile::SetOption(DWORD aCommand, LPVOID aArg)
{
	QT_ASSERTE_RETURN(aArg, QT_ERROR_INVALID_ARG);

	switch (aCommand) {
	case QT_OPT_CHANNEL_FILE_SYNC:
		m_bSync = *static_cast<BOOL*>(aArg);
		return QT_OK;
	
	default:
		QT_WARNING_TRACE_THIS("CQtChannelFile::SetOption,"
			" unknow aCommand=" << aCommand << 
			" aArg=" << aArg);
		return QT_ERROR_INVALID_ARG;
	}
}

QtResult CQtChannelFile::GetOption(DWORD aCommand, LPVOID aArg)
{
	QT_ASSERTE(!"CQtChannelFile::GetOption");
	return QT_ERROR_NOT_IMPLEMENTED;
}

QtResult CQtChannelFile::Disconnect(QtResult aReason)
{
	if (m_pfGet) {
		fclose(m_pfGet);
		m_pfGet = NULL;
	}
	m_pSink = NULL;
	m_TimerReadFile.Cancel();
	return QT_OK;
}

