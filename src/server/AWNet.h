#ifndef AW_NET_H
#define AW_NET_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <event2/util.h>        // evutil_socket_t
#include <arpa/inet.h>          // inet_ntop
#include <event2/event.h>       // event_base
#include <event2/bufferevent.h> // bufferevent


// #include <event2/thread.h>
// #include <event2/event_compat.h>

using AWSOCK = int64_t;
class AWNet;

enum class EConneObjectState
{
    None,
    Entering,
    Entered,
    WaitRemove,
};

class ConnObject
{
public:
    ConnObject(AWNet* pNet, AWSOCK fd, sockaddr& addr, void* pBev)
    {
        mFD = fd;
        mNet = pNet;
        mCtx = pBev;
        mSockAddr = addr;
        mState = EConneObjectState::None;

        char* s = nullptr;
        switch (mSockAddr.sa_family)
        {
        case AF_INET: //IPv4
        {
            struct sockaddr_in* addr_in = (struct sockaddr_in*)&mSockAddr;
            s = reinterpret_cast<char*>(malloc(INET_ADDRSTRLEN));
            inet_ntop(AF_INET, &addr_in->sin_addr, s, INET_ADDRSTRLEN);
            break;
        }
        case AF_INET6: //IPv6
        {
            struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)&mSockAddr;
            s = reinterpret_cast<char*>(malloc(INET6_ADDRSTRLEN));
            inet_ntop(AF_INET6, &addr_in6->sin6_addr, s, INET6_ADDRSTRLEN);
            break;
        }
        default:
            break;
        }
        mIp = s;
        free(s);
    }

    virtual ~ConnObject() { }

    AWNet* GetNet() { return mNet; }
    AWSOCK GetFD() { return mFD; }
    void* GetBev() { return mCtx; }

    EConneObjectState GetState() { return mState; }
    void SetState(EConneObjectState state) { mState = state; }

    const char* GetBuffer() const { return mBuffer.data(); }
    int GetBufferLen() const { return static_cast<int>(mBuffer.length()); }

    int AddTail(const char* str, size_t len)
    {
        mBuffer.append(str, len);

        return static_cast<int>(mBuffer.length());
    }

    int RemoveHead(uint32_t len)
    {
        if (len > mBuffer.length())
            return 0;

        mBuffer.erase(0, len);

        return static_cast<int>(mBuffer.length());
    }

private:
    void* mCtx;
    AWNet* mNet;
    AWSOCK mFD;
    sockaddr mSockAddr;
    std::string mIp;
    std::string mBuffer;
    EConneObjectState mState;
};

class AWNet
{
public:
    AWNet()
    {
        mBase = nullptr;
        mListener = nullptr;

        mIP = "";
        mPort = 0;

        mWorking = false;
        mbServer = false;
    }

    virtual ~AWNet() {}

public:
    virtual bool Execute();
    virtual bool Final();

    virtual void Initialization(const char* ip, const uint16_t nPort);
    virtual int Initialization(const uint32_t nMaxClient, const uint16_t nPort, const int nCpuCount = 4);

    virtual bool SendMsg(const std::string& msg, const AWSOCK sockIndex);

    virtual int GetConnObjectCount();
    virtual bool AddConnObject(const AWSOCK sockIndex, ConnObject* pObject);
    virtual bool CloseConnObject(const AWSOCK sockIndex);
    virtual ConnObject* GetNetObject(const AWSOCK sockIndex);

    virtual bool IsServer();

private:
    void ExecuteClose();
    bool CloseSocketAll();
    bool ExtractPackage(ConnObject* pObject);

    int InitClientNet();
    int InitServerNet();
    void CloseObject(const AWSOCK sockIndex);

    static void listener_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* sa, int socklen, void* ctx);
    static void conn_readcb(struct bufferevent* bev, void* ctx);
    static void conn_eventcb(struct bufferevent* bev, int16_t events, void* ctx);
    static void log_cb(int severity, const char* msg);
    static void event_fatal_cb(int err);

private:
    //<sockindex/fd,object>
    //Use share pointer replace C-style pointer
    std::map<AWSOCK, ConnObject*> mConnObjects;
    std::vector<AWSOCK> mWaitRemoveSocks;

    std::string mIP;
    int mPort;

    struct event_base* mBase;
    struct evconnlistener* mListener;
    bool mWorking;
    bool mbServer;
};
#endif
