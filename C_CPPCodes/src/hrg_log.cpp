#include "hrg_log.h"

// 基类方法的实现：
HrgLogger::~HrgLogger()
{
	destroy_logger();
}
 
 
void HrgLogger::generate_file_name_automaticaly()
{
	struct tm *cur_time;
	time_t local_time;
	time(&local_time);
	cur_time = localtime(&local_time);
 
	/* 通过时间命名日志文件 */
	char filename[200];
	strftime(filename, 100, "%Y-%m-%d_%H-%M-%S.log", cur_time);
 
	/* log file name, does not contain file path */
	std::string log_file_name = std::string(filename);
 
	/* 判断日志路径是否存在，若不存在，则创建 */
	/* Check if the input log path exists, if not, create the path */
	if(access(log_path.c_str(), F_OK) != 0)
	{
		mkdir(log_path.c_str()); //linux: , S_IRWXU
	}
 
	/* 日志文件全路径 */
	log_full_name = log_path + log_file_name;
 
	/* 记录日志创建时间，在通过时间长度控制日志文件大小时，该成员变量会被使用 */
	logger_create_time = local_time;
 
}
 
/* 除了使用时间命名日志文件，用户还可以自定义日志文件名称 */
void HrgLogger::set_specified_file_name(std::string filename)
{
	/* Check if the input log path exists, if not, create the path */
	if(access(log_path.c_str(), F_OK) != 0)
	{
		mkdir(log_path.c_str()); //linux: , S_IRWXU
	}
	
	/* Make full log path */
	log_full_name = log_path + filename;
}

/* 创建控制台logger */
void ConsoleLogger::create_logger()
{
	try
	{	
		hrg_logger = spdlog::stdout_color_mt("console");
		//hrg_logger->set_pattern("%+");	
		hrg_logger->set_pattern("[%Y-%m-%d %T][thread %t][%l]%v");
	}
	catch(const spdlog::spdlog_ex &ex)
	{
		std::cout << "Create console logger failed: " << ex.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	
	if(!hrg_logger)
	{
		std::cout << "Create console logger failed." << std::endl;
	}
	else
	{
		std::cout << "Create console logger seccessfully." << std::endl;
		logger_created = true;
	}
 
}

/* 创建文件logger */
void FileLogger::create_logger()
{
	try
	{
		hrg_logger = spdlog::basic_logger_mt("basic_logger", log_full_name.c_str());	
		
		spdlog::set_pattern("[%Y-%m-%d %T][%l]%v"); 
		hrg_logger->set_level(spdlog::level::trace);
	}
	catch(const spdlog::spdlog_ex &ex)
	{
		std::cout << "Create file logger failed: " << ex.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	
	if(!hrg_logger)
	{
		std::cout << " Create file logger failed." << std::endl;
	}
	else
	{
		std::cout << "Create file logger successfully." << std::endl;
		logger_created = true;
	}
}


/* 创建复合logger */
void MultiLogger::create_logger()
{
	try
	{
		/* 通过multi-sink的方式创建复合logger，实现方式为：先分别创建文件sink和控制台sink，并将两者放入sink 向量中，组成一个复合logger */
		/* file sink */
		std::cout << "MultiLogger: log_full_name = " << log_full_name << std::endl;
		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_full_name.c_str(), true);
		file_sink->set_level(spdlog::level::trace);
		file_sink->set_pattern("[%Y-%m-%d %T][%l]%v");
		std::cout << "MultiLogger: create file sink OK." << std::endl;
		
		/* 控制台sink */
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::trace);
		console_sink->set_pattern("%+");
		std::cout << "MultiLogger: create console sink OK." << std::endl;
		
		/* Sink组合 */
		std::vector<spdlog::sink_ptr> sinks;
		sinks.push_back(console_sink);
		sinks.push_back(file_sink);
		hrg_logger = std::make_shared<spdlog::logger>("multi-sink", begin(sinks), end(sinks));
		
		std::cout << "MultiLogger: create multi sink OK." << std::endl;
		
	}
	catch(const spdlog::spdlog_ex &ex)
	{
		std::cout << "Create multi-logger failed: " << ex.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	
	if(!hrg_logger)
	{
		std::cout << " Create multi-logger failed." << std::endl;
	}
	else
	{
		std::cout << "Create multi-logger successfully." << std::endl;
		logger_created = true;
	}
	
}

/* 通过指定输出名称选择logger */
HrgLogger* LoggerSelector::select_logger(std::string out_type)
{
	HrgLogger *p_logger = NULL;
	
	if(out_type == "console")
	{
		p_logger = new ConsoleLogger();
		p_logger->create_logger();
		
	}
	else if(out_type == "file")
	{
		p_logger = new FileLogger(path);
		std::cout << "For file logger, path = " << path << std::endl;
		p_logger->generate_file_name_automaticaly();
		p_logger->create_logger();
	}
	else if(out_type == "both")
	{
		p_logger = new MultiLogger(path);
		p_logger->generate_file_name_automaticaly();
		p_logger->create_logger();
	}
	else 
	{
		std::cout << "Unsupported logger type!" << std::endl;
		return NULL;
	}
	
	return p_logger;
}
 
 
/* 通过日志模式选择logger */
HrgLogger* LoggerSelector::select_logger(int mode)
{
	HrgLogger *p_logger = NULL;
	
	switch(mode)
	{
		case LAB_MODE:
		{
			p_logger = new ConsoleLogger();
			p_logger->create_logger();
			p_logger->set_print_level(LOG_LEVEL_TRACE);
			
			break;			
		}
		case TRIAL_MODE:
		{
			p_logger = new MultiLogger(path);
			p_logger->generate_file_name_automaticaly();
			p_logger->create_logger();
			p_logger->set_print_level(LOG_LEVEL_DEBUG);
			
			break;
		}
		case USER_MODE:
		{
			p_logger = new FileLogger(path);
			p_logger->generate_file_name_automaticaly();
			p_logger->create_logger();
			p_logger->set_print_level(LOG_LEVEL_INFO);
			
			break;
		}
		default:
		{
			break;
		}
	}
 
	return p_logger;
}

