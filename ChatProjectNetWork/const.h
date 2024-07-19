#pragma once
///****************************************************************************
/// @Author      : haoks                                                       
/// @Date        : 2024/7/10
/// @Description : 引入基本的头文件                                                           
///****************************************************************************
#define BOOST_LIB_DIAGNOSTIC
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>
#include <memory>
#include <exception>
#include <iostream>
#include <unordered_map>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "hiredis.h"


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

enum ErrorCodes {
	SUCCESS = 0,
	ERROR_JSON = 1001,
	RPCFAILED = 1002,
	USRE_EXIST = 1003,
	verify_EXPRIRED = 1004,
	verify_CODE_ERROR = 1005,
	PASSWDERR = 1006,
	EMAIL_NOT_MATCH = 1007,
	PASSWD_UP_FAILED = 1008
};

#define CODE_PREFIX "code_"