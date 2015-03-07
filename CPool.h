#ifndef CPOOL_H
#define CPOOL_H

#include <iostream>
#include <deque>
#include <ev++.h>

#include "gstypes.h"

#define MAX_REUSED_OBJECT    20480  /**< 缓存的最大对象指针数量 */

template <class T>
/**
 * @brief The CPool class
 * 对象重用池
 */
class CPool
{
public:
    static CPool *instance();
    static void uninstance();

    void push_object( T *p );
    T *pop_object();
private:
    CPool();
    ~CPool();

    static CPool *m_pool;

    std::deque< T * > m_queue;

    ev::idle m_reused_watcher;

    void reused_cb( ev::idle &w,int revents );
};

/**
 * @brief CPool::push_object
 * @param p
 * 空闲的对象指针入队列
 */
template <class T>
void CPool<T>::push_object(T *p)
{
    /* 无论是否达上限，都放到队列中。不允许直接delete，需要保证在当前loop中，对指针p的操作都是安全的 */
    m_queue.push_back( p );

    if ( !m_reused_watcher.is_pending() )
        m_reused_watcher.feed_event( 0 );
}

template <class T>
/**
 * @brief CPool::reused_cb
 * @param w
 * @param revents
 * 当一个对象完成自己的任务后，有两种方式销毁
 * 1.自杀。但需要保证自杀后不要调用对象的成员数据
 * 2.交给别人管理，在下一轮事件循环中处理
 * 这里采用第二种，更安全
 */
void CPool<T>::reused_cb(ev::idle &w, int revents)
{
    if ( ev::ERROR & revents )
    {
        w.stop();
        return;
    }

    /* 在下一轮loop中，将多出的对象销毁 */
    while ( MAX_REUSED_OBJECT < m_queue.size() )
    {
        typename std::deque<T *>::iterator itr = m_queue.begin();
        delete *itr;

        m_queue.erase( itr );/* 如果存的是对象，erase会自动销毁内存，存指针则不会 */
    }
}

template <class T>
/**
 * @brief CPool::pop_object
 * @return 对象指针，失败为null
 * 从池中弹出一个可用对象
 * 如果池为空，则返回null
 */
T *CPool<T>::pop_object()
{
    if ( m_queue.size() > 0 )
    {
        T *p = m_queue.front();

        m_queue.pop_front();

        return p;
    }

    return null;
}


template <class T>
CPool<T> *CPool<T>::m_pool = null;

template <class T>
/**
 * @brief CPool::CPool
 * 初始化对象池
 */
CPool<T>::CPool()
{
    m_reused_watcher.set< CPool,&CPool::reused_cb >( this );
}

template <class T>
/**
 * @brief CPool<T>::~CPool
 * 清理重用池内存
 */
CPool<T>::~CPool()
{
    typename std::deque<T*>::iterator itr = m_queue.begin();
    while ( itr != m_queue.end() )
    {
        delete *itr;

        itr ++;
    }

    m_queue.clear();
}

template <class T>
/**
 * @brief CPool::instance
 * @return 唯一对象指针
 * 获取单例对象指针
 */
CPool<T> *CPool<T>::instance()
{
    if ( !m_pool )
        m_pool = new CPool();

    return m_pool;
}

template <class T>
/**
 * @brief CPool::uninstance
 * 销毁单例对象指针
 */
void CPool<T>::uninstance()
{
    if ( m_pool )
    {
        delete m_pool;
        m_pool = null;
    }
}

#endif // CPOOL_H
