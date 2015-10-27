#include "CSetting.h"
#include "gslog.h"

#define CONFIG_LP_KEY     "listen_port"
#define CONFIG_URL_KEY    "server_url"
#define CONFIG_SP_KEY     "server_port"

/**
 * @brief CSetting::CSetting
 * 初始化配置
 */
CSetting::CSetting()
{
    m_session_value = null;
    m_session_array = null;
}

/**
 * @brief CSetting::~CSetting
 * 清理配置文件
 */
CSetting::~CSetting()
{
    reset_config();
}

/**
 * @brief CSetting::reset_config
 * 重置配置
 */
void CSetting::reset_config()
{
    if ( m_session_value )
        json_value_free( m_session_value);

    /* json_value_get_array do't allocate any memory,no need to free m_session_array */
    m_session_value = null;
    m_session_array = null;
}

/**
 * @brief CSetting::get_proxy_session_count
 * @return 开启的网关数量
 * 获取配置中网关会话数量
 */
int32 CSetting::get_session_count()
{
    if ( m_session_array )
        return json_array_get_count( m_session_array );

    return 0;
}

/**
 * @brief CSetting::write_config_sample
 * @param session_number
 * @param file
 * 将配置样本写入文件
 */
void CSetting::write_config_sample(int32 session_number, const char *file)
{
    JSON_Value *root_value = json_value_init_array();
    JSON_Array *root_array = json_value_get_array ( root_value );

    /* write session sample */
    for ( int32 i = 0;i < session_number;i ++ )
    {
        JSON_Value *sample_value = json_value_init_object();
        JSON_Object *sample_object = json_value_get_object( sample_value );

        if ( JSONFailure == json_object_set_number( sample_object,CONFIG_LP_KEY,7998+i )
            || JSONFailure == json_object_set_string( sample_object,CONFIG_URL_KEY,"127.0.0.1" )
            || JSONFailure == json_object_set_number( sample_object,CONFIG_SP_KEY,6998+i ) )
        {
            ERROR( "fail to set json value" );

            json_value_free( sample_value );

            break;
        }

        json_array_append_value( root_array,sample_value );
        /*
        json_array_append_value,just set a pointer,you can't reused or free sample_value when
        you still need it.
        */
    }

    if ( JSONFailure == json_serialize_to_file( root_value,file ) )
    {
        ERROR( "fail to serialize json to file");
    }

    json_value_free( root_value ); /* free a array or object will free it's child node */
}

/**
 * @brief CSetting::read_config
 * @param file
 * @return
 */
bool CSetting::read_config(const char *file)
{
    reset_config(); /* in case someone read more than one times */

    m_session_value = json_parse_file( file );

    if ( null == m_session_value )
    {
        return false;
    }

    m_session_array = json_value_get_array( m_session_value );

    return true;
}

/**
 * @brief CSetting::get_session_listen_port
 * @param session_number 从0开始
 * @return 网关监听端口
 * 获取网关监听端口
 */
int32 CSetting::get_session_listen_port(int32 session_number)
{
    if ( session_number > get_session_count() )
        return 0;

    JSON_Object *session_object = json_array_get_object( m_session_array,session_number );

    return static_cast<int32>( json_object_get_number( session_object,CONFIG_LP_KEY ) );
}

/**
 * @brief CSetting::get_session_server_url
 * @param session_number
 * @return 服务器url
 * 获取服务器url
 */
const char *CSetting::get_session_server_url(int32 session_number)
{
    if ( session_number > get_session_count() )
        return 0;

    JSON_Object *session_object = json_array_get_object( m_session_array,session_number );

    return json_object_get_string( session_object,CONFIG_URL_KEY );
}

/**
 * @brief CSetting::get_session_server_port
 * @param session_number
 * @return 服务器端口
 * 获取服务器端口
 */
int32 CSetting::get_session_server_port(int32 session_number)
{
    if ( session_number > get_session_count() )
        return 0;

    JSON_Object *session_object = json_array_get_object( m_session_array,session_number );

    return static_cast<int32>( json_object_get_number( session_object,CONFIG_SP_KEY ) );
}
