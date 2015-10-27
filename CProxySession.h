#ifndef CGAMEPROXYSESSION_H
#define CGAMEPROXYSESSION_H

#include <map>
#include "CTcpSession.h"
#include "CListenSocket.h"

/**
 * @brief The CProxySession class
 * 网关会话
 * 一个网关会话代表一台服务器的网关
 */
class CProxySession
{
public:
    explicit CProxySession( );
    ~CProxySession();

    bool start();
    void set_session( const char *ser_addr,uint32 ser_port,uint32 listen_port );
    void add_new_connection( uint32 fd,struct sockaddr_in &sock_addr );

private:
    CListenSocket *m_listen_socket;

    const char *m_ser_addr;    /**< 注意为指针，所指向位置需要一直存在 */
    uint32 m_ser_port;
    uint32 m_listen_port;

    std::map<uint32,CTcpSession *> m_tcp_sessions; /**< 本次会话所有tcp连接 */

    friend class CTcpSession;
};

#endif // CGAMEPROXYSESSION_H
