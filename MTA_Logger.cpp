#include "MTA_Logger.h"

static const std::string LOG_TAG_CRIT = "[CRT] ";
static const std::string LOG_TAG_ERROR = "[ERR] ";
static const std::string LOG_TAG_WARN = "[WAN] ";
static const std::string LOG_TAG_INFO = "[INF] ";
static const std::string LOG_TAG_VBS = "[VBS] ";
static const std::string LOG_TAG_DBG = "[DBG] ";
static const std::string LOG_TAG_PERF = "[PRF] ";
static const std::string LOG_TAG_TRACE = "[TRC] ";
static const std::string LOG_TAG_Unkown = "[Unknown] ";

#define LOG_PCS_MAX_SIZE	1024

#define		HANDLE_VAR_ARG		\
	char msg[LOG_PCS_MAX_SIZE] = {0};	\
	va_list lst;				\
	va_start(lst, fmt);			\
	vsnprintf(msg, LOG_PCS_MAX_SIZE - 1, fmt, lst); \
	va_end(lst)

std::string MTA_Logger::threadIdStamp()
{
	char bf[16] = {0};
#ifdef WIN32
	DWORD tid = GetCurrentThreadId();
	snprintf(bf, 15, "[Tx0X%X]", tid);
	return bf;
#else
	return "";
#endif // WIN32
}

std::string MTA_Logger::timeStamp()
{
	char bf[32] = {0};
#ifdef WIN32
	SYSTEMTIME st;
	GetLocalTime(&st);
	snprintf(bf, 31,
		"[%04d-%02d-%02d %02d:%02d:%02d.%03d]",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#endif // WIN32
	return bf;
}

std::string MTA_Logger::logHeadStamp(int lvl)
{
	return (timeStamp() + processIdStamp() + threadIdStamp() + logLvlStamp(lvl));
}

MTA_Logger::MTA_Logger()
{
	m_bInited.store(false);
}

std::string const & MTA_Logger::logLvlStamp(int lvl)
{
	switch (lvl)
	{
	case lgl_crit:
		return LOG_TAG_CRIT;
	case lgl_error:
		return LOG_TAG_ERROR;
	case lgl_warn:
		return LOG_TAG_WARN;
	case lgl_info:
		return LOG_TAG_INFO;
	case lgl_vbs:
		return LOG_TAG_VBS;
	case lgl_dbg:
		return LOG_TAG_DBG;
	case lgl_perf:
		return LOG_TAG_PERF;
	case lgl_trace:
		return LOG_TAG_TRACE;
	default:
		break;
	}
	return LOG_TAG_Unkown;
}

std::string const& MTA_Logger::processIdStamp()
{
#ifdef WIN32
	if (m_strPid.empty())
	{
		char bf[16] = { 0 };
		DWORD pid = GetCurrentProcessId();
		snprintf(bf, 15, "[Px0X%X]", pid);
		m_strPid = bf;
	}
#endif // WIN32
	return m_strPid;
}

MTA_Logger * MTA_Logger::Instance()
{
#ifdef WITH_DEBUG_LOG
	static MTA_Logger lg;
	return &lg;
#else
	return nullptr;
#endif // WITH_DEBUG_LOG
}

int MTA_Logger::log_open(std::string const & name, bool appendMode, int output, int lvl)
{
#ifdef WITH_DEBUG_LOG
	if (m_bInited.load())
	{
		return -1;
	}
	m_lvl = lvl;
	m_op = output;
	if (!(m_op & lgo_file))
	{
		if (m_fp)
		{
			return -1;
		}
		m_bInited.store(true);
		return 0;
	}
	if (m_fp || name.empty())
	{
		return -1;
	}
#ifdef WIN32
	if (m_strPid.empty())
	{
		char bf[16] = { 0 };
		DWORD pid = GetCurrentProcessId();
		snprintf(bf, 15, "[Px0X%X]", pid);
		m_strPid = bf;
	}
#else
	m_strPid = "[Px0X----]";
#endif // WIN32
	std::string mode = "wb";
	if (appendMode)
	{
		mode = "ab";
	}
#ifdef WIN32
	int ret = fopen_s(&m_fp, name.c_str(), mode.c_str());
	if (ret != 0)
	{
		return ret;
	}
#else
	m_fp = fopen(name.c_str(), mode.c_str());
	if (!m_fp)
	{
		return -1;
	}
#endif // WIN32
	std::string sFileHead = "****************************************"
		"****************************************\n"
		"*\tLog File Open ...\n"
		"*\tLogFileName: [" + name + "]\n"
		"*\tCreateTime: " + timeStamp() +
		"\n*\tPid: " + m_strPid + 
		"\n****************************************"
		"****************************************\n";
	fwrite(sFileHead.c_str(), 1, sFileHead.length(), m_fp);
	
#endif
	m_bInited.store(true);
	return 0;
}

int MTA_Logger::log_close()
{
#ifdef WITH_DEBUG_LOG
	if (m_fp == nullptr)
	{
		return -1;
	}
	std::string sTail = "****************************************"
		"****************************************\n"
		"*\tClose Log File ..."
		"\n****************************************"
		"****************************************\n";
	fwrite(sTail.c_str(), 1, sTail.length(), m_fp);
	fclose(m_fp);
	m_fp = nullptr;
#endif // WITH_DEBUG_LOG
	m_bInited.store(false);
	return 0;
}

void MTA_Logger::log_crit(const char * fmt, ...)
{
#ifdef WITH_DEBUG_LOG
	if (!(m_lvl & lgl_crit))
	{
		return;
	}
	if (m_fp == nullptr && !(m_op & lgo_console))
	{
		return;
	}
	HANDLE_VAR_ARG;
	//std::size_t sz = strnlen(msg, LOG_PCS_MAX_SIZE - 1);
	//std::string lgh = logHeadStamp(lgl_crit);
	std::string pcs = "";
	pcs = pcs.append(logHeadStamp(lgl_crit).c_str());
	pcs = pcs.append(msg);
	pcs = pcs.append("\n");
	if (m_fp)
	{
		//fwrite(lgh.c_str(), 1, lgh.length(), m_fp);
		//fwrite(msg, 1, sz, m_fp);
		//fwrite("\n", 1, 1, m_fp);
		fwrite(pcs.c_str(), pcs.length(), 1, m_fp);
	}
	if (m_op & lgo_console)
	{
		//printf("%s%s\n", lgh.c_str(), msg);
		printf("%s", pcs.c_str());
	}
#endif // WITH_DEBUG_LOG
}

void MTA_Logger::log_error(const char * fmt, ...)
{
#ifdef WITH_DEBUG_LOG
	if (!(m_lvl & lgl_error))
	{
		return;
	}
	if (m_fp == nullptr && !(m_op & lgo_console))
	{
		return;
	}
	HANDLE_VAR_ARG;
	//std::size_t sz = strnlen(msg, LOG_PCS_MAX_SIZE - 1);
	//std::string lgh = logHeadStamp(lgl_error);
	std::string pcs = "";
	pcs = pcs.append(logHeadStamp(lgl_error).c_str());
	pcs = pcs.append(msg);
	pcs = pcs.append("\n");
	if (m_fp)
	{
		//fwrite(lgh.c_str(), 1, lgh.length(), m_fp);
		//fwrite(msg, 1, sz, m_fp);
		//fwrite("\n", 1, 1, m_fp);
		fwrite(pcs.c_str(), pcs.length(), 1, m_fp);
	}
	if (m_op & lgo_console)
	{
		//printf("%s%s\n", lgh.c_str(), msg);
		printf("%s", pcs.c_str());
	}
#endif // WITH_DEBUG_LOG
}

void MTA_Logger::log_warn(const char * fmt, ...)
{
#ifdef WITH_DEBUG_LOG
	if (!(m_lvl & lgl_warn))
	{
		return;
	}
	if (m_fp == nullptr && !(m_op & lgo_console))
	{
		return;
	}
	HANDLE_VAR_ARG;
	std::size_t sz = strnlen(msg, LOG_PCS_MAX_SIZE - 1);
	std::string lgh = logHeadStamp(lgl_warn);
	if (m_fp)
	{
		fwrite(lgh.c_str(), 1, lgh.length(), m_fp);
		fwrite(msg, 1, sz, m_fp);
		fwrite("\n", 1, 1, m_fp);
	}
	if (m_op & lgo_console)
	{
		printf("%s%s\n", lgh.c_str(), msg);
	}
#endif // WITH_DEBUG_LOG
}

void MTA_Logger::log_info(const char * fmt, ...)
{
#ifdef WITH_DEBUG_LOG
	if (!(m_lvl & lgl_info))
	{
		return;
	}
	if (m_fp == nullptr && !(m_op & lgo_console))
	{
		return;
	}
	HANDLE_VAR_ARG;
	std::size_t sz = strnlen(msg, LOG_PCS_MAX_SIZE - 1);
	std::string lgh = logHeadStamp(lgl_info);
	if (m_fp)
	{
		fwrite(lgh.c_str(), 1, lgh.length(), m_fp);
		fwrite(msg, 1, sz, m_fp);
		fwrite("\n", 1, 1, m_fp);
	}
	if (m_op & lgo_console)
	{
		printf("%s%s\n", lgh.c_str(), msg);
	}
#endif // WITH_DEBUG_LOG
}

void MTA_Logger::log_vbs(const char * fmt, ...)
{
#ifdef WITH_DEBUG_LOG
	if (!(m_lvl & lgl_vbs))
	{
		return;
	}
	if (m_fp == nullptr && !(m_op & lgo_console))
	{
		return;
	}
	HANDLE_VAR_ARG;
	std::size_t sz = strnlen(msg, LOG_PCS_MAX_SIZE - 1);
	std::string lgh = logHeadStamp(lgl_vbs);
	if (m_fp)
	{
		fwrite(lgh.c_str(), 1, lgh.length(), m_fp);
		fwrite(msg, 1, sz, m_fp);
		fwrite("\n", 1, 1, m_fp);
	}
	if (m_op & lgo_console)
	{
		printf("%s%s\n", lgh.c_str(), msg);
	}
#endif // WITH_DEBUG_LOG
}

void MTA_Logger::log_dbg(const char * fmt, ...)
{
#ifdef WITH_DEBUG_LOG
	if (!(m_lvl & lgl_dbg))
	{
		return;
	}
	if (m_fp == nullptr && !(m_op & lgo_console))
	{
		return;
	}
	HANDLE_VAR_ARG;
	std::size_t sz = strnlen(msg, LOG_PCS_MAX_SIZE - 1);
	std::string lgh = logHeadStamp(lgl_dbg);
	if (m_fp)
	{
		fwrite(lgh.c_str(), 1, lgh.length(), m_fp);
		fwrite(msg, 1, sz, m_fp);
		fwrite("\n", 1, 1, m_fp);
	}
	if (m_op & lgo_console)
	{
		printf("%s%s\n", lgh.c_str(), msg);
	}
#endif // WITH_DEBUG_LOG
}

void MTA_Logger::log_perf(std::string const & msg, unsigned long el)
{
#ifdef WITH_DEBUG_LOG
	if (!(m_lvl & lgl_perf))
	{
		return;
	}
	if (m_fp == nullptr && !(m_op & lgo_console))
	{
		return;
	}
	std::string lgh = logHeadStamp(lgl_perf);
	char bf[64] = {0};
	snprintf(bf, 63, " Time Elapesed: %lu", el);
	std::string s = bf;
	if (m_fp)
	{
		fwrite(lgh.c_str(), 1, lgh.length(), m_fp);
		fwrite(msg.c_str(), 1, msg.length(), m_fp);
		fwrite(s.c_str(), 1, s.length(), m_fp);
		fwrite("\n", 1, 1, m_fp);
	}
	if (m_op & lgo_console)
	{
		printf("%s%s%s\n", lgh.c_str(), msg.c_str(), bf);
	}
#endif // WITH_DEBUG_LOG
}

void MTA_Logger::log_trace(std::string const & msg)
{
#ifdef WITH_DEBUG_LOG
	if (!(m_lvl & lgl_trace))
	{
		return;
	}
	if (m_fp == nullptr && !(m_op & lgo_console))
	{
		return;
	}
	std::string lgh = logHeadStamp(lgl_trace);
	if (m_fp)
	{
		fwrite(lgh.c_str(), 1, lgh.length(), m_fp);
		fwrite(msg.c_str(), 1, msg.length(), m_fp);
		fwrite("\n", 1, 1, m_fp);
	}
	if (m_op & lgo_console)
	{
		printf("%s%s\n", lgh.c_str(), msg.c_str());
	}
#endif // WITH_DEBUG_LOG
}

int MTA_LogOpen(std::string const & name, bool appendMode, int output, int lvl)
{
	if (MTA_Logger::Instance())
	{
		return MTA_Logger::Instance()->log_open(name, appendMode, output, lvl);
	}
	return 0;
}

int MTA_LogOpen(MTA_LogCfg const & lgcfg)
{
	return MTA_LogOpen(lgcfg.path(), lgcfg.append(), lgcfg.output(), lgcfg.level());
}

int MTA_LogClose()
{
	if (MTA_Logger::Instance())
	{
		return MTA_Logger::Instance()->log_close();
	}
	return 0;
}

MTA_Logger::MTA_Logger_Perf::MTA_Logger_Perf(std::string const & msg) : m_msg(msg)
{
#ifdef WIN32
	m_start = GetTickCount();
#else
	//
#endif // WIN32
}

MTA_Logger::MTA_Logger_Perf::~MTA_Logger_Perf()
{
#ifdef WIN32
	m_end = GetTickCount();
	m_elapse = m_end - m_start;
#else
	//
#endif // WIN32
	if (MTA_Logger::Instance())
		MTA_Logger::Instance()->log_perf(m_msg, m_elapse);
}

MTA_Logger::MTA_Logger_Tracer::MTA_Logger_Tracer(std::string const & msg)
	: m_msg(msg)
{
	if (MTA_Logger::Instance())
	{
		std::string s = m_msg + m_enter;
		MTA_Logger::Instance()->log_trace(s);
	}
}

MTA_Logger::MTA_Logger_Tracer::~MTA_Logger_Tracer()
{
	if (MTA_Logger::Instance())
	{
		std::string s = m_msg + m_return;
		MTA_Logger::Instance()->log_trace(s);
	}
}

std::string const& MTA_LogCfg::path() const
{
	return m_path;
}

int MTA_LogCfg::output() const
{
	return m_op;
}

int MTA_LogCfg::level() const
{
	return m_lvl;
}

bool MTA_LogCfg::append() const
{
	return m_ap;
}

void MTA_LogCfg::setOutput(int op)
{
	m_op = op;
}

void MTA_LogCfg::setLevel(int lvl)
{
	m_lvl = lvl;
}

void MTA_LogCfg::setAppend(bool ap)
{
	m_ap = ap;
}

void MTA_LogCfg::setPath(std::string const & path)
{
	m_path = path;
}
