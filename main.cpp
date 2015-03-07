#include <iostream>
#include <ev++.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <signal.h>

#include "CNetGateSession.h"
#include "CSetting.h"
#include "CBackend.h"
#include "CPool.h"
#include "gslog.h"

using namespace std;

void start_session(const char *file);
void write_config(const char *file);
void usage();
void thanks();
void become_daemon();
void sig_hold();
void sig_set(int32 signum, int32 sa_flag);
void sig_handler(int32 signum);

/**
 * @brief main
 * @return
 * 程序入口
 */
int main(int argc,char **argv)
{
    if ( argc < 3)
    {
        usage();
        return 0;
    }

    if ( 0 == strcmp( "-s",argv[1] ) )
    {
        thanks();
        become_daemon();
        start_session( argv[2] );
    }
    else if ( 0 == strcmp( "-c",argv[1] ) )
        write_config( argv[2] );
    else
        usage();

    /* 销毁单例内存 */
    CPool<CTcpSession>::uninstance();
    CBackend::uninstance();

    return 0;
}

/**
 * @brief start_session
 * @param file
 * 开启网关
 */
void start_session( const char *file )
{
    bool is_ok = true;
    CSetting setting;

    if ( !setting.read_config(file) )
    {
        std::cerr << "read config file fail:" << file << std::endl;
        return;
    }

    int32 session_count = setting.get_session_count();

    std::cout << "start with config file [" << file << "],"<< session_count
              << " session found:" << std::endl;

    CNetGateSession* p = new CNetGateSession[session_count];

    for ( int32 i = 0;i < session_count;i ++ )
    {
        int32 lp = setting.get_session_listen_port(i);
        int32 sp = setting.get_session_server_port(i);
        const char *url = setting.get_session_server_url(i);

        std::cout << "  " << "listen port:" << lp << ",server_url:" << url
                  << ",server port:" << sp << ".";

        (p+i)->set_session( url,sp,lp);

        if ( (p+i)->start() )
            std::cout << "OK" << std::endl;
        else
        {
            std::cout << "FAIL" << std::endl;
            is_ok = false;
        }
    }

    if ( is_ok )
    {
        std::cout << "done ..." << std::endl;

        sig_hold();/* 进入守护进程，监听必要信号 */

        CBackend::instance()->start();
    }
    else
        std::cout << "session start fail,exit..." << std::endl;

    delete []p;
}

/**
 * @brief write_config
 * @param file
 * 写入样本配置
 */
void write_config( const char *file )
{
    CSetting setting;

    setting.write_config_sample( 2,file );

    std::cout << "write config to " << file << " done." << std::endl;
}

void usage()
{
    std::cout << "usage:netgate [command] [config]" << std::endl;
    std::cout << std::endl;
    std::cout << "command:" << std::endl;
    std::cout << "  -s  start session" << std::endl;
    std::cout << "  -c  write config sample" << std::endl;
    std::cout << std::endl;
    std::cout << "config:" << std::endl;
    std::cout << "  the config file path" << std::endl;
    std::cout << std::endl;
    std::cout << "example:" << std::endl;
    std::cout << "./netgate -c session.json" << std::endl;
    std::cout << "./netgate -s session.json" << std::endl;
}

/**
 * @brief become_daemon
 * 转化为守护进程
 */
void become_daemon()
{
    pid_t pid =fork();
    if ( pid<0 )
    {
        ERROR( "fork error" );
        exit( 0 );
        return;
    }
    else if ( pid > 0 )  //父进程退出，子进程脱离终端shell,ctrl+c或关闭终端shell不会终止程序
        exit( 0 );

    //在Qt creator开启的shell中仍会终止掉程序，在用户终端则不会
    setsid();  //创建新会话，当前会话结束（用户登出）程序也不会终止

    /*
     * 当一个会话产生（如用户登录），会有一个默认的mask权限值，该值决定了在此会话中对文件操作的权限
     * umask(0)就是设置允许当前进程创建文件或者目录最大可操作的权限，比如这里设置为0，它的意思就是0
     * 取反再创建文件时权限相与，也就是：(~0) & mode 等于八进制的值0777 & mode了
     * 如果不需要，可以不设置，用继承的即可
     *
     */
    //umask(0);  // 设置权限掩码
    //chdir("/");  // 设置工作目录,没必要

    /*
     * 关闭文件描述符，这里还是需要用到std::cout std::cerr
     * 可以关闭std::cin
     * 没什么必要
    for (int fd=0;fd < getdtablesize();fd++ )
    {
        close(fd);
    }
    */

    close( STDIN_FILENO );
}

/**
 * @brief thanks
 * 鸣谢相关开源软件
 */
void thanks()
{
    std::cout << "uzone netgate 1.0" << std::endl;
    std::cout << "thanks:" << std::endl;
    std::cout << "  GNU     http://www.gnu.org" << std::endl;
    std::cout << "  libev   http://software.schmorp.de/pkg/libev.html" << std::endl;
    std::cout << "  parson  https://github.com/kgabis/parson" << std::endl;
    std::cout << std::endl;

}

/**
 * @brief sig_set
 * @param signum
 * @param sa_flag
 * 设置单个信号
 */
void sig_set( int32 signum,int32 sa_flag )
{
    struct sigaction sa;

    sa.sa_handler = sig_handler;
    sigemptyset( &sa.sa_mask );     /* will not block any other signal */
    sa.sa_flags = sa_flag;          /* usually SA_RESTART or SA_RESETHAND,man sigaction for more */

    sigaction( signum,&sa,0 );
}

/**
 * @brief sig_handler
 * @param signum
 * 处理信号回调
 */
void sig_handler(int32 signum )
{
    char buff[256];

    snprintf( buff,256,"catch signal %d:%s",signum,strsignal( signum ) );
    ERROR( buff );

    switch ( signum )
    {
    case SIGHUP :
    case SIGINT :
    case SIGQUIT :
    case SIGCONT :
    case SIGTSTP :
    case SIGUSR1 :
    case SIGUSR2 :
        return;
        break;

    case SIGILL :
    case SIGABRT :
    case SIGSEGV :
    case SIGPIPE :
    case SIGCHLD :
    case SIGTTIN :
    case SIGTTOU :
    case SIGXCPU :
    case SIGXFSZ :
    case SIGPWR :

        break;


    case SIGTERM :         //正常关闭
        ERROR( "exit normal..." );
        /* ev_break( EV_DEFAULT,EVBREAK_ALL ); no effect */
        CBackend::instance()->stop();
        break;
    }
}

/**
 * @brief sig_hold
 * 设置所需要捕捉的信号
 */
void sig_hold()
{
    sig_set( SIGCONT,SA_RESTART );
    sig_set( SIGTERM,SA_RESTART );
}
