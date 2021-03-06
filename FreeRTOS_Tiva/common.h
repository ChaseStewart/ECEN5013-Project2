/****************************************************************
* file  : common.h
* brief : This file implements the macros and structures which are
*         used in common across the project
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "drivers/pinout.h"
#include "utils/uartstdio.h"


// TivaWare includes
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/gpio.h"


#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

// FreeRTOS includes
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*System clock rate, 120 MHz*/
#define SYSTEM_CLOCK    120000000U

/*Macro Definitions*/
#define MAIN_QUEUE_SIZE     (10)
#define TEMP_QUEUE_SIZE     (10)
#define LIGHT_QUEUE_SIZE    (10)
#define SOCKET_QUEUE_SIZE   (10)
#define SOIL_QUEUE_SIZE     (10)
#define CHARGE_QUEUE_SIZE   (10)

#define TASK_NOTIFYVAL_MSGQUEUE     (0x01)  /*This would be the common notification value when a msg is sent to a task*/
#define TASK_NOTIFYVAL_HEARTBEAT    (0x02)  /*Notification Value to send Heart-beat*/
#define TASK_NOTIFYVAL_REQDATA      (0x04)  /*Used by main to request data*/
#define TASK_NOTIFYVAL_SENDTOSCKT   (0x08)  /*Used by main to send data to socket*/
#define ULONG_MAX                   (0xFFFFFFFF)

/*HBs Tracking*/
#define HB_OK_LIGHT      (0x01)
#define HB_OK_TEMP       (0x02)
#define HB_OK_SOCKET     (0x04)
#define HB_OK_SOIL       (0x08)
#define HB_OK_CHARGE     (0x10)

/*Data Packets Enum*/
typedef enum packet_type {
    UNUSED = 0,
    DATA_PKT,
    NO_DATA_PKT,
    ALRT_PKT,
    LOG_PKT,
    ERR_PKT
} Packet_Type;

/*Message IDs to differentiate between messages*/
typedef enum message_type
{
    HEARTBEAT_REQ = 0,
    HEARTBEAT_RSP,
    TEMP_VALUE,
    TEMP_DATA_REQ,
    LIGHT_VALUE,
    LIGHT_DATA_REQ,
    LOGGER,
    FILE_CHANGE,
    CLIENT_SHUTDOWN,
    SOIL_MOIST_DATA,
    SOIL_MOIST_DATA_REQ,
    FUEL_GAUGE_DATA,
    FUEL_GAUGE_DATA_REQ,
    NOTIFY_USER,
    ERROR_OCCURED
}Message_Type;

typedef enum taskID
{
    MAIN_TASK_ID = 0,
    TEMP_TASK_ID,
    LIGHT_TASK_ID,
    SOCKET_TASK_ID,
    SOIL_TASK_ID,
    FUEL_TASK_ID
}Task_Id;

/*Structure to send messages into the queue*/
typedef struct
{
    Message_Type id;        /*Message ID*/
    uint32_t timestamp;     /*Time stamp*/
    Task_Id  source;        /*Source*/
    size_t length;          /*Size of the message*/
    union
    {
        uint8_t* message;   /*Message Payload should be a String*/
        uint32_t intData;   /*Use this when sending Numeric values*/
    }data;
    size_t priority;        /*Priority of the message*/
}message_t;

/*Structures to send data over the socket*/
typedef struct
{
    Message_Type id;     /*Type of information from the Client*/
    size_t dataLen;     /*Length of the data*/
    uint8_t *data;      /*Actual data in the packet*/
}packet_t;

/*Global variables*/
/*Use extern to avoid duplicate variables creation during inclusion*/

/*****************************************************************
* name  : sendHeartBeat
* brief : This function is sends heart-beat to the main task
* param : Task_Id - Task ID of the calling task
* return: 0 on SUCCESS, -1 on Failure
*****************************************************************/
int8_t sendHeartBeat(Task_Id taskId);

/*****************************************************************
* name  : sendDataToMain
* brief : This function is sends sensor data to the main task
* param : Task_Id - Task ID of the calling task, Msg ID, Data -  Sensor data
* return: 0 on SUCCESS, -1 on Failure
*****************************************************************/
int8_t sendDataToMain(Task_Id taskId, Message_Type msgId, int32_t data);

#endif /* COMMON_H_ */
