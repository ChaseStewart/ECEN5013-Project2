/****************************************************************
* file  : socketTIVA.c
* brief : This file implements the socket functionalities
* authors: Raj Kumar Subramaniam, Chase Stewart
* date  : 11/29/2017
*****************************************************************/

#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/rom.h" /* needed for MAC addr */
#include "utils/lwiplib.h"
#include "utils/locator.h"
#include "utils/ustdlib.h"

#include "common.h"
#include "socketTIVA.h"

/* TODO SET STATE VAR */

/*Global variable for Queue Handles in main*/
extern QueueHandle_t mainQueue;
extern QueueHandle_t tempQueue;
extern QueueHandle_t lightQueue;
extern QueueHandle_t socketQueue;
extern QueueHandle_t soilQueue;
extern QueueHandle_t chargeQueue;

extern bool stateRunning;
int32_t g_ui32IPAddress;

struct tcp_pcb *pcb;
struct ip_addr my_ip;
struct ip_addr host_ip;

bool is_ready = true;

void socketTask(void *pvParameters)
{
    uint8_t pui8MACArray[8];            /* Variable to store the MAC address */
    message_t queueData;                /* Variable to store msgs read from queue */
    uint32_t notificationValue = 0;
    uint32_t ui32User0, ui32User1;
    uint32_t g_ui32IPAddress = 0;
    err_t err;

    stateRunning = true;
    /* Thanks to TI for the enet_lwip example code to get an IP below. */

    //
    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.  The MAC address will be stored in the non-volatile
    // USER0 and USER1 registers.
    //
    MAP_FlashUserGet(&ui32User0, &ui32User1);
    if((ui32User0 == 0xffffffff) || (ui32User1 == 0xffffffff))
    {
        //
        // We should never get here.  This is an error if the MAC address has
        // not been programmed into the device.  Exit the program.
        // Let the user know there is no MAC address
        //
        UARTprintf("\r\nNo MAC programmed!\n");
        while(1)
        {
        }
    }

    //
    // Tell the user what we are doing just now.
    //
    UARTprintf("\r\nWaiting for IP.");

    //
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
    // address needed to program the hardware registers, then program the MAC
    // address into the Ethernet Controller registers.
    //
    pui8MACArray[0] = ((ui32User0 >>  0) & 0xff);
    pui8MACArray[1] = ((ui32User0 >>  8) & 0xff);
    pui8MACArray[2] = ((ui32User0 >> 16) & 0xff);
    pui8MACArray[3] = ((ui32User1 >>  0) & 0xff);
    pui8MACArray[4] = ((ui32User1 >>  8) & 0xff);
    pui8MACArray[5] = ((ui32User1 >> 16) & 0xff);

    //
    // Initialize the lwIP library, using DHCP.
    //
    lwIPInit(SYSTEM_CLOCK, pui8MACArray, 0, 0, 0, IPADDR_USE_DHCP);

    //
    // Setup the device locator service.
    //
    LocatorInit();
    LocatorMACAddrSet(pui8MACArray);
    LocatorAppTitleSet("EK-TM4C1294XL enet_io");

    //
    // Set the interrupt priorities.  We set the SysTick interrupt to a higher
    // priority than the Ethernet interrupt to ensure that the file system
    // tick is processed if SysTick occurs while the Ethernet handler is being
    // processed.  This is very likely since all the TCP/IP and HTTP work is
    // done in the context of the Ethernet interrupt.
    //
    MAP_IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);
    MAP_IntPrioritySet(FAULT_SYSTICK, SYSTICK_INT_PRIORITY);

    tcp_socket_init(IP_ADDR_ANY);

    while(stateRunning)
    {
        xTaskNotifyWait(0x00, ULONG_MAX, &notificationValue, portMAX_DELAY);   /*Blocks indefinitely waiting for notification*/
        if(notificationValue & TASK_NOTIFYVAL_HEARTBEAT)
        {
            sendHeartBeat(SOCKET_TASK_ID);
        }
        if(notificationValue & TASK_NOTIFYVAL_MSGQUEUE)
        {
            while(errQUEUE_EMPTY != xQueueReceive(socketQueue,(void*)&queueData,0))                     /*Non-blocking call, Read Until Queue is empty*/
            {
                UARTprintf("\r\nSocket Task Received a Queue Data");
                if(queueData.id == HEARTBEAT_REQ)
                {
                    sendHeartBeat(SOCKET_TASK_ID);
                }
                if(queueData.id == LOGGER)
                {
                    UARTprintf("\r\nSocket Received Message: %s", queueData.data.message);
                   // tcp_write(pcb, queueData.data.message , queueData.length, TCP_WRITE_FLAG_COPY);
                    //tcp_output(pcb);
                }
            }
        }
    }
    vTaskDelete(NULL);  /*Deletes Current task and frees up memory*/
}

void tcp_socket_init(uint32_t local_addr)
{
    struct tcp_pcb *pcb;
    err_t err;

    UARTprintf("\r\nInitializing listener");
    pcb = tcp_new();
    if (pcb == NULL)
    {
        UARTprintf("\r\nhttpd_init: tcp_new failed");
    }
    tcp_setprio(pcb, SOCKET_TCP_PRIO);

    /* set SOF_REUSEADDR here to explicitly bind httpd to multiple interfaces */
    err = tcp_bind(pcb, local_addr, SOCKET_PORT);
    if ( err != ERR_OK )
    {
        UARTprintf("\r\nhttpd_init: tcp_bind failed with error %d", err);
    }
    UARTprintf("\r\nBound to port %d", SOCKET_PORT);

    pcb = tcp_listen(pcb);
    if (pcb == NULL)
    {
        UARTprintf("\r\nhttpd_init: tcp_listen failed");
    }

    /* initialize callback arg and accept callback */
    tcp_arg(pcb, pcb);
    tcp_accept(pcb, socket_accept);
}


 /*
 * A new incoming connection has been accepted.
 */

static err_t socket_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
     UARTprintf("\r\nConnected %p / %p", (void*)pcb, arg);
     struct tcp_pcb_listen *lpcb = (struct tcp_pcb_listen*)arg;

    /* Decrease the listen backlog counter */
    tcp_accepted(lpcb);

    /* Set priority */
    tcp_setprio(pcb, SOCKET_TCP_PRIO);

    /* Tell TCP that this is the structure we wish to be passed for our
     callbacks. */
    tcp_arg(pcb, pcb);

    /* Set up the various callback functions */
    tcp_recv(pcb, socket_recv);
    tcp_err(pcb, socket_err);
    tcp_poll(pcb, socket_close_conn, SOCKET_POLL_INTERVAL);
    tcp_sent(pcb, socket_sent);


    //tcp_write(pcb, "Hello\0", 6, TCP_WRITE_FLAG_COPY);
   // tcp_output(pcb);


    return ERR_OK;
}


static void socket_err(void *arg, err_t err)
{
  UARTprintf("\r\ntcp_err: %s", lwip_strerr(err));
  socket_close_conn(pcb);
}



static err_t socket_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
   UARTprintf("\r\nsent data");
   return ERR_OK;
}


static err_t socket_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    UARTprintf("\r\nReceived data: %s", (char *)p->payload);
    return ERR_OK;
}


static err_t socket_poll(void *arg, struct tcp_pcb *pcb)
{
    return ERR_OK;
}


static err_t socket_close_conn(struct tcp_pcb *pcb)
{
    err_t err;
    //UARTprintf("\r\nClosing connection");

    /* Set the callbacks to none */
    /*
    tcp_arg(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_err(pcb, NULL);
    tcp_poll(pcb, NULL, 0);
    tcp_sent(pcb, NULL);

    err = tcp_close(pcb);
    if (err != ERR_OK) {
        UARTprintf("\r\nError %d closing connection", err);
    }
    */
    return ERR_OK;
}



void DisplayIPAddress(uint32_t ui32Addr)
{
    char pcBuf[16];

    // Convert the IP Address into a string.
    usprintf(pcBuf, "%d.%d.%d.%d", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);

    // Display the string.
    UARTprintf("\r\nIP Addr is: ");
    UARTprintf(pcBuf);
}


void mySysTickHandler(void)
{
    //
    // Call the lwIP timer handler.
    //
    lwIPTimer(10);
}



void lwIPHostTimerHandler(void)
{
    uint32_t ui32Idx, ui32NewIPAddress;


    // Get the current IP address.
    ui32NewIPAddress = lwIPLocalIPAddrGet();

    // See if the IP address has changed.
    if(ui32NewIPAddress != g_ui32IPAddress)
    {
        // See if there is an IP address assigned.
        if(ui32NewIPAddress == 0xffffffff)
        {
            // Indicate that there is no link.
            UARTprintf("\r\nWaiting for link.");
        }
        else if(ui32NewIPAddress == 0)
        {
            //
            // There is no IP address, so indicate that the DHCP process is
            // running.
            //
            UARTprintf("\r\nWaiting for IP address.");
        }
        else
        {
            //
            // Display the new IP address.
            //
            DisplayIPAddress(ui32NewIPAddress);
            is_ready = true;
        }

        //
        // Save the new IP address.
        //
        g_ui32IPAddress = ui32NewIPAddress;

        //
        // Turn GPIO off.
        //
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, ~GPIO_PIN_1);
    }

    //
    // If there is not an IP address.
    //
    if((ui32NewIPAddress == 0) || (ui32NewIPAddress == 0xffffffff))
    {
        //
        // Loop through the LED animation.
        //
        for(ui32Idx = 1; ui32Idx < 17; ui32Idx++)
        {

            //
            // Toggle the GPIO
            //
            MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,
                    (MAP_GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_1) ^
                     GPIO_PIN_1));

            SysCtlDelay(SYSTEM_CLOCK/(ui32Idx << 1));
        }
    }
}
