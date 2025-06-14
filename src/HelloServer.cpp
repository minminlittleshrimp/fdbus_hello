#define FDB_LOG_TAG "HELLO_SERVER"

#include <iostream>
#include <fdbus/fdbus.h>
#include <fdbus/CFdbProtoMsgBuilder.h>
#include <fdbus/cJSON/cJSON.h>
#include <fdbus/CFdbCJsonMsgBuilder.h>
#include "HelloWorld.pb.h"

using namespace std;
using namespace ipc::fdbus;
static CBaseWorker main_worker;
static const int METHOD_ID = 1;

class HelloServer : public CBaseServer
{
public:
    HelloServer(const char *name, CBaseWorker* work = 0)
        : CBaseServer(name, work)
    {
        // Empty Constructor
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
    void onInvoke(CBaseJob:: Ptr &msg_ref)
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
                HelloWorld server;
                server.set_name("Dr.Mint");
                CFdbProtoMsgBuilder builder(server);
                msg->reply(msg_ref, builder);
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
    }

    // WorkThread
    CBaseWorker background_worker;
    background_worker.start(FDB_WORKER_EXE_IN_PLACE);
}