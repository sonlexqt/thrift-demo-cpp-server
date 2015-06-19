/*
 * INCLUDES
 */
// include thrift generated code
#include "APIs.h"
// include Thrift library
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
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
// include other libraries
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <ctime>

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
// using namespaces from POCO
using Poco::AutoPtr;
using Poco::Util::PropertyFileConfiguration;
using Poco::Logger;
using Poco::FileChannel;
using Poco::ConsoleChannel;
using Poco::SplitterChannel;
using Poco::PatternFormatter;
using Poco::FormattingChannel;
// using other namespaces
using namespace std;
using boost::shared_ptr;

/*
 * GLOBAL VARS
 */
int port = 0;

class APIsHandler : virtual public APIsIf {
public:

    APIsHandler() {
        // Your initialization goes here
    }

    void put(const std::string& _username, const int32_t _newValue) {
        Logger::root().information("put");
    }

    void increase(const std::string& _username) {
        Logger::root().information("increase");
    }

    int32_t get(const std::string& _username) {
        Logger::root().information("get");
    }

    bool ping() {
        Logger::root().information("ping");
        return true;
    }

};

void printPropertyFileInfo() {
    AutoPtr<PropertyFileConfiguration> pConf;
    pConf = new PropertyFileConfiguration("viewcount.properties");
    cout << "=== The following are information get from the viewcount.properties file: ===" << endl;
    
    int serverPort = pConf->getInt("SERVER_PORT");
    cout << "serverPort: " << serverPort << endl;
    port = serverPort;
            
    int applyCache = pConf->getInt("APPLY_CACHE");
    cout << "applyCache: " << applyCache << endl;
    
    string authorName = pConf->getString("AUTHOR_NAME");
    cout << "authorName: " << authorName << endl;
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

int main(int argc, char **argv) {
    // print out viewcount.properties file information
    printPropertyFileInfo();
    // init POCO's logger settings
    initLogger();
    
    // start the server
    shared_ptr<APIsHandler> handler(new APIsHandler());
    shared_ptr<TProcessor> processor(new APIsProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
    TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

    Logger::root().information("> Server is running");

    server.serve();

    return 0;
}



