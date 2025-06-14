#define FDB_LOG_TAG "HELLO_SERVER"

#include <iostream>
#include <fdbus/fdbus.h>
#include <fdbus/CFdbProtoMsgBuilder.h>
#include <fdbus/cJSON/cJSON.h>
#include <fdbus/CFdbCJsonMsgBuilder.h>
#include "HelloWorld.pb.h"
#include <thread>
#include <chrono>

using namespace std;
using namespace ipc::fdbus;
static CBaseWorker main_worker;

enum MethodId
{
    METHOD_ID = 1,
    LOG_METHOD_ID = 2
};

class HelloServer : public CBaseServer
{
public:
    HelloServer(const char *name, CBaseWorker* work = 0)
        : CBaseServer(name, work)
    {
        // Empty Constructor
    }

    // Periodically broadcast log messages
    void logLoop()
    {
        int count = 0;
        while (true)
        {
            LogData log_data;
            log_data.set_log("Periodic log message #" + std::to_string(count++));
            CFdbProtoMsgBuilder log_builder(log_data);
            this->broadcast(LOG_METHOD_ID, log_builder);
            std::this_thread::sleep_for(std::chrono::seconds(1)); // 1 second interval
        }
    }

protected:
    void onOnline(const CFdbOnlineInfo &info)
    {
        cout << "Connect to the Client" << endl;
    }
    void onOffline(const CFdbOnlineInfo &info)
    {
        cout << "Connect to the Client" << endl;
    }
    void onInvoke(CBaseJob::Ptr &msg_ref)
    {
        auto msg = castToMessage<CFdbMessage *>(msg_ref);
        static int32_t elapse_time = 0;
        switch (msg->code())
        {
            case METHOD_ID:
            {
                HelloWorld client;
                CFdbProtoMsgParser parser(client);
                if (!msg->deserialize(parser))
                {
                    return;
                }
                cout << "Client name " << client.name() << endl;

                // Reply with HelloWorld
                HelloWorld server;
                server.set_name("Dr.Mint");
                CFdbProtoMsgBuilder builder(server);
                msg->reply(msg_ref, builder);

                // Send a log message with LOG_METHOD_ID
                LogData log_data;
                log_data.set_log("Hello from server log!");
                CFdbProtoMsgBuilder log_builder(log_data);
                this->broadcast(LOG_METHOD_ID, log_builder);
            }
            break;
            default:
                break;
        }
    }
};

int main(int argc, char *argv[])
{
    /* Start FDBus context thread */
    FDB_CONTEXT->start();
    fdbLogAppendLineEnd(true);
    FDB_CONTEXT->registerNsWatchdogListener([](const tNsWatchdogList &dropped_list)
    {
        for (auto it = dropped_list.begin(); it != dropped_list.end(); ++it)
        {
            printf("Error!!! Endpoint drops - name: %s, pid: %d\n",
                   it->mClientName.c_str(), it->mPid);
        }
    });

    CBaseWorker *worker_ptr = &main_worker;
    /* Start worker thread */
    worker_ptr->start();

    /* Create servers and bind the address: svc://service_name */
    for (int i = 1; i < argc; ++i)
    {
        string server_name = argv[i];
        string url(FDB_URL_SVC);
        url += server_name;
        server_name += "_server";
        auto server = new HelloServer(server_name.c_str(), worker_ptr);
        server->enableWatchdog(true);
        server->enableUDP(true);
        server->setExportableLevel(FDB_EXPORTABLE_SITE);
        server->bind(url.c_str());

        std::thread([server](){ server->logLoop(); }).detach();
    }

    // WorkThread
    CBaseWorker background_worker;
    background_worker.start(FDB_WORKER_EXE_IN_PLACE);
}