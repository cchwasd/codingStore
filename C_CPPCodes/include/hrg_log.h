#ifndef __HRG_LOG__
#define __HRG_LOG__
 
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
 
 
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/logger.h"
 
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/common.h"
 
enum LoggerMode
{
	LAB_MODE = 0,  //实验室模式，打印比较细致的信息
	TRIAL_MODE = 1,   //试用模式，打印调试信息
	USER_MODE = 2,    //用户模式，打印较重要的信息
};
 
#define LOG_MODE USER_MODE
 
using namespace spdlog;
 
#define LOG_LEVEL_TRACE spdlog::level::trace
#define LOG_LEVEL_DEBUG spdlog::level::debug
#define LOG_LEVEL_INFO spdlog::level::info
#define LOG_LEVEL_WARN spdlog::level::warn
#define LOG_LEVEL_ERROR spdlog::level::err
#define LOG_LEVEL_CRITICAL spdlog::level::critical
#define LOG_LEVEL_OFF spdlog::level::off
 
using print_level = spdlog::level::level_enum;    //变量重命名
using format_string = string_view_t;      // 变量重命名 //wstring_view_t;   //fmt::basic_string_view<char>;
 
 
class HrgLogger
{
public:
	HrgLogger()
	{
		logger_created = false;
		logger_droped = false;
	}
	
	~HrgLogger();
	
	std::shared_ptr<spdlog::logger> hrg_logger;
	
	bool logger_created;         //标识日志创建状态
	bool logger_droped;          //标识日志关闭状态
	time_t logger_create_time;   //日志创建时间
 
	/* Generate log file name automatically according to real time, usually used when many real-time log files should be output */
	virtual void generate_file_name_automaticaly();
 
	/* Set a specified log file name, usually used when only one log file is needed.
		input param @filename is the specified filename.
	  */
	void set_specified_file_name(std::string filename);
 
	/* The abstract api for create logger, would be realized by the derived classes */
	virtual void create_logger() = 0;   //创建logger的纯虚函数，只能在子函数中实现
		
	/* 设置日志的打印级别 */
	void set_print_level(print_level lvl)
	{
		hrg_logger->set_level(lvl);
	}
 
	/* 重新封装logger的trace, debug, warn, error和critical打印接口 */
	template<typename... Args>
	inline void print_trace(format_string fmt, const Args &... args)
	{
		hrg_logger->trace(fmt, args...);
	}
	
	template<typename... Args>
	inline void print_debug(format_string fmt, const Args &... args)
	{
		hrg_logger->debug(fmt, args...);
	}
	
	template<typename... Args>
	inline void print_info(format_string fmt, const Args &... args)
	{
		hrg_logger->info(fmt, args...);
	}
	
	template<typename... Args>
	inline void print_warn(format_string fmt, const Args &... args)
	{
		hrg_logger->warn(fmt, args...);
	}
	
	template<typename... Args>
	inline void print_error(format_string fmt, const Args &... args)
	{
		hrg_logger->error(fmt, args...);
	}
 
	template<typename... Args>
	inline void print_critical(format_string fmt, const Args &... args)
	{
		hrg_logger->critical(fmt, args...);
	}
	
	/* 销毁logger */
	void destroy_logger()
	{
		spdlog::drop_all();
		logger_droped = true;
	}
	
protected:
	std::string log_full_name;  //The full name contains log_path and log_file_name
	std::string log_path;  // The log path which is specified in class constructor
	
};
 
/* 控制台logger子类 */
class ConsoleLogger : public HrgLogger
{
public:
	ConsoleLogger()
	{
		std::cout << "ConsoleLogger constructor." << std::endl;
	}
	
	void generate_file_name_automaticaly(){}  //对于控制台子类来说，无需生成file文件，因此该方法的函数体置空
	
	virtual void create_logger();
	
};
 
/* 文件logger子类 */
class FileLogger : public HrgLogger
{
public:
	FileLogger(std::string str)
	{
		log_path = str;
		std::cout << "FileLogger Constructor." << std::endl;
	}
	
	virtual void create_logger();
};
 
/* 控制台+文件复合logger子类 */
class MultiLogger : public HrgLogger
{
public:
	MultiLogger(std::string str)       
	{
		log_path = str;
		std::cout << "MultiLogger with parameters constructor." << std::endl;
	}
	
	virtual void create_logger();
};
 
/* Logger选择类，通过传入的logger模式生成相应的logger */
class LoggerSelector
{
public:
	LoggerSelector(){}
	LoggerSelector(std::string str)
	{
		path = str;
	}
	~LoggerSelector(){}
	
	HrgLogger* select_logger(std::string out_type);
	HrgLogger* select_logger(int mode);
 
private:
	std::string path;
};
 
 
#endif
 