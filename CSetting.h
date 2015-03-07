#ifndef CSETTING_H
#define CSETTING_H

#include "parson.h"
#include "gstypes.h"

/**
 * @brief The CSetting class
 * 配置读取类
 */
class CSetting
{
public:
    CSetting();
    ~CSetting();

    int32 get_session_count();
    bool read_config( const char *file );
    void write_config_sample(int32 session_number,const char *file);

    int32 get_session_listen_port( int32 session_number );
    const char *get_session_server_url( int32 session_number );
    int32 get_session_server_port( int32 session_number );

private:
    JSON_Value *m_session_value;
    JSON_Array *m_session_array;

    void reset_config();
};

#endif // CSETTING_H
