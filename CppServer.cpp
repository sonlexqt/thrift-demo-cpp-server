/*
 * INCLUDES
 */
// include thrift generated code
#include "APIs.h"
// include Thrift library
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
// include POCO library files
#include "Poco/Foundation.h"
#include "Poco/Util/PropertyFileConfiguration.h"
#include "Poco/Logger.h"
#include "Poco/FileChannel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/AutoPtr.h"
#include "Poco/SplitterChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/Data/Session.h"
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/SessionFactory.h>
// include other libraries
#include <libmemcached/memcached.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sstream> 
#include <string>

/*
 * USING
 */
// using thrift generated code
using namespace ::thriftDemo;
// using namespaces from Thrift
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace apache::thrift::concurrency;
// using namespaces from POCO
using Poco::AutoPtr;
using Poco::Util::PropertyFileConfiguration;
using Poco::Logger;
using Poco::FileChannel;
using Poco::ConsoleChannel;
using Poco::SplitterChannel;
using Poco::PatternFormatter;
using Poco::FormattingChannel;
using namespace Poco::Data;
using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;
using Poco::Data::MySQL::ConnectionException;
using Poco::Data::MySQL::StatementException;
// using other namespaces
using namespace std;
using boost::shared_ptr;

/*
 * GLOBAL VARS
 */
int port = 0;
int isApplyCache = 0;
std::string mySQLHost = "";
int mySQLPort = 0;
std::string mySQLDb = "";
std::string mySQLUsername = "";
std::string mySQLPassword = "";
Session* mySQLsession;

struct ViewCountInfo {
    std::string username;
    int counter;
};

class Utilities {
public:

    static std::string convertToString(int number) {
        ostringstream convert; // stream used for the conversion
        convert << number; // insert the textual representation of 'Number' in the characters in the stream
        std::string res = convert.str(); // set 'Result' to the contents of the stream
        return res;
    }
};

class APIsHandler : virtual public APIsIf {
private:

    int getRequest(const std::string& _username) {
        int res = -1;
        Session localSession = getSession();
        Statement select(localSession);
        // commented out because of thread conflicts
//        Statement select(*mySQLsession);
        std::string match("'" + _username + "'");
        select << "SELECT counter FROM view_count_info WHERE username = " + match + " LIMIT 1;", into(res);
        select.execute();
        return res;
    }

    void putRequest(const std::string& _username, const int32_t _newValue) {
        Session localSession = getSession();
//        Statement update(*mySQLsession);
        Statement update(localSession);
        std::string match("'" + _username + "'");
        update << "UPDATE view_count_info SET counter=" + Utilities::convertToString(_newValue) + " WHERE username = " + match;
        update.execute();
    }

    Session getSession() {
        Poco::Data::MySQL::Connector::registerConnector();
        std::string sessionStr = "host=";
        sessionStr += mySQLHost;
        sessionStr += ";port=";
        sessionStr += Utilities::convertToString(mySQLPort);
        sessionStr += ";db=";
        sessionStr += mySQLDb;
        sessionStr += ";user=";
        sessionStr += mySQLUsername;
        sessionStr += ";password=";
        sessionStr += mySQLPassword;
        sessionStr += ";compress=true;auto-reconnect=true";
        Session session("MySQL", sessionStr);
        return session;
    }

public:

    APIsHandler() {
        // Your initialization goes here
    }

    bool put(const std::string& _username, const int32_t _newValue) {
        Logger::root().information("handle PUT request");
        // check if existed or not
        int isExisted = getRequest(_username);
        if (isExisted < 0) {
            // the username is not existed yet
            return false;
        } else {
            putRequest(_username, _newValue);
        }
        return true;
    }

    bool increase(const std::string& _username) {
        Logger::root().information("handle INCREASE request");
        // check if existed or not
        int isExisted = getRequest(_username);
        if (isExisted < 0) {
            // the username is not existed yet
            return false;
        } else {
            putRequest(_username, isExisted + 1);
        }
        return true;
    }

    int32_t get(const std::string& _username) {
        Logger::root().information("handle GET request");
        cout << _username << endl;
        int result = getRequest(_username);
        //Logger::root().information("GET: username = " + _username + " ,counter = " + result);
        cout << result << endl;
        return result;
    }

    bool ping() {
        Logger::root().information("handle PING request");
        return true;
    }
};

void getPropertiesInfo() {
    AutoPtr<PropertyFileConfiguration> pConf;
    pConf = new PropertyFileConfiguration("viewcount.properties");
    cout << "=== The following are information get from the viewcount.properties file: ===" << endl;

    int serverPort = pConf->getInt("SERVER_PORT");
    cout << "serverPort: " << serverPort << endl;
    port = serverPort;

    int applyCache = pConf->getInt("APPLY_CACHE");
    cout << "applyCache: " << applyCache << endl;
    isApplyCache = applyCache;

    string authorName = pConf->getString("AUTHOR_NAME");
    cout << "authorName: " << authorName << endl;

    mySQLHost = pConf->getString("MYSQL_HOST");
    cout << "mySQLHost: " << mySQLHost << endl;
    mySQLPort = pConf->getInt("MYSQL_PORT");
    cout << "mySQLPort: " << mySQLPort << endl;

    mySQLDb = pConf->getString("MYSQL_DB");
    cout << "mySQLDb: " << mySQLDb << endl;

    mySQLUsername = pConf->getString("MYSQL_USERNAME");
    cout << "mySQLUsername: " << mySQLUsername << endl;

    mySQLPassword = pConf->getString("MYSQL_PASSWORD");
    cout << "mySQLPassword: " << mySQLPassword << endl;

    cout << "==========" << endl;

}

void initLogger() {
    AutoPtr<FileChannel> pFileChannel(new FileChannel);
    pFileChannel->setProperty("path", "viewCount.log");
    AutoPtr<ConsoleChannel> pConsoleChannel(new ConsoleChannel);

    AutoPtr<SplitterChannel> pSplitter(new SplitterChannel);

    // add pattern formatter, more details go in to the messages
    AutoPtr<PatternFormatter> pPatternFormatter(new PatternFormatter);
    pPatternFormatter->setProperty("pattern", "%Y-%m-%d %H:%M:%S %s: %t");
    AutoPtr<FormattingChannel> pFCFileChannel(new FormattingChannel(pPatternFormatter, pFileChannel));
    AutoPtr<FormattingChannel> pFCConsoleChannel(new FormattingChannel(pPatternFormatter, pConsoleChannel));

    pSplitter->addChannel(pFCConsoleChannel);
    pSplitter->addChannel(pFCFileChannel);

    Logger::root().setChannel(pSplitter);
}

void insertTestData(Session* session) {
    // insert some rows
    ViewCountInfo vcInfo ={
        "A",
        1
    };

    Statement insert(*session);
    insert << "INSERT INTO view_count_info VALUES(?, ?)",
            use(vcInfo.username),
            use(vcInfo.counter);
    insert.execute();

    vcInfo.username = "B";
    vcInfo.counter = 5;
    insert.execute();

    vcInfo.username = "C";
    vcInfo.counter = 20;
    insert.execute();

    vcInfo.username = "D";
    vcInfo.counter = 50;
    insert.execute();
}

void initDb() {
    // register MySQL connector
    Poco::Data::MySQL::Connector::registerConnector();
    // create a session
    std::string sessionStr = "host=";
    sessionStr += mySQLHost;
    sessionStr += ";port=";
    sessionStr += Utilities::convertToString(mySQLPort);
    sessionStr += ";db=";
    sessionStr += mySQLDb;
    sessionStr += ";user=";
    sessionStr += mySQLUsername;
    sessionStr += ";password=";
    sessionStr += mySQLPassword;
    sessionStr += ";compress=true;auto-reconnect=true";
    mySQLsession = new Session("MySQL", sessionStr);
    // drop sample table, if it exists
    *mySQLsession << "DROP TABLE IF EXISTS view_count_info", now;
    // (re)create table
    *mySQLsession << "CREATE TABLE view_count_info (username VARCHAR(30), counter INTEGER(5))", now;

    insertTestData(mySQLsession);
}

void initCaching(){
    if (isApplyCache){
        cout << "Using cache" << endl;
    }
    else cout << "Not using cache" << endl;
    
    //connect server
//    memcached_st *memc;
//    memcached_return rc;
//    memcached_server_st *server;
//    time_t expiration;
//    uint32_t flags;
//
//    memc = memcached_create(NULL);
//    server = memcached_server_list_append(NULL, "localhost", 11211, &rc);
//    rc = memcached_server_push(memc, server);
//    memcached_server_list_free(server);
//
//    string key = "username";
//    string value = "hehe";
//    size_t value_length = value.length();
//    size_t key_length = key.length();
//
//    //Save data
//    rc = memcached_set(memc, key.c_str(), key.length(), value.c_str(), value.length(), expiration, flags);
//    if (rc == MEMCACHED_SUCCESS) {
//        cout << "Save data:" << value << " sucessful!" << endl;
//    }
//
//    //Get data
//    char* result = memcached_get(memc, key.c_str(), key_length, &value_length, &flags, &rc);
//    if (rc == MEMCACHED_SUCCESS) {
//        cout << "Get value:" << result << " sucessful!" << endl;
//    }
//
//    //Delete data
//    rc = memcached_delete(memc, key.c_str(), key_length, expiration);
//    if (rc == MEMCACHED_SUCCESS) {
//        cout << "Delete key:" << key << " sucessful!" << endl;
//    }
//
//    //free
//    memcached_free(memc);
}

int main(int argc, char **argv) {
    // print out viewcount.properties file information
    getPropertiesInfo();
    // init POCO's logger settings
    initLogger();
    // initialize the system database
    initDb();
    // initialize the caching database
    initCaching();

    // start the server
    boost::shared_ptr<APIsHandler> handler(new APIsHandler());
    boost::shared_ptr<TProcessor> processor(new APIsProcessor(handler));
    boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
    
    //TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
    
    // TThreadPoolServer
    const int workerCount = 4;
    boost::shared_ptr<ThreadManager> threadManager =
            ThreadManager::newSimpleThreadManager(workerCount);
    boost::shared_ptr<PosixThreadFactory> threadFactory =
            boost::shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
    threadManager->threadFactory(threadFactory);
    threadManager->start();
    TThreadPoolServer server(processor,
            serverTransport,
            transportFactory,
            protocolFactory,
            threadManager);

    Logger::root().information("> Server is running");

    server.serve();

    return 0;
}