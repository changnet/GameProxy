#ifndef CBACKEND_H
#define CBACKEND_H

#include <ev++.h>
#include "gstypes.h"

/**
 * @brief The CBackend class
 * 后台工作类
 */
class CBackend
{
public:
    static CBackend *instance();
    static void uninstance();

    void start();
    void stop();
private:
    CBackend();

    static CBackend *m_backend;

    ev::idle m_idle_watcher;
    void backend_cb( ev::idle &w,int32 revents );
};

#endif // CBACKEND_H
