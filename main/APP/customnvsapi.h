/**
 ****************************************************************************************************
 * @file        customnvsapi.h
 * @author      fzk
 * @version     V1.0
 * @date        2024-08-09
 * @brief       nvs flash区读写函数封装
 ****************************************************************************************************
**/

#ifndef __CUSTOMNVSAPI_H_
#define __CUSTOMNVSAPI_H_
#include "nvs_flash.h"

size_t read_nvs_str(const char* namespace,const char* key,char* value,int maxlen);
esp_err_t write_nvs_str(const char* namespace,const char* key,const char* value);
size_t read_nvs_blob(const char* namespace,const char* key,uint8_t *value,int maxlen);
esp_err_t erase_nvs_key(const char* namespace,const char* key);
esp_err_t write_nvs_blob(const char* namespace,const char* key,uint8_t* value,size_t len);


esp_err_t nvs_config_init();


//需要时再更换为结构体保存
// typedef struct nvc_config_station
// {
// 	char stationId[16];
// 	char stationType[16];
// }st_config_station;
// typedef struct nvc_config_device
// {
// 	char deviceId[16];
// 	char deviceType[16];
// }st_config_device;
// typedef struct nvc_config_wifihalow
// {
// 	char ssid[16];
// 	char password[16];
// }st_config_wifihalow;
// typedef struct nvc_config_user
// {
// 	char userName[16];
// 	char userRole[16];
// }st_config_user;

#define NVS_STATION_NAMESPACE		"STA"		//基站参数
#define NVS_WIFIHALOW_NAMESPACE		"HALOW"		//基站WifiHalow
#define NVS_DEVICE_NAMESPACE		"DEV"		//设备参数
#define NVS_USER_NAMESPACE			"USER"		//用户参数

#define NVS_STATION_ID_KEY			"id"		//基站ID
#define NVS_STATION_TYPE_KEY		"type"		//基站类型

#define NVS_WIFIHALOW_SSID_KEY		"ssid" 		//基站WifiHalowSsid
#define NVS_WIFIHALOW_PASSWORD_KEY	"pw"		//基站WifiHalowPassword

#define NVS_DEVICE_CONFIG_KEY		"config"	//设备配置标志
#define NVS_DEVICE_ID_KEY			"id"		//设备ID
#define NVS_DEVICE_TYPE_KEY			"type"		//设备TYPE

#define NVS_USER_NAME_KEY			"name"		//用户名称
#define NVS_USER_ROLE_KEY			"role"		//用户角色

#endif
