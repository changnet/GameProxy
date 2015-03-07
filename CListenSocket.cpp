#include "CListenSocket.h"
#include "CNetGateSession.h"
#include "gssocket.h"

/**
 * @brief CListenSocket::CListenSocket
 * @param netgate
 * 初始化监听tcp类
 */
CListenSocket::CListenSocket(CNetGateSession *netgate)
{
    m_netgate = netgate;

    m_socket_fd = 0;
}

/**
 * @brief CListenSocket::start
 * @param port
 * @return 成功或失败
 * 开启监听
 */
bool CListenSocket::start( uint32 port )
{
    m_port = port;
    if ( 0 == m_port )
    {
        return false;
    }

    struct sockaddr_in sk_socket;
    int32 optval = 1;


    m_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( -1 == m_socket_fd )
    {
        stop();

        return false;
    }

    /*
     * enable address reuse.it will help when the socket is in TIME_WAIT status.
     * for example:
     *     server crash down and the socket is still in TIME_WAIT status.if try
     * to restart server immediately,you need to reuse address.but note you may
     * receive the old data from last time.
     */
    if ( setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR,(char *) &optval, sizeof(optval)) == -1)
    {
        stop();

        return false;
    }

    memset( &sk_socket,0,sizeof(sk_socket) );
    sk_socket.sin_family = AF_INET;
    sk_socket.sin_addr.s_addr = INADDR_ANY;
    sk_socket.sin_port = htons( m_port );

    if ( bind( m_socket_fd, (struct sockaddr *) & sk_socket,sizeof(sk_socket)) == -1)
    {
        stop();

        return false;
    }

    if ( set_socket_block_status( m_socket_fd, true ) == -1 )
    {
        stop();

        return false;
    }

    if ( listen( m_socket_fd, PENDING ) == -1)
    {
        stop();

        return false;
    }

    m_accept_watcher.set<CListenSocket,&CListenSocket::accept_cb>(this);
    m_accept_watcher.start( m_socket_fd,ev::READ );

    return true;
}

/**
 * @brief CListenSocket::stop
 * 停止监听
 */
void CListenSocket::stop()
{
    if ( m_socket_fd <= 0 )
        return;

    m_accept_watcher.stop();

    close( m_socket_fd );
    m_socket_fd = 0;
}

/**
 * @brief CListenSocket::accept_cb
 * @param w
 * @param revents
 * 新用户接入回调
 */
void CListenSocket::accept_cb(ev::io &w, int32 revents)
{
    if ( EV_ERROR & revents )  //error
    {
        w.stop();

        return;
    }

    struct sockaddr_in socket_address;
    int32 length = sizeof( socket_address );

    //w.fd is m_plistenning_socket->get_listen_socket_fd()
    int32 fd = accept( w.fd,(sockaddr *)&socket_address,(socklen_t *)&length );
    if ( fd < 0 )
    {
        return;
    }

    m_netgate->add_new_connection( fd,socket_address );
}
