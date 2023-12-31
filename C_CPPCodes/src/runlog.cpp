
#include <iostream>
#include <string>
#include <unistd.h>
#include "hrg_log.h"

// https://blog.csdn.net/DeliaPu/article/details/117733659?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_baidulandingword~default-0-117733659-blog-128780480.235^v40^pc_relevant_default_base&spm=1001.2101.3001.4242.1&utm_relevant_index=3
void multi_sink_example()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::warn);
    console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log/multisink.txt", true);
    file_sink->set_level(spdlog::level::trace);

#if 0
    spdlog::logger logger("multi_sink", {console_sink, file_sink});
    logger.set_level(spdlog::level::debug);
    logger.warn("this should appear in both console and file");
    logger.info("this message should not appear in the console, only in the file");
#endif

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(console_sink);
    sinks.push_back(file_sink);
    std::shared_ptr<spdlog::logger> p_logger;
    p_logger = std::make_shared<spdlog::logger>("multi_sink", begin(sinks), end(sinks));

    p_logger->warn("this should appear in both console and file");
    p_logger->info("this message should not appear in the console, only in the file");
}

#if 0
int main()
{
	std::string file_path = "./log/";
	
	/* 测试控制台日志 */
	HrgLogger *my_console_logger = new ConsoleLogger();
	if(!my_console_logger)
	{
		std::cout << "Create console logger failed!" << std::endl;
		exit(1);
	}
	my_console_logger->create_logger();
	my_console_logger->set_print_level(LOG_LEVEL_TRACE);
	
	my_console_logger->print_trace("This is a trace message for console logger, number {}.", 1);
	my_console_logger->print_debug("This is a debug message for console logger, number {}.", 2);
	my_console_logger->print_info("This is a info message for console logger, number {}.", 3);
	my_console_logger->print_warn("This is a warn message for console logger, number {}.", 4);
	my_console_logger->print_error("This is a error message for console logger, number {}.", 5);
	
	delete my_console_logger;
	
	
	/* 测试文件日志 */
	HrgLogger *my_file_logger = new FileLogger(file_path);
	if(!my_file_logger)
	{
		std::cout << "Create file logger failed!" << std::endl;
		exit(1);
	}
	my_file_logger->set_specified_file_name("myfilelog.txt");  //测试指定日志名称
	my_file_logger->create_logger();
	my_file_logger->set_print_level(LOG_LEVEL_INFO);
	
	my_file_logger->print_trace("This is a(n) {} message for file logger.", "trace");
	my_file_logger->print_debug("This is a(n) {} message for file logger.", "debug");
	my_file_logger->print_info("This is a(n) {} message for file logger.", "info");
	my_file_logger->print_warn("This is a(n) {} message for file logger.", "warn");
	my_file_logger->print_error("This is a(n) {} message for file logger.", "error");
	
	delete my_file_logger;
	
	sleep(3);
	
	/* 测试复合日志 */
	HrgLogger *my_multi_logger = new MultiLogger(file_path);
	if(!my_multi_logger)
	{
		std::cout << "Create multi logger failed!" << std::endl;
		exit(1);
	}
	
	my_multi_logger->generate_file_name_automaticaly();  //测试自动生成日志名称
	my_multi_logger->create_logger();
	my_multi_logger->set_print_level(LOG_LEVEL_DEBUG);
	
	my_multi_logger->print_trace("This is a trace message for multi logger.");
	my_multi_logger->print_debug("This is a debug message for multi logger.");
	my_multi_logger->print_info("This is a info message for multi logger.");
	my_multi_logger->print_warn("This is a warn message for multi logger.");
	my_multi_logger->print_error("This is a error message for multi logger.");
	
	delete my_multi_logger;
 
}
#endif

int main()
{
    std::string file_path = "./log/";

    LoggerSelector *p_selector = new LoggerSelector(file_path);

    /* 测试控制台日志 */
    HrgLogger *my_console_logger = p_selector->select_logger("console");
    if (!my_console_logger)
    {
        std::cout << "Create console logger failed!" << std::endl;
        exit(1);
    }
    // my_console_logger->create_logger();
    my_console_logger->set_print_level(LOG_LEVEL_TRACE);

    my_console_logger->print_trace("This is a trace message for console logger, number {}.", 1);
    my_console_logger->print_debug("This is a debug message for console logger, number {}.", 2);
    my_console_logger->print_info("This is an info message for console logger, number {}.", 3);
    my_console_logger->print_warn("This is a warn message for console logger, number {}.", 4);
    my_console_logger->print_error("This is an error message for console logger, number {}.", 5);

    delete my_console_logger;

    /* 测试文件日志 */
    HrgLogger *my_file_logger = p_selector->select_logger("file");
    if (!my_file_logger)
    {
        std::cout << "Create file logger failed!" << std::endl;
        exit(1);
    }
    my_file_logger->set_print_level(LOG_LEVEL_INFO);

    my_file_logger->print_trace("This is a(n) {} message for file logger.", "trace");
    my_file_logger->print_debug("This is a(n) {} message for file logger.", "debug");
    my_file_logger->print_info("This is a(n) {} message for file logger.", "info");
    my_file_logger->print_warn("This is a(n) {} message for file logger.", "warn");
    my_file_logger->print_error("This is a(n) {} message for file logger.", "error");

    delete my_file_logger;

    /* 延迟3秒，以免复合日志中的文件名称与文件日志的文件名称冲突 */
    sleep(3);

    /* 测试复合日志 */
    HrgLogger *my_multi_logger = p_selector->select_logger("both");
    if (!my_multi_logger)
    {
        std::cout << "Create multi logger failed!" << std::endl;
        exit(1);
    }

    my_multi_logger->generate_file_name_automaticaly(); // 测试自动生成日志名称
    my_multi_logger->set_print_level(LOG_LEVEL_DEBUG);

    my_multi_logger->print_trace("This is a trace message for multi logger.");
    my_multi_logger->print_debug("This is a debug message for multi logger.");
    my_multi_logger->print_info("This is an info message for multi logger.");
    my_multi_logger->print_warn("This is a warn message for multi logger.");
    my_multi_logger->print_error("This is an error message for multi logger.");

    delete my_multi_logger;

    delete p_selector;

    return 0;
}