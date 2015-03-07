#include "CBackend.h"

CBackend *CBackend::m_backend = null;

/**
 * @brief CBackend::CBackend
 * 初始化后台工作
 */
CBackend::CBackend()
{
    m_idle_watcher.set< CBackend,&CBackend::backend_cb >( this );
}

/**
 * @brief CBackend::instance
 * @return 后台工作对象单例指针
 * 获取后台工作对象单例指针
 */
CBackend *CBackend::instance()
{
    if ( !m_backend )
        m_backend = new CBackend();

    return m_backend;
}

/**
 * @brief CBackend::uninstance
 * 销毁后台工作对象单例指针
 */
void CBackend::uninstance()
{
    if ( m_backend )
        delete m_backend;

    m_backend = null;
}

/**
 * @brief CBackend::start
 * 开始进入后台工作
 * 该函数在后台工作结束前不会返回
 */
void CBackend::start()
{
    /**< 确保所有ev_loop使用EV_DEFAULT，否则需要自行处理 */
    struct ev_loop *loop = EV_DEFAULT;
    ev_run (loop, 0);
}

/**
 * @brief CBackend::stop
 * 终止后台工作
 * 直接调用ev_break无法终止libev
 * It is safe to call ev_break from outside any ev_run calls, too,
 * in which case it will have no effect.
 */
void CBackend::stop()
{
    m_idle_watcher.feed_event( EV_CUSTOM );/* 用自定义事件表示停止 */
}

/**
 * @brief CBackend::backend_cb
 * @param w
 * @param revents
 * 后台事件回调
 */
void CBackend::backend_cb(ev::idle &w, int32 revents)
{
    if ( ev::ERROR & revents )
    {
        w.stop();
        ev_break( EV_DEFAULT,EVBREAK_ALL );

        return;
    }

    if ( EV_CUSTOM & revents )
    {
        ev_break( EV_DEFAULT,EVBREAK_ALL );
    }
}
