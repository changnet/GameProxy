#include "CNetGateSession.h"
#include "CPool.h"
#include "gslog.h"

/**
 * @brief CNetGateSession::CNetGateSession
 * 初始化一次网关会话
 */
CNetGateSession::CNetGateSession()
{
    m_listen_socket = null;

    m_ser_addr    = null;
    m_ser_port    = 0;
    m_listen_port = 0;
}

/**
 * @brief CNetGateSession::set_session
 * @param ser_addr
 * @param ser_port
 * @param listen_port
 * 设置网关各个参数
 */
void CNetGateSession::set_session(const char *ser_addr, uint32 ser_port,
                                  uint32 listen_port)
{
    m_listen_socket = null;

    m_ser_addr = ser_addr;
    m_ser_port = ser_port;
    m_listen_port = listen_port;
}

/**
 * @brief CNetGateSession::~CNetGateSession
 * 析构函数
 */
CNetGateSession::~CNetGateSession()
{
    if ( m_listen_socket )
    {
        delete m_listen_socket;
        m_listen_socket = null;
    }
}

/**
 * @brief CNetGateSession::start
 * @return
 * 开启网关会话
 */
bool CNetGateSession::start()
{
    assert( m_ser_port > 0 );
    assert( m_listen_port > 0 );
    assert( m_ser_addr != null );

    if ( null == m_listen_socket )
    {
        m_listen_socket = new CListenSocket( this );
    }

    return m_listen_socket->start( m_listen_port );
}

/**
 * @brief CNetGateSession::add_new_connection
 * @param fd
 * @param sock_addr
 * 处理新接入用户
 */
void CNetGateSession::add_new_connection(uint32 fd, sockaddr_in &sock_addr)
{
    (void)sock_addr;  /* avoid warning: unused parameter */

    CTcpSession *psession = CPool<CTcpSession>::instance()->pop_object();

    if ( !psession )
    {
        psession = new CTcpSession( this );
    }
    else
    {
        psession->reset();    /* 重用的需要清除旧数据 */
        psession->set_netgate_session( this );
    }

    psession->set_user_fd( fd );

    if ( !psession->init_tcp_connection(fd) )
    {
        ERROR( "init_tcp_connection fail" );
        psession->session_close();

        CPool<CTcpSession>::instance()->push_object( psession );

        return;
    }

    m_tcp_sessions[fd] = psession;
    psession->start_user_tcp();
}
