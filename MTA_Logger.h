/**********************************************************************
 * @FileName: MTA_Logger.h
 * @Author: Frodo Cheng
 * @CreatedTime: May 17th 2019
***********************************************************************/
#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>
#include <stdarg.h>
#include <atomic>

#ifdef WIN32

#include <Windows.h>
#include <atlstr.h>

#endif // WIN32

#define WITH_DEBUG_LOG
// #undef WITH_DEBUG_LOG

#ifdef WITH_DEBUG_LOG
#pragma message("#define WITH_DEBUG_LOG, debug log enabled!")
#else
#pragma message("#undef WITH_DEBUG_LOG, debug log disabled!")
#endif // WITH_DEBUG_LOG


enum
{
	lgo_console = 1,
	lgo_file = 2,
	lgo_all = lgo_console | lgo_file
};

enum
{
	lgl_crit = 1,
	lgl_error = 2,
	lgl_warn = 4,
	lgl_low = lgl_crit | lgl_error | lgl_warn,
	lgl_info = 8,
	lgl_vbs = 16,
	lgl_dbg = 32,
	lgl_perf = 64,
	lgl_trace = 128,
	lgl_all = -1
};

class MTA_LogCfg
{
public:
	std::string const& path() const;
	int output() const;
	int level() const;
	bool append() const;

	void setOutput(int op);
	void setLevel(int lvl);
	void setAppend(bool ap);
	void setPath(std::string const& path);
private:
	std::string		m_path = "";
	int				m_op = 0;
	int				m_lvl = 0;
	bool			m_ap = false;
};

/**
 * @description:
 *		This API is not thread safe, must call in main thread before all other threads running.
**/
int MTA_LogOpen(std::string const& name, bool appendMode = false, int output = lgo_file, int lvl = lgl_low);

/**
 * @description:
 *		This API is not thread safe, must call in main thread before all other threads running.
**/
int MTA_LogOpen(MTA_LogCfg const& lgcfg);

/**
 * @description:
 *		This API is not thread safe, must call in main thread and after all other threads joined
**/
int MTA_LogClose();

#define		MTA_LogCrit(...)	if (MTA_Logger::Instance()) MTA_Logger::Instance()->log_crit(__VA_ARGS__);
#define		MTA_LogError(...)	if (MTA_Logger::Instance()) MTA_Logger::Instance()->log_error(__VA_ARGS__);
#define		MTA_LogWarn(...)	if (MTA_Logger::Instance()) MTA_Logger::Instance()->log_warn(__VA_ARGS__);
#define		MTA_LogInfo(...)	if (MTA_Logger::Instance()) MTA_Logger::Instance()->log_info(__VA_ARGS__);
#define		MTA_LogVbs(...)		if (MTA_Logger::Instance()) MTA_Logger::Instance()->log_vbs(__VA_ARGS__);
#define		MTA_LogDbg(...)		if (MTA_Logger::Instance()) MTA_Logger::Instance()->log_dbg(__VA_ARGS__);
#define		MTA_LogPerf(msg)	MTA_Logger::MTA_Logger_Perf	perf(msg)
#define		MTA_LogPerfer(msg)	MTA_Logger::MTA_Logger_Perf	perfer(#msg)
#define		MTA_LogTrace(msg)	MTA_Logger::MTA_Logger_Tracer	trace(msg)
#define		MTA_LogTracer(msg)	MTA_Logger::MTA_Logger_Tracer	tracer(#msg)


class MTA_Logger
{
public:
	static MTA_Logger* Instance();

	//
	friend int MTA_LogOpen(std::string const& name, bool appendMode, int output, int lvl);
	friend int MTA_LogClose();

	void log_crit(const char* fmt, ...);
	void log_error(const char* fmt, ...);
	void log_warn(const char* fmt, ...);
	void log_info(const char* fmt, ...);
	void log_vbs(const char* fmt, ...);
	void log_dbg(const char* fmt, ...);
	void log_perf(std::string const& msg, unsigned long el);
	void log_trace(std::string const& msg);

private:
	int log_open(std::string const& name, bool appendMode = false, int output = lgo_file, int lvl = lgl_low);
	int log_close();

	std::string const& logLvlStamp(int lvl);
	std::string const& processIdStamp();
	std::string threadIdStamp();
	std::string timeStamp();
	std::string logHeadStamp(int lvl);
private:
	MTA_Logger();
	MTA_Logger(MTA_Logger const&) = delete;

	FILE*	m_fp	= nullptr;
	int		m_op	= 0;
	int		m_lvl	= 0;
	std::string m_strPid = "";
	std::atomic<bool>	m_bInited;

public:
	class MTA_Logger_Perf
	{
	public:
		MTA_Logger_Perf(std::string const& msg);
		~MTA_Logger_Perf();
		unsigned long	m_start = 0;
		unsigned long	m_end = 0;
		unsigned long	m_elapse = 0;
		std::string		m_msg = "";
	};
	class MTA_Logger_Tracer
	{
	public:
		MTA_Logger_Tracer(std::string const& msg);
		~MTA_Logger_Tracer();
		std::string m_msg = "";
		std::string m_enter = " enter ...";
		std::string m_return = " return !";
	};
};
