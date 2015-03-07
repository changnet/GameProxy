#include "CTcpSession.h"
#include "CNetGateSession.h"
#include "gssocket.h"
#include "gslog.h"
#include "CPool.h"

#include <unistd.h>    /* for close(int fd) */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/**
 * @brief CTcpSession::CTcpSession
 * @param netgate
 * 网关tcp会话初始化
 */
CTcpSession::CTcpSession(CNetGateSession *netgate)
{
    assert( netgate != null );

    reset();

    m_netgate = netgate;
}

/**
 * @brief CTcpSession::set_user_fd
 * @param user_fd
 * 设置玩家tcp连接文件描述符
 */
void CTcpSession::set_user_fd(int32 user_fd)
{
    m_user_tcp.fd = user_fd;
    m_user_tcp.open = true;
}

/**
 * @brief CTcpSession::set_netgate_session
 * @param netgate
 * 设置所属网关会话
 */
void CTcpSession::set_netgate_session(CNetGateSession *netgate)
{
    m_netgate = netgate;
}

/**
 * @brief CTcpSession::reset
 * 重置会话的各个参数
 */
void CTcpSession::reset()
{
    m_netgate = null;

    m_user_tcp.fd = 0;
    m_user_tcp.position = 0;
    m_user_tcp.length = 0;
    m_user_tcp.open = false;

    m_server_tcp.fd = 0;
    m_server_tcp.position = 0;
    m_server_tcp.length = 0;
    m_server_tcp.open = false;

    m_connecting_server = false;
}

/**
 * @brief CTcpSession::session_close
 * 关闭会话连接
 */
void CTcpSession::session_close()
{
    stop_user_tcp();
    stop_server_tcp();

    if ( m_user_tcp.open )
    {
        close( m_user_tcp.fd );
        m_user_tcp.open =false;
    }

    if ( m_server_tcp.open )
    {
        close( m_server_tcp.fd );
        m_server_tcp.open =false;
    }

    CPool<CTcpSession>::instance()->push_object( this ); /*放入空闲池重用，此时不应该再操作此对象的数据 */
}

/**
 * @brief CTcpSession::start_user_tcp
 * 开始接受玩家数据
 */
void CTcpSession::start_user_tcp()
{
    m_user_watcher.set< CTcpSession,&CTcpSession::user_tcp_cb >( this );
    m_user_watcher.start( m_user_tcp.fd,ev::READ );
}

/**
 * @brief CTcpSession::stop_user_tcp
 * 关闭玩家tcp连接事件监听
 */
void CTcpSession::stop_user_tcp()
{
    if ( m_user_watcher.is_active() )
        m_user_watcher.stop();
}

/**
 * @brief CTcpSession::start_server_tcp
 * 监听服务端tcp读写事件
 */
void CTcpSession::start_server_tcp()
{
    m_server_watcher.set< CTcpSession,&CTcpSession::server_tcp_cb >( this );
    m_server_watcher.start( m_server_tcp.fd,ev::READ );
}

/**
 * @brief CTcpSession::stop_server_tcp
 * 关闭服务器tcp连接事件监听
 */
void CTcpSession::stop_server_tcp()
{
    if ( m_server_watcher.is_active() )
        m_server_watcher.stop();
}

/**
 * @brief CTcpSession::user_tcp_cb
 * @param w
 * @param revents
 * 玩家tcp读写事件回调
 */
void CTcpSession::user_tcp_cb(ev::io &w, int32 revents)
{
    if ( EV_ERROR & revents )
    {
        w.stop();

        session_close();

        return;
    }

    if ( ev::READ & revents )
    {
        read_user_data();

        /* 当有数据第一次从玩家到达时才连接至服务器 */
        if ( !m_server_tcp.open && !m_connecting_server )
            connect_to_server();
        else
        {
            write_server_data(); /* 尝试将读到的数据写入，防止驻留缓冲区 */
        }
    }

    if ( ev::WRITE & revents )
    {
        write_user_data();
    }
}

/**
 * @brief CTcpSession::read_user_data
 * 读取玩家数据，数据存入server端tcp连接的缓冲区
 */
void CTcpSession::read_user_data()
{
    move_buff( &m_server_tcp );

    if ( MAX_TCP_BUFF - m_server_tcp.length < MIN_TCP_BUFF  ) /* 缓冲区已满，断开连接 */
    {
        session_close();
        return;
    }

    int32 ret = read( m_user_tcp.fd,m_server_tcp.buff+m_server_tcp.position,
                      MAX_TCP_BUFF - m_server_tcp.length );

    /* 一次应该能读完,所设置BUFF大于socket默认缓冲区.如果未读完或其他原因，则libev会再次触发 */

    if ( ret < 0 )  //error
    {
        if ( EAGAIN == errno || EWOULDBLOCK == errno )  //不处理，等待下次触发再重新读取数据
        {
            return;
        }

        session_close();
    }
    else if ( 0 == ret ) //客户端关闭
    {
        session_close();
    }
    else    //read data
    {
        m_server_tcp.length += ret;
    }
}

/**
 * @brief CTcpSession::write_user_data
 * 写入数据到玩家tcp连接
 */
void CTcpSession::write_user_data()
{
    if ( !m_user_tcp.open || m_user_tcp.length <= 0 )  /* 无数据可以发送 */
        return;

    int32 ret = write( m_user_tcp.fd,m_user_tcp.buff+m_user_tcp.position,m_user_tcp.length - m_user_tcp.position );
    if ( ret < 0 )
    {
        if ( EAGAIN == errno || EWOULDBLOCK == errno )
            return;

        session_close();
        return;
    }
    else if ( 0 == ret )
    {
        session_close();
        return;
    }
    else
    {
        m_user_tcp.position += ret;
        if ( m_user_tcp.position >= m_user_tcp.length )
            m_user_tcp.zero_buff();
    }

    /**< 一直监听写事件是很耗cpu的，而更改watcher的监听事件libev会停止watcher再启动，消耗也不小 */
    if ( m_user_tcp.length > 0 && !(ev::WRITE & m_user_watcher.events) )
    {
        m_user_watcher.set( ev::READ | ev::WRITE );
    }
    else if ( m_user_tcp.length <= 0 && ev::WRITE & m_user_watcher.events )
    {
        m_user_watcher.set( ev::READ );
    }
}

/**
 * @brief CTcpSession::move_buff
 * @param ptcp
 * 当tcp不能一次发送过多的数据，此时会导致position增加
 * 如果此时又接收到更多的数据，导致length增加
 * 在极端情况下，会导致缓冲区前段空闲，而此时需要写入的
 * 数据大于后段空闲区，此时需要移动数据段到最前
 * 复制数据很耗效率，故缓冲区较大，基本不会发生
 */
void CTcpSession::move_buff(STcp *ptcp)
{
    /* 前端空闲区足够大，后端空闲区很小才移动 */
    if ( MIN_TCP_BUFF < ptcp->position && MIN_TCP_BUFF > MAX_TCP_BUFF - ptcp->length )
    {
        char tmp_buff[MAX_TCP_BUFF];
        uint32 length = ptcp->length - ptcp->position;
        memcpy( tmp_buff,ptcp->buff+ptcp->position,length );

        memcpy( ptcp->buff,tmp_buff,length);
        ptcp->position = 0;
        ptcp->length   = length;
    }
}

/**
 * @brief CTcpSession::server_tcp_cb
 * @param w
 * @param revents
 * 网关到服务器tcp连接读写事件回调
 */
void CTcpSession::server_tcp_cb(ev::io &w, int32 revents)
{
    if ( EV_ERROR & revents )
    {
        w.stop();

        session_close();

        return;
    }

    if ( ev::READ & revents )
    {
        read_server_data();
        write_user_data(); /* 尝试将读到的数据写入，防止驻留缓冲区 */
    }

    if ( ev::WRITE & revents )
    {
        write_server_data();
    }
}

/**
 * @brief CTcpSession::connect__cb
 * @param w
 * @param revents
 * 连接读写事件回调
 */
void CTcpSession::connect__cb(ev::io &w, int32 revents)
{
    if ( EV_ERROR & revents )
    {
        w.stop();

        session_close();

        return;
    }

    /*
    通过可读和可写来判断连接是否成功，处理三种情况：

    (1) 如果连接建立好了，对方没有数据到达，那么 sockfd 是可写的

    (2) 如果在 select 之前，连接就建立好了，而且对方的数据已到达，那么 sockfd 是可读和可写的。

    (3) 如果连接发生错误，sockfd 也是可读和可写的。

    判断 connect 是否成功，就得区别 (2) 和 (3)，这两种情况下 sockfd 都是

    可读和可写的，区分的方法是，调用 getsockopt 检查是否出错。

    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len)

    在 sockfd 都是可读和可写的情况下，我们使用 getsockopt 来检查连接

    是否出错。但这里有一个可移植性的问题。如果发生错误，getsockopt 源自 Berkeley 的实现将在变量 error 中

    返回错误，getsockopt 本身返回0；然而 Solaris 却让 getsockopt 返回 -1，并把错误保存在 errno 变量中。

    所以在判断是否有错误的时候，要处理这两种情况。
    */
    if ( m_server_tcp.open ) /* 表示本次回调是connect结果 */
    {
        ERROR( "illega connect_cb call" );
        session_close();
        return;
    }


    int32 _error = 0;
    socklen_t len = sizeof(_error);

    /* < 0表示getsockopt函数出错，_error不为0表示socket出错,得到的_error其实就是errno */
    if ( getsockopt( m_server_tcp.fd,SOL_SOCKET,SO_ERROR,&_error,&len ) < 0 || _error )
    {
        session_close();
        return;
    }

    m_server_tcp.open = true;
    w.set< CTcpSession,&CTcpSession::server_tcp_cb > ( this );

    server_tcp_cb( w,revents );/* 这里不要返回，需要把第一次收到的数据写入 */
}

/**
 * @brief CTcpSession::read_server_data
 * 读取来自服务器的数据并写入玩家缓冲区
 */
void CTcpSession::read_server_data()
{
    move_buff( &m_user_tcp );

    if ( MAX_TCP_BUFF - m_user_tcp.length < MIN_TCP_BUFF  ) /* 缓冲区已满，断开连接 */
    {
        session_close();
        return;
    }

    int32 ret = read( m_server_tcp.fd,m_user_tcp.buff+m_user_tcp.position,
                      MAX_TCP_BUFF - m_user_tcp.length );

    /* 一次应该能读完,所设置BUFF大于socket默认缓冲区.如果未读完或其他原因，则libev会再次触发 */

    if ( ret < 0 )  //error
    {
        if ( EAGAIN == errno || EWOULDBLOCK == errno )  //不处理，等待下次触发再重新读取数据
        {
            return;
        }

        session_close();
    }
    else if ( 0 == ret ) //客户端关闭
    {
        session_close();
    }
    else    //read data
    {
        m_user_tcp.length += ret;
    }
}

/**
 * @brief CTcpSession::write_server_data
 * 写入数据到服务器
 */
void CTcpSession::write_server_data()
{
    if ( !m_server_tcp.open || m_server_tcp.length <= 0 )  /* 无数据可以发送 */
        return;

    int32 ret = write( m_server_tcp.fd,m_server_tcp.buff+m_server_tcp.position,m_server_tcp.length - m_server_tcp.position );
    if ( ret < 0 )
    {
        if ( EAGAIN == errno || EWOULDBLOCK == errno )
            return;

        session_close();
        return;
    }
    else if ( 0 == ret )
    {
        session_close();
        return;
    }
    else
    {
        m_server_tcp.position += ret;
        if ( m_server_tcp.position >= m_server_tcp.length ) /* 发送完成，清理缓冲区指针 */
            m_server_tcp.zero_buff();
    }

    /**< 一直监听写事件是很耗cpu的，而更改watcher的监听事件libev会停止watcher再启动，消耗也不小 */
    if ( m_server_tcp.length > 0 && !(ev::WRITE & m_server_watcher.events) )
    {
        m_server_watcher.set( ev::READ | ev::WRITE );
    }
    else if ( m_server_tcp.length <= 0 && ev::WRITE & m_server_watcher.events )
    {
        m_server_watcher.set( ev::READ );
    }
}

/**
 * @brief CTcpSession::connect_to_server
 * 网关连接到服务器
 */
void CTcpSession::connect_to_server()
{
    m_server_tcp.fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( m_server_tcp.fd < 0 )
    {
        session_close();
        return;
    }

    struct sockaddr_in server_addr;

    bzero( &server_addr, sizeof(server_addr) );
    server_addr.sin_family = AF_INET;
    inet_aton( m_netgate->m_ser_addr, &server_addr.sin_addr );
    server_addr.sin_port = htons( m_netgate->m_ser_port );

    if ( !init_tcp_connection( m_server_tcp.fd ) )
    {
        session_close();
        return;
    }

    /*
    实现非阻塞 connect ，首先把 sockfd 设置成非阻塞的。这样调用 connect 可以立刻返回，根据返回值和 errno 处理三种情况：

    (1) 如果返回 0，表示 connect 成功。

    (2) 如果返回值小于 0， errno 为 EINPROGRESS, 表示连接建立已经启动但是尚未完成。这是期望的结果，不是真正的错误。

    (3) 如果返回值小于0，errno 不是 EINPROGRESS，则连接出错了。
    */
    if ( connect( m_server_tcp.fd, (struct sockaddr*)&server_addr, sizeof(server_addr) ) < 0
         && EINPROGRESS != errno )
    {
        session_close();
        return;
    }

    /* 开始监听读写事件来判断是否连接成功 */
    m_server_watcher.set< CTcpSession,&CTcpSession::connect__cb >( this );
    m_server_watcher.start( m_server_tcp.fd,ev::READ |ev::WRITE );

    m_connecting_server = true;
}

/**
 * @brief CTcpSession::init_tcp_connection
 * @param fd
 * @return 成功true，失败false
 * 初始化tcp属性（非阻塞、keep-alive等）
 */
bool CTcpSession::init_tcp_connection(int32 fd)
{
    if ( set_socket_block_status( fd,false ) < 0 || open_socket_keepalive( fd ) < 0
         || set_socket_user_timeout( fd ) < 0 )
    {
        return false;
    }

    return true;
}
