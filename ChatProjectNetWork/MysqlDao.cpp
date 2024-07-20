#include "MysqlDao.h"
#include "const.h"
#include "ConfigMgr.h"
#include <sstream>
MysqlDao::MysqlDao(){
    auto& cfg = ConfigMgr::Inst();
    const auto& host = cfg["Mysql"]["Host"];
    const auto& port = cfg["Mysql"]["Port"];
    const auto& pwd = cfg["Mysql"]["Passwd"];
    const auto& schema = cfg["Mysql"]["Schema"];
    const auto& user = cfg["Mysql"]["User"];
    pool_.reset(new MySqlPool(host + ":" + port, user, pwd, schema, 5));
}
MysqlDao::~MysqlDao() {
    pool_->Close();
}
int MysqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
    auto con = pool_->getConnection();
    try {
        if (con == nullptr) {
//            pool_->returnConnection(std:1:move(con));
            return false;
        }
        // ȷ�� con->_con ��Ϊ nullptr
        if (con->_con == nullptr) {
            pool_->returnConnection(std::move(con));
            return false;
        }
        // ׼�����ô洢����
        std::unique_ptr<sql::PreparedStatement> stmt(con->_con->prepareStatement("CALL chat.reg_user(?,?,?,@result)"));
        // �����������
        stmt->setString(1, name);
        stmt->setString(2, email);
        stmt->setString(3, pwd);
        // ����PreparedStatement��ֱ��֧��ע�����������������Ҫʹ�ûỰ������������������ȡ���������ֵ
          // ִ�д洢����
        stmt->execute();
        // ����洢���������˻Ự��������������ʽ��ȡ���������ֵ�������������ִ��SELECT��ѯ����ȡ����
       // ���磬����洢����������һ���Ự����@result���洢������������������ȡ��
        std::unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));
        if (res->next()) {
            int result = res->getInt("result");
            log4cpp::Category::getInstance("server").infoStream() << "Result:" << result;;
            pool_->returnConnection(std::move(con));
            return result;
        }
        pool_->returnConnection(std::move(con));
        return -1;
    }catch (sql::SQLException& e ) {
        pool_->returnConnection(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return -1;
    }
}

bool MysqlDao::checkEmail(const std::string& name, const std::string& email){
    auto con = pool_->getConnection();
    try {
        if (con == nullptr) {
            pool_->returnConnection(std::move(con));
            return false;
        }
        // ׼����ѯ���
        std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT email FROM user WHERE name = ?"));
        // �󶨲���
        pstmt->setString(1, name);
        // ִ�в�ѯ
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        // ���������
        while (res->next()) {
            std::cout << "Check Email: " << res->getString("email") << std::endl;
            if (email != res->getString("email")) {
                pool_->returnConnection(std::move(con));
                return false;
            }
            pool_->returnConnection(std::move(con));
            return true;
        }
    }
    catch (sql::SQLException& e) {
        pool_->returnConnection(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool MysqlDao::updatePasswd(const std::string& name, const std::string& pwd){
    auto con = pool_->getConnection();
    try {
        if (con == nullptr) {
            pool_->returnConnection(std::move(con));
            return false;
        }
        // ׼����ѯ���
        std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("UPDATE user SET pwd = ? WHERE name = ?"));
        // �󶨲���
        pstmt->setString(2, name);
        pstmt->setString(1, pwd);
        // ִ�и���
        int updateCount = pstmt->executeUpdate();
        std::cout << "Updated rows: " << updateCount << std::endl;
        pool_->returnConnection(std::move(con));
        return true;
    }
    catch (sql::SQLException& e) {
        pool_->returnConnection(std::move(con));
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return false;
    }
}

bool MysqlDao::checkPwd(const std::string& name, const std::string& pwd, UserInfo& user_info)
{
    
}
