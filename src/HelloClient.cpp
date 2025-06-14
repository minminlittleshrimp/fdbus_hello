#define FDB_LOG_TAG "HELLO_CLIENT"

#include <iostream>
#include <thread>
#include <fdbus/fdbus.h>
#include <fdbus/CFdbProtoMsgBuilder.h>
#include <fdbus/cJSON/cJSON.h>
#include <fdbus/CFdbCJsonMsgBuilder.h>
#include "HelloWorld.pb.h"

using namespace std;
using namespace ipc::fdbus;
static CBaseWorker main_worker;
enum MethodId
{
    METHOD_ID = 1,
    LOG_METHOD_ID = 2
};

class HelloClient : public CBaseClient
{
public:
    HelloClient(const char *name, CBaseWorker* work = 0)
        : CBaseClient(name, work)
    {
        // Empty Constructor
    }

    void callServwerSync()
    {
        HelloWorld hlwd;
        hlwd.set_name("Venom");
        CFdbProtoMsgBuilder builder(hlwd);
        CBaseJob::Ptr ref(new CBaseMessage(1));
        invoke(ref, builder);

        auto msg = castToMessage<CBaseMessage *>(ref);
        if (msg->isStatus())
        {
            /* Something wrong!*/
            return;
        }

        HelloWorld server;
        CFdbProtoMsgParser parser(server);
        bool result = msg->deserialize(parser);
        if (result)
        {
            cout << "My name: " << endl;
            cout << server.name() << endl;
        }
    }

    void callServwerAsync()
    {
        HelloWorld hlwd;
        hlwd.set_name("Venom");
        CFdbProtoMsgBuilder builder(hlwd);
        invoke(METHOD_ID, builder);
    }

protected:
    void onOnline(const CFdbOnlineInfo &info) override
    {
        cout << "Connected to the Server" << endl;

        // Subscribe to LOG_METHOD_ID broadcasts
        CFdbMsgSubscribeList subscribe_list;
        addNotifyItem(subscribe_list, LOG_METHOD_ID);
        subscribe(subscribe_list);

        callServwerAsync();
    }

    void onOffline(const CFdbOnlineInfo &info)
    {
        cout << "Disconnected from the Server" << endl;
    }

    void onReply(CBaseJob::Ptr &msg_ref)
    {
        auto msg = castToMessage<CFdbMessage *>(msg_ref);
        switch (msg->code())
        {
            case METHOD_ID:
            {
                if (msg->isStatus())
                {
                    cout << "Error: " << endl;
                    if (msg->isError())
                    {
                        return;
                    }
                    return;
                }

                HelloWorld server;
                CFdbProtoMsgParser parser(server);
                if (msg->deserialize(parser))
                {
                    cout << "Server name: " << server.name() << endl;
                }
                callServwerSync();
            }
            break;
            case LOG_METHOD_ID:
            {
                LogData log_data;
                CFdbProtoMsgParser parser(log_data);
                if (msg->deserialize(parser))
                {
                    std::cout << "Log from Server: " << log_data.log() << std::endl;
                }
            }
            break;
            default:
                break;
        }
    }

    void onBroadcast(CBaseJob::Ptr &msg_ref) override
    {
        auto msg = castToMessage<CFdbMessage *>(msg_ref);
        switch (msg->code())
        {
            case LOG_METHOD_ID:
            {
                LogData log_data;
                CFdbProtoMsgParser parser(log_data);
                if (msg->deserialize(parser))
                {
                    std::cout << "Log from Server (broadcast): " << log_data.log() << std::endl;
                }
                else
                {
                    std::cout << "Failed to parse log broadcast!" << std::endl;
                }
                break;
            }
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
        string client_name = argv[i];
        string url(FDB_URL_SVC);
        url += client_name;
        client_name += "_client";
        auto client = new HelloClient(client_name.c_str(), worker_ptr);

        client->enableReconnect(true);
        client->enableUDP(true);
        client->enableTimeStamp(true);
        client->connect(url.c_str());
    }

    // WorkThread
    CBaseWorker background_worker;
    background_worker.start(FDB_WORKER_EXE_IN_PLACE);
}