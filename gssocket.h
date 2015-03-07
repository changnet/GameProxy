#ifndef GSSOCKET_H
#define GSSOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>    //for keep-alive

#include "gstypes.h"

#define MAXUSER    6000   //max client
#define PENDING    256    //max pending accept client

//TCP keep-alive参数
#define KEEP_ALIVE_TIME        60    //无通信后60s开始发送
#define KEEP_ALIVE_INTERVAL    10    //间隔10s发送
#define KEEP_ALIVE_PROBES      5     //总共发送5次

#define SOCKET_USER_TIMEOUT    60000    //TCP_USER_TIMEOUT,milliseconds

/*
 * 最大文件描述符,应该是int32能表示的正数
 * 这里用/proc/sys/fs/file-max
 * 大小影响CNetBackend::m_tcp_sockets大小
 */
#define MAX_FD                 738339

//set socket block status
//inline ,to avoid multiple definition.or you can define funtion here,reference in .c file
inline int32 set_socket_block_status( int32 sock_fd,bool is_block )
{
    int32 flags = fcntl( sock_fd, F_GETFL, 0 ); //get old status

    if ( flags == -1 )
        return flags;

    if ( is_block )
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    return fcntl( sock_fd, F_SETFL, flags);
}

/*keepalive并不是TCP规范的一部分。在Host Requirements RFC罗列有不使用它的三个理由：
 * （1）在短暂的故障期间，它们可能引起一个良好连接（good connection）被释放（dropped），
 * （2）它们消费了不必要的宽带，
 * （3）在以数据包计费的互联网上它们（额外）花费金钱。
 *
 * 在程序中表现为,当tcp检测到对端socket不再可用时(不能发出探测包,或探测包没有收到ACK的响应包),
 * select会返回socket可读,并且在recv时返回-1,同时置上errno为ETIMEDOUT.
 *
 * 但是，tcp自己的keepalive有这样的一个bug：
 *    正常情况下，连接的另一端主动调用colse关闭连接，tcp会通知，我们知道了该连接已经关闭。但是如果tcp连接的另一端突然掉线，
 *或者重启断电，这个时候我们并不知道网络已经关闭。而此时，如果有发送数据失败，tcp会自动进行重传。重传包的优先级高于keepalive，
 *那就意味着，我们的keepalive总是不能发送出去。 而此时，我们也并不知道该连接已经出错而中断。在较长时间的重传失败之后，
 *我们才会知道。即我们在重传超时后才知道连接失败.
 */
inline int32 open_socket_keepalive( int32 sock_fd )
{
    int32 optval = 1;
    int32 optlen = sizeof(optval);
    int32 ret    = 0;

    ret = setsockopt( sock_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen ); //open keep alive
    if ( 0 > ret )
        return ret;

    int32 tcp_keepalive_time     = KEEP_ALIVE_TIME;
    int32 tcp_keepalive_interval = KEEP_ALIVE_INTERVAL;
    int32 tcp_keepalive_probes   = KEEP_ALIVE_PROBES;

    optlen = sizeof( tcp_keepalive_time );
    ret = setsockopt( sock_fd,SOL_TCP,TCP_KEEPIDLE,&tcp_keepalive_time,optlen );
    if ( 0 > ret )
        return ret;

    optlen = sizeof( tcp_keepalive_interval );
    ret = setsockopt( sock_fd,SOL_TCP,TCP_KEEPINTVL,&tcp_keepalive_interval,optlen );
    if ( 0 > ret )
        return ret;

    optlen = sizeof( tcp_keepalive_probes );
    ret = setsockopt( sock_fd,SOL_TCP,TCP_KEEPCNT,&tcp_keepalive_probes,optlen );

    return ret;
}

/* http://www.man7.org/linux/man-pages/man7/tcp.7.html
 * since linux 2.6.37
 * 网络中存在unack数据包时开始计时，超时仍未收到ack，关闭网络
 * 网络中无数据包，该计算器不会生效，此时由keep-alive负责
 * 如果keep-alive的数据包也无ack，则开始计时.这时keep-alive同时有效，
 * 谁先超时则谁先生效
 */
inline int32 set_socket_user_timeout( int32 fd )
{
#ifdef TCP_USER_TIMEOUT
    uint32 timeout = SOCKET_USER_TIMEOUT;

    return setsockopt( fd,IPPROTO_TCP,TCP_USER_TIMEOUT,&timeout,sizeof(timeout) );
#else
    (void *)fd;
    return 0;
#endif
}

#endif // GSSOCKET_H
