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
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
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
#include "data.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

#define CODE_PREFIX "code_"
#define USERIPPREFIX  "uip_"
#define USERTOKENPREFIX  "utoken_"

enum ErrorCodes {
	SUCCESS = 0,
	ERROR_JSON = 1001,
	ERROR_RPCFAILED = 1002,
	ERROR_USRE_EXIST = 1003,
	ERROR_VERIFY_EXPRIRED = 1004,
	ERROR_VERIFY_CODE = 1005,
	ERROR_PASSWD = 1006,
	ERROR_EMAIL_NOT_MATCH = 1007,
	ERROR_PASSWD_UP_FAILED = 1008,
	ERROR_UID_INVALID = 1009,
	ERROR_TOKEN_INVALID = 1010,
};
#define USERIPPREFIX  "uip_"
#define USERTOKENPREFIX  "utoken_"
#define IPCOUNTPREFIX  "ipcount_"
#define USER_BASE_INFO "ubaseinfo_"
#define LOGIN_COUNT  "logincount"
#define NAME_INFO  "nameinfo_"
