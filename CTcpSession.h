#ifndef CTCPSESSION_H
#define CTCPSESSION_H

#include <ev++.h>
#include "gstypes.h"

#define MAX_TCP_BUFF 16*1024    /**< tcp缓冲区大小 */
#define MIN_TCP_BUFF 1024       /**< 预留最小缓冲区大小 */

class CProxySession;

/**
 * @brief The STcp struct
 * 一个tcp连接,为了方便缓冲区交互,不采用类
 */
struct STcp
{
    bool open;                 /**< 连接是否打开 */
    int32 fd;                /**< tcp文件描述符 */
    char buff[MAX_TCP_BUFF];   /**< 缓冲区 */
    int32 position;            /**< 缓冲区位置 */
    int32 length;              /**< 缓冲区数据长度 */

    void zero_buff()
    {
        position = 0;
        length = 0;
    }
};

/**
 * @brief The CTcpSession class
 * tcp会话,一次tcp会话包括一个网关与玩家tcp连接,一个网关与服务器tcp连接
 * 玩家tcp包含一个发送缓冲区,缓存发送给玩家的数据
 * 服务器tcp包含一个接收缓冲区,缓存从玩家收到的数据并往服务器发送
 */
class CTcpSession
{
public:
    explicit CTcpSession( CProxySession *proxy );
    void set_user_fd( int32 user_fd );
    void set_proxy_session( CProxySession *proxy );
    void reset();
    void session_close();

    void start_user_tcp();
    void stop_user_tcp();

    void start_server_tcp();
    void stop_server_tcp();

    static void move_buff( struct STcp *ptcp );
    static bool init_tcp_connection( int32 fd );

private:
    CProxySession *m_proxy;
    struct STcp m_user_tcp;
    struct STcp m_server_tcp;

    bool m_connecting_server;

    ev::io m_user_watcher;
    ev::io m_server_watcher;

    void user_tcp_cb( ev::io &w, int32 revents );
    void server_tcp_cb( ev::io &w, int32 revents );
    void connect__cb( ev::io &w, int32 revents );

    void read_user_data();
    void write_user_data();

    void read_server_data();
    void write_server_data();

    void connect_to_server();
};

#endif // CTCPSESSION_H
