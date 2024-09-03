#include "AWNet.h"
#include <string.h>
#include <iostream>
#include <netinet/tcp.h>
#include <event2/listener.h>
#include <event2/buffer.h>

/*
Any one who want to upgrade the networking library(libEvent), please change the size of evbuffer showed below:
*MODIFY--libevent/buffer.c
#define EVBUFFER_MAX_READ	4096
TO
#define EVBUFFER_MAX_READ	65536
*/

//1048576 = 1024 * 1024
#define KW_BUFFER_MAX_READ 1048576

void AWNet::log_cb(int severity, const char* msg)
{
    std::cout << "[" << severity << "] " << msg;
}

void AWNet::event_fatal_cb(int err)
{
    //LOG(FATAL) << "event_fatal_cb " << err;
}

void AWNet::conn_eventcb(struct bufferevent* bev, int16_t events, void* ctx)
{
    ConnObject* pObject = reinterpret_cast<ConnObject*>(ctx);
    if (!pObject)
    {
        std::cerr << "conn_eventcb: pObject is null" << std::endl;
        return;
    }

    AWNet* pNet = reinterpret_cast<AWNet*>(pObject->GetNet());
    if (!pNet)
    {
        std::cerr << "conn_eventcb: pNet is null" << std::endl;
        return;
    }

    if (events & BEV_EVENT_CONNECTED)
    {
        //must to set it's state before the "EventCB" functional be called[maybe user will send msg in the callback function]
        pNet->mWorking = true;
    }
    else
    {
        if (!pNet->mbServer)
            pNet->mWorking = false;
    }

    // if (pNet->mEventCB)
    //     pNet->mEventCB(pObject->GetFD(), KW_NET_EVENT(events), pNet);

    // if (events & BEV_EVENT_CONNECTED)
    // {
    //     struct evbuffer* input = bufferevent_get_input(bev);
    //     struct evbuffer* output = bufferevent_get_output(bev);
    //     if (pNet->mnBufferSize > 0)
    //     {
    //         evbuffer_expand(input, pNet->mnBufferSize);
    //         evbuffer_expand(output, pNet->mnBufferSize);
    //     }
    // }
    // else
    // {
    //     pNet->CloseNetObject(pObject->GetFD());
    // }
}

void AWNet::listener_cb(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* sa, int socklen, void* ctx)
{
    AWNet* pNet = reinterpret_cast<AWNet*>(ctx);
    if (!pNet || pNet->CloseConnObject(fd))
        return;

    // if (pNet->GetConnObjectCount() >= pNet->mnMaxConnect)
    //     return;

    struct event_base* base = evconnlistener_get_base(listener);
    struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev)
    {
        std::cerr << "Error constructing bufferevent!" << std::endl;
        return;
    }

    ConnObject* pObject = new ConnObject(pNet, fd, *sa, bev);
    pNet->AddConnObject(fd, pObject);

    int optval = 1;
    int result = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
    //setsockopt(fd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(optval));
    if (result < 0)
    {
        std::cout << "setsockopt TCP_NODELAY ERROR!" << std::endl;
    }

    int recvBufLen = KW_BUFFER_MAX_READ;
    result = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&recvBufLen, sizeof(recvBufLen));
    if (result < 0)
    {
        std::cout << "setsockopt SO_RCVBUF ERROR!" << std::endl;
    }

    bufferevent_setcb(bev, conn_readcb, nullptr, conn_eventcb, reinterpret_cast<void*>(pObject));

    bufferevent_enable(bev, EV_READ | EV_WRITE | EV_CLOSED | EV_TIMEOUT | EV_PERSIST);

    event_set_fatal_callback(event_fatal_cb);

    conn_eventcb(bev, BEV_EVENT_CONNECTED, reinterpret_cast<void*>(pObject));

    bufferevent_set_max_single_read(bev, KW_BUFFER_MAX_READ);
    bufferevent_set_max_single_write(bev, KW_BUFFER_MAX_READ);
}

void AWNet::conn_readcb(struct bufferevent* bev, void* ctx)
{
    ConnObject* pObject = reinterpret_cast<ConnObject*>(ctx);
    if (!pObject)
        return;

    AWNet* pNet = reinterpret_cast<AWNet*>(pObject->GetNet());
    if (!pNet)
        return;

    if (pObject->GetState() == EConneObjectState::WaitRemove)
        return;

    struct evbuffer* input = bufferevent_get_input(bev);
    if (!input)
        return;

    size_t len = evbuffer_get_length(input);
    unsigned char* data = evbuffer_pullup(input, len);
    pObject->AddTail((const char*)data, len);
    evbuffer_drain(input, len);

    while (pNet->ExtractPackage(pObject));
}

bool AWNet::Execute()
{
    ExecuteClose();

    if (mBase)
        event_base_loop(mBase, EVLOOP_ONCE | EVLOOP_NONBLOCK);

    return true;
}

void AWNet::Initialization(const char* ip, const uint16_t nPort)
{
    mIP = ip;
    mPort = nPort;

    InitClientNet();
}

int AWNet::Initialization(const uint32_t nMaxClient, const uint16_t nPort, const int nCpuCount)
{
    mPort = nPort;

    return InitServerNet();
}

bool AWNet::Final()
{
    CloseSocketAll();

    if (mListener)
    {
        evconnlistener_free(mListener);
        mListener = nullptr;
    }

    if (mBase)
    {
        event_base_free(mBase);
        mBase = nullptr;
    }

    return true;
}

bool AWNet::SendMsg(const std::string& msg, const AWSOCK sockIndex)
{
    if (!mWorking || msg.empty())
        return false;

    auto it = mConnObjects.find(sockIndex);
    if (it != mConnObjects.end())
    {
        ConnObject* pNetObject = reinterpret_cast<ConnObject*>(it->second);
        if (pNetObject)
        {
            bufferevent* bev = reinterpret_cast<bufferevent*>(pNetObject->GetBev());
            if (nullptr != bev)
            {
                bufferevent_write(bev, msg.data(), msg.length());
                return true;
            }
        }
    }

    return false;
}

bool AWNet::CloseConnObject(const AWSOCK sockIndex)
{
    auto it = mConnObjects.find(sockIndex);
    if (it != mConnObjects.end())
    {
        ConnObject* pObject = it->second;

        pObject->SetState(EConneObjectState::WaitRemove);
        mWaitRemoveSocks.push_back(sockIndex);

        return true;
    }

    return false;
}

bool AWNet::ExtractPackage(ConnObject* pObject)
{
    bool continueExtract = false;

    int len = pObject->GetBufferLen();
    if (len > 0)
    {
        std::cout << std::string(pObject->GetBuffer(), len) << std::endl;

        pObject->RemoveHead(len);

        continueExtract = false;
    }

    return continueExtract;
}

bool AWNet::AddConnObject(const AWSOCK sockIndex, ConnObject* pObject)
{
    //lock
    return mConnObjects.insert(std::map<AWSOCK, ConnObject *>::value_type(sockIndex, pObject)).second;
}

int AWNet::InitClientNet()
{
    struct sockaddr_in addr;
    struct bufferevent* bev = nullptr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(mPort);

    if (evutil_inet_pton(AF_INET, mIP.c_str(), &addr.sin_addr) <= 0)
    {
        std::cerr << "inet_pton error" << std::endl;
        return -1;
    }

    mBase = event_base_new();
    if (mBase == nullptr)
    {
        std::cerr << "event_base_new error" << std::endl;
        return -1;
    }

    bev = bufferevent_socket_new(mBase, -1, BEV_OPT_CLOSE_ON_FREE);
    if (bev == nullptr)
    {
        std::cerr << "bufferevent_socket_new error" << std::endl;
        return -1;
    }

    if (0 != bufferevent_socket_connect(bev, (struct sockaddr *)&addr, sizeof(addr)))
    {
        std::cerr << "bufferevent_socket_new error" << std::endl;
        return -1;
    }

    AWSOCK sockfd = bufferevent_getfd(bev);
    ConnObject* pObject = new ConnObject(this, sockfd, *(struct sockaddr *)&addr, bev);
    if (!AddConnObject(0, pObject))
    {
        std::cerr << "AddConnObject error" << std::endl;
        return -1;
    }

    mbServer = false;

    int optval = 1;
    //setsockopt(fd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(optval));
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) < 0)
    {
        std::cout << "setsockopt TCP_NODELAY ERROR !!!" << std::endl;
        return -1;
    }

    int nRecvBufLen = KW_BUFFER_MAX_READ;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBufLen, sizeof(int));

    bufferevent_setcb(bev, conn_readcb, nullptr, conn_eventcb, reinterpret_cast<void*>(pObject));
    bufferevent_enable(bev, EV_READ | EV_WRITE | EV_CLOSED | EV_TIMEOUT | EV_PERSIST);

    event_set_log_callback(&AWNet::log_cb);

    bufferevent_set_max_single_read(bev, KW_BUFFER_MAX_READ);
    bufferevent_set_max_single_write(bev, KW_BUFFER_MAX_READ);

    std::cout << "Want to connect " << mIP << " SizeRead: " << bufferevent_get_max_to_read(bev)
        << " SizeWrite: " << bufferevent_get_max_to_write(bev) << std::endl;

    return sockfd;
}

int AWNet::InitServerNet()
{
    struct event_config* cfg = event_config_new();
    if (cfg)
    {
        //event_config_avoid_method(cfg, "epoll");
        if (event_config_set_flag(cfg, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST) < 0)
            return -1;

        mBase = event_base_new_with_config(cfg); //event_base_new()

        event_config_free(cfg);
    }

    if (!mBase)
    {
        std::cerr << "Could not initialize libevent!" << std::endl;
        Final();

        return -1;
    }

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(mPort);

    mListener = evconnlistener_new_bind(mBase, listener_cb, reinterpret_cast<void*>(this),
                                       LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
                                       (struct sockaddr *)&sin,
                                       sizeof(sin));

    if (!mListener)
    {
        std::cerr << "Could not create a listener!" << std::endl;
        Final();

        return -1;
    }

    event_set_log_callback(&AWNet::log_cb);

    return 0;
}

bool AWNet::CloseSocketAll()
{
    for (const auto& [sockIndex, _] : mConnObjects)
    {
        mWaitRemoveSocks.push_back(sockIndex);
    }

    ExecuteClose();

    mConnObjects.clear();

    return true;
}

int AWNet::GetConnObjectCount()
{
    return mConnObjects.size();
}

ConnObject* AWNet::GetNetObject(const AWSOCK sockIndex)
{
    auto it = mConnObjects.find(sockIndex);
    if (it != mConnObjects.end())
        return it->second;

    return nullptr;
}

void AWNet::CloseObject(const AWSOCK sockIndex)
{
    auto it = mConnObjects.find(sockIndex);
    if (it != mConnObjects.end())
    {
        ConnObject* pObject = it->second;

        struct bufferevent* bev = reinterpret_cast<bufferevent*>(pObject->GetBev());

        bufferevent_free(bev);

        mConnObjects.erase(it);

        delete pObject;
        pObject = nullptr;
    }
}

void AWNet::ExecuteClose()
{
    for (const auto sockIndex : mWaitRemoveSocks)
    {
        CloseObject(sockIndex);
    }

    mWaitRemoveSocks.clear();
}

bool AWNet::IsServer()
{
    return mbServer;
}