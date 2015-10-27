#ifndef CLISTENSOCKET_H
#define CLISTENSOCKET_H

#include <ev++.h>
#include "gstypes.h"

class CProxySession; /* forward declaration */

/**
 * @brief The CListenSocket class
 * 网关会话tcp监听类
 */
class CListenSocket
{
public:
    explicit CListenSocket( CProxySession *proxy );

    bool start( uint32 port );
    void stop();
private:
    CProxySession *m_proxy;
    int32 m_socket_fd;
    uint32 m_port;

    ev::io m_accept_watcher;
    void accept_cb(ev::io &w, int32 revents);
};

#endif // CLISTENSOCKET_H
