#include "customnvsapi.h"
#include "string.h"
#include "esp_log.h"
#define		TAG		"nvs_config"
/** 从nvs中读取字符值
 * @param namespace NVS命名空间
 * @param key 要读取的键值
 * @param value 读到的值
 * @param maxlen 外部存储数组的最大值
 * @return 读取到的字节数
*/
size_t read_nvs_str(const char* namespace,const char* key,char* value,int maxlen)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret_val = ESP_FAIL;
    size_t required_size = 0;
    ESP_ERROR_CHECK(nvs_open(namespace, NVS_READWRITE, &nvs_handle));
    ret_val = nvs_get_str(nvs_handle, key, NULL, &required_size);
    if(ret_val == ESP_OK && required_size <= maxlen)
    {
        nvs_get_str(nvs_handle,key,value,&required_size);
    }
    else
        required_size = 0;
    nvs_close(nvs_handle);
    return required_size;
}

/** 写入值到NVS中（字符数据）
 * @param namespace NVS命名空间
 * @param key NVS键值
 * @param value 需要写入的值
 * @return ESP_OK or ESP_FAIL
*/
esp_err_t write_nvs_str(const char* namespace,const char* key,const char* value)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;
    ESP_ERROR_CHECK(nvs_open(namespace, NVS_READWRITE, &nvs_handle));
    
    ret = nvs_set_str(nvs_handle, key, value);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return ret;
}

/** 从nvs中读取字节数据（二进制）
 * @param namespace NVS命名空间
 * @param key 要读取的键值
 * @param value 读到的值
 * @param maxlen 外部存储数组的最大值
 * @return 读取到的字节数
*/
size_t read_nvs_blob(const char* namespace,const char* key,uint8_t *value,int maxlen)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret_val = ESP_FAIL;
    size_t required_size = 0;
    ESP_ERROR_CHECK(nvs_open(namespace, NVS_READWRITE, &nvs_handle));
    ret_val = nvs_get_blob(nvs_handle, key, NULL, &required_size);
    if(ret_val == ESP_OK && required_size <= maxlen)
    {
        nvs_get_blob(nvs_handle,key,value,&required_size);
    }
    else
        required_size = 0;
    nvs_close(nvs_handle);
    return required_size;
}

/** 擦除nvs区中某个键
 * @param namespace NVS命名空间
 * @param key 要读取的键值
 * @return 错误值
*/
esp_err_t erase_nvs_key(const char* namespace,const char* key)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret_val = ESP_FAIL;
    ESP_ERROR_CHECK(nvs_open(namespace, NVS_READWRITE, &nvs_handle));
    ret_val = nvs_erase_key(nvs_handle,key);
    ret_val = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return ret_val;
}

/** 写入值到NVS中(字节数据)
 * @param namespace NVS命名空间
 * @param key NVS键值
 * @param value 需要写入的值
 * @return ESP_OK or ESP_FAIL
*/
esp_err_t write_nvs_blob(const char* namespace,const char* key,uint8_t* value,size_t len)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;
    ESP_ERROR_CHECK(nvs_open(namespace, NVS_READWRITE, &nvs_handle));
    ret = nvs_set_blob(nvs_handle, key, value,len);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return ret;
}

// #define NVS_STATION_NAMESPACE		"STATION"		//基站参数
// #define NVS_WIFIHALOW_NAMESPACE		"WIFIHALOW"		//基站WifiHalow
// #define NVS_DEVICE_NAMESPACE			"DEVICE"		//设备参数
// #define NVS_USER_NAMESPACE			"USER"			//用户参数
/** NVS配置初始化包含 设备配置、基站配置、基站wifiHaloW配置、用户配置
 * @return ESP_OK or ESP_FAIL
*/
esp_err_t nvs_config_init()
{
	//erase_nvs_key(NVS_DEVICE_NAMESPACE,NVS_DEVICE_CONFIG_KEY);
	char read_buf[16];
	memset(read_buf,0,16);
    int len = 0;
	len = read_nvs_str(NVS_DEVICE_NAMESPACE,NVS_DEVICE_CONFIG_KEY,read_buf,16);
	if(len == 0)//没有写过配置进行默认配置写入
	{
		ESP_LOGI(TAG,"nvs flash config is null");
		// write_nvs_str(NVS_DEVICE_NAMESPACE,NVS_DEVICE_CONFIG_KEY,"dafault"); 

		// write_nvs_str(NVS_DEVICE_NAMESPACE,NVS_DEVICE_ID_KEY,"163"); 
		// write_nvs_str(NVS_DEVICE_NAMESPACE,NVS_DEVICE_TYPE_KEY,"beidou");
		
		// setDeviceId("163");
		// setDeviceType("beidou");

		// write_nvs_str(NVS_STATION_NAMESPACE,NVS_STATION_ID_KEY,"000");
		// write_nvs_str(NVS_STATION_NAMESPACE,NVS_STATION_TYPE_KEY,"Versatile");
		// setStationId("000");
		// setStationType("Versatile");

		// write_nvs_str(NVS_WIFIHALOW_NAMESPACE,NVS_WIFIHALOW_SSID_KEY,"EYINHE_AP100");
		// write_nvs_str(NVS_WIFIHALOW_NAMESPACE,NVS_WIFIHALOW_PASSWORD_KEY,"eyinhe01");
		// setWifiHalowSsid("EYINHE_AP100");
		// setWifiHalowPassword("eyinhe01");

		// write_nvs_str(NVS_USER_NAMESPACE,NVS_USER_NAME_KEY,"sx1163");
		// write_nvs_str(NVS_USER_NAMESPACE,NVS_USER_ROLE_KEY,"root");
		// setUserName("sx1163");
		// setUserRole("root");
	}
	else//读取配置文件中的配置写入全局变量
	{
		ESP_LOGI(TAG,"nvs flash config read now");
		len = 0;
		bzero(read_buf,16);
		len = read_nvs_str(NVS_STATION_NAMESPACE,NVS_STATION_ID_KEY,read_buf,16);
		ESP_LOGI(TAG,"nvs flash stationId = %s len=%d",read_buf,len);
		// setStationId(read_buf);
		len = 0;
		bzero(read_buf,16);
		len = read_nvs_str(NVS_STATION_NAMESPACE,NVS_STATION_TYPE_KEY,read_buf,16);
		ESP_LOGI(TAG,"nvs flash stationType = %s len=%d",read_buf,len);
		// setStationType(read_buf);

		len = 0;
		bzero(read_buf,16);
		len = read_nvs_str(NVS_DEVICE_NAMESPACE,NVS_DEVICE_ID_KEY,read_buf,16);
		ESP_LOGI(TAG,"nvs flash deviceID = %s len=%d",read_buf,len);
		// setDeviceId(read_buf);
		len = 0;
		bzero(read_buf,16);
		len = read_nvs_str(NVS_DEVICE_NAMESPACE,NVS_DEVICE_TYPE_KEY,read_buf,16);
		ESP_LOGI(TAG,"nvs flash deviceType = %s len=%d",read_buf,len);
		// setDeviceType(read_buf);

		len = 0;
		bzero(read_buf,16);
		len = read_nvs_str(NVS_WIFIHALOW_NAMESPACE,NVS_WIFIHALOW_SSID_KEY,read_buf,16);
		ESP_LOGI(TAG,"nvs flash wifihalow ssid = %s len=%d",read_buf,len);
		// setWifiHalowSsid(read_buf);
		len = 0;
		bzero(read_buf,16);
		len = read_nvs_str(NVS_WIFIHALOW_NAMESPACE,NVS_WIFIHALOW_PASSWORD_KEY,read_buf,16);
		ESP_LOGI(TAG,"nvs flash wifihalow password = %s len=%d",read_buf,len);
		// setWifiHalowPassword(read_buf);

		len = 0;
		bzero(read_buf,16);
		len = read_nvs_str(NVS_USER_NAMESPACE,NVS_USER_NAME_KEY,read_buf,16);
		ESP_LOGI(TAG,"nvs flash userName = %s len=%d",read_buf,len);
		// setUserName(read_buf);
		len = 0;
		bzero(read_buf,16);
		len = read_nvs_str(NVS_USER_NAMESPACE,NVS_USER_ROLE_KEY,read_buf,16);
		ESP_LOGI(TAG,"nvs flash userRole = %s len=%d",read_buf,len);
		// setUserRole(read_buf);
	}
	return 0;
}