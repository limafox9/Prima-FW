#include "main.h"
#include "stm32f1xx_hal.h"
//#include "debug.h"

#include "wizchip_conf.h"
#include "socket.h"
#include "dhcp.h"
#include "dns.h"
#include "loopback.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>







/* USER CODE BEGIN Includes */
/* vim: set ai et ts=4 sw=4: */
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include "socket.h"
#include "dhcp.h"
#include "dns.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
extern SPI_HandleTypeDef hspi1;


/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI1_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

#define DHCP_SOCKET     0
#define DNS_SOCKET      1
#define HTTP_SOCKET     2

void W5500_Select(void) {
    HAL_GPIO_WritePin(ETH_SPI1_nCS_GPIO_Port, ETH_SPI1_nCS_Pin, GPIO_PIN_RESET);
}

void W5500_Unselect(void) {
    HAL_GPIO_WritePin(ETH_SPI1_nCS_GPIO_Port, ETH_SPI1_nCS_Pin, GPIO_PIN_SET);
}

uint8_t W5500_ReadByte(void) {
	uint8_t rbuf;
	HAL_SPI_Receive(&hspi1, &rbuf, 1, 0xFFFFFFFF);
	return rbuf;
}

void W5500_WriteByte(uint8_t b) {
	HAL_SPI_Transmit(&hspi1, &b, 1, 0xFFFFFFFF);
}






void W5500_Init() {

	uint8_t retVal, sockStatus;
	int16_t rcvLen;
	uint8_t rcvBuf[20], bufSize[] = {2, 2, 2, 2, 2, 2, 2, 2};
//	uint8_t memsize[2][8] = { {2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};

	reg_wizchip_cs_cbfunc(W5500_Select, W5500_Unselect);
	reg_wizchip_spi_cbfunc(W5500_ReadByte, W5500_WriteByte);
//	reg_wizchip_spiburst_cbfunc(W5500_ReadBuff, W5500_WriteBuff);

	wizchip_init(bufSize, bufSize);
	wiz_NetInfo netInfo = { .mac 	= {0x11, 0x22, 0x33, 0x44, 0x55, 0xff},	// Mac address
						  .ip 	= {192, 168, 1, 99},					// IP address
						  .sn 	= {255, 255, 255, 0},					// Subnet mask
						  .gw 	= {192, 168, 1, 1}};					// Gateway address
	wizchip_setnetinfo(&netInfo);
	wizchip_getnetinfo(&netInfo);
//	PRINT_NETINFO(netInfo);

}

















//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//extern SPI_HandleTypeDef hspi1;
//
//#define RESET_PORT GPIOA
//#define RESET_PIN GPIO_PIN_0
//#define CS_PORT GPIOB
//#define CS_PIN GPIO_PIN_6
//
//#define DHCP_SOCKET     0
//#define DNS_SOCKET      1
//#define HTTP_SOCKET     2
//
//#ifndef DATA_BUF_SIZE
//	#define DATA_BUF_SIZE			2048
//#endif
//
//
//
//
//
//
//
////
////void W5500_Reset(void) {
////    HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_RESET);
////    HAL_Delay(1);
////    HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_SET);
////}
//
//void W5500_Select(void) {
//    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
//}
//
//void W5500_Unselect(void) {
//    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
//}
//
//void W5500_ReadBuff(uint8_t* buff, uint16_t len) {
//    HAL_SPI_Receive(&hspi1, buff, len, HAL_MAX_DELAY);
//}
//
//void W5500_WriteBuff(uint8_t* buff, uint16_t len) {
//    HAL_SPI_Transmit(&hspi1, buff, len, HAL_MAX_DELAY);
//}
//
//uint8_t W5500_ReadByte(void) {
//    uint8_t byte;
//    W5500_ReadBuff(&byte, sizeof(byte));
//    return byte;
//}
//
//void W5500_WriteByte(uint8_t byte) {
//    W5500_WriteBuff(&byte, sizeof(byte));
//}
//
//volatile bool ip_assigned = false;
//
//void Callback_IPAssigned(void) {
//	debugf("Callback: IP assigned! Leased time: %d sec\r\n", getDHCPLeasetime());
//    ip_assigned = true;
//}
//
//void Callback_IPConflict(void) {
//	debugf("Callback: IP conflict!\r\n");
//}
//
//
//
//
//
//
//
//
//
//
//// 1K should be enough, see https://forum.wiznet.io/t/topic/1612/2
//uint8_t dhcp_buffer[1024];
//// 1K seems to be enough for this buffer as well
//uint8_t dns_buffer[1024];
//
//
//int8_t W5500_Init() {
//
//	HAL_Delay(500);
//
////	W5500_Unselect();
////	W5500_Reset();
//
//	debugf("Registering W5500 callbacks...\r\n");
//	    reg_wizchip_cs_cbfunc(W5500_Select, W5500_Unselect);
//	    reg_wizchip_spi_cbfunc(W5500_ReadByte, W5500_WriteByte);
//	    reg_wizchip_spiburst_cbfunc(W5500_ReadBuff, W5500_WriteBuff);
//
//	    debugf("Calling wizchip_init()...\r\n");
//	    uint8_t rx_tx_buff_sizes[] = {2, 2, 2, 2, 2, 2, 2, 2};
//	    wizchip_init(rx_tx_buff_sizes, rx_tx_buff_sizes);
//
//
//	    debugf("Calling DHCP_init()...\r\n");
//	        wiz_NetInfo net_info = {
//	            .mac  = { 0xEA, 0x11, 0x22, 0x33, 0x44, 0xEA },
//	            .dhcp = NETINFO_DHCP
//	        };
//	        // set MAC address before using DHCP
//	        setSHAR(net_info.mac);
//	        DHCP_init(DHCP_SOCKET, dhcp_buffer);
//
//	        debugf("Registering DHCP callbacks...\r\n");
//	            reg_dhcp_cbfunc(
//	                Callback_IPAssigned,
//	                Callback_IPAssigned,
//	                Callback_IPConflict
//	            );
//
//	debugf("Calling DHCP_run()...\r\n");
//	// actually should be called in a loop, e.g. by timer
//	uint32_t ctr = 10000;
//	while((!ip_assigned) && (ctr > 0)) {
//		DHCP_run();
//		ctr--;
//	}
//	if(!ip_assigned) {
//		debugf("\r\nIP was not assigned :(\r\n");
//		return;
//	}
//
//	getIPfromDHCP(net_info.ip);
//	getGWfromDHCP(net_info.gw);
//	getSNfromDHCP(net_info.sn);
//
//	uint8_t dns[4];
//	getDNSfromDHCP(dns);
//
//	debugf("IP:  %d.%d.%d.%d\r\nGW:  %d.%d.%d.%d\r\nNet: %d.%d.%d.%d\r\nDNS: %d.%d.%d.%d\r\n",
//		net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3],
//		net_info.gw[0], net_info.gw[1], net_info.gw[2], net_info.gw[3],
//		net_info.sn[0], net_info.sn[1], net_info.sn[2], net_info.sn[3],
//		dns[0], dns[1], dns[2], dns[3]
//	);
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
////void network_init(void)
////{
////  wiz_NetTimeout gWIZNETTIME = {.retry_cnt = 3, .time_100us = 2000};
////  ctlnetwork(CN_SET_TIMEOUT,(void*)&gWIZNETTIME);
////  ctlnetwork(CN_GET_TIMEOUT, (void*)&gWIZNETTIME);
////  debugf("TIMEOUT: %d, %d\r\n", gWIZNETTIME.retry_cnt,gWIZNETTIME.time_100us);
////
////	ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
////	ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);
////	// Display Network Information
////  uint8_t tmpstr[6];
////	ctlwizchip(CW_GET_ID,(void*)tmpstr);
////	debugf("\r\n=== %s NET CONF ===\r\n",(char*)tmpstr);
////	debugf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
////    gWIZNETINFO.mac[0],gWIZNETINFO.mac[1],gWIZNETINFO.mac[2],gWIZNETINFO.mac[3],gWIZNETINFO.mac[4],gWIZNETINFO.mac[5]);
////	debugf("SIP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
////	debugf("GAR: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0],gWIZNETINFO.gw[1],gWIZNETINFO.gw[2],gWIZNETINFO.gw[3]);
////	debugf("SUB: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0],gWIZNETINFO.sn[1],gWIZNETINFO.sn[2],gWIZNETINFO.sn[3]);
////	debugf("DNS: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0],gWIZNETINFO.dns[1],gWIZNETINFO.dns[2],gWIZNETINFO.dns[3]);
////	debugf("======================\r\n");
////}
////
////void my_ip_assign(void)
////{
////   getIPfromDHCP(gWIZNETINFO.ip);
////   getGWfromDHCP(gWIZNETINFO.gw);
////   getSNfromDHCP(gWIZNETINFO.sn);
////   getDNSfromDHCP(gWIZNETINFO.dns);
////   gWIZNETINFO.dhcp = NETINFO_DHCP;
////   /* Network initialization */
////   network_init(); // apply from dhcp
////   debugf("DHCP LEASED TIME : %d Sec.\r\n", getDHCPLeasetime());
////}
////
////void my_ip_conflict(void)
////{
////	debugf("CONFLICT IP from DHCP\r\n");
////	//halt or reset or any...
////	while(1); // this example is halt.
////}
//
////int8_t W5500_Init() {
////
////	uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
////
//////	wiz_NetInfo net_info = {
//////		.mac  = { 0xEA, 0x11, 0x22, 0x33, 0x44, 0xEA },
//////		.ip = {192, 168, 1, 200},
//////		.sn = {255, 255, 255, 0},
//////		.gw = {192., 168, 1, 1},
//////		.dns = {8, 8, 8, 8},
//////		.dhcp = NETINFO_STATIC
//////	};
////
////	wiz_NetInfo net_info = {
////		.mac  = { 0xEA, 0x11, 0x22, 0x33, 0x44, 0xEA },
////		.dhcp = NETINFO_DHCP
////	};
////
////	uint8_t buff[DATA_BUF_SIZE];
////	uint8_t sendbuf[1024]= {""};
////
////	HAL_Delay(500);
////
////	W5500_Unselect();
////	W5500_Reset();
////
////    reg_wizchip_cs_cbfunc(W5500_Select, W5500_Unselect);
////    reg_wizchip_spi_cbfunc(W5500_ReadByte, W5500_WriteByte);
////    reg_wizchip_spiburst_cbfunc(W5500_ReadBuff, W5500_WriteBuff);
////
//////    /* WIZCHIP SOCKET Buffer initialize */
//////	if (ctlwizchip(0, (void*)memsize) == -1){
////////		printf("WIZCHIP Initialized fail.\r\n");
//////		return -1;
//////	} else {
//////		return 0;
//////	}
////    Load_Net_Parameters();
////
////    if(ctlwizchip(0, (void*)memsize) == -1){
////       debugf("WIZCHIP Initialized fail.\r\n");
////       while(1);
////     }
////
////
////    setSHAR(gWIZNETINFO.mac);
////    DHCP_init(0, buff);
////    reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);
////
////    uint8_t dhcp_ret = DHCP_run();
////    while(dhcp_ret != DHCP_IP_LEASED) {
////    	debugf("Waiting DHCP: %d\r\n", dhcp_ret);
////        HAL_Delay(500);
////      dhcp_ret = DHCP_run();
////    }
////
////    uint8_t ret = 0;
////    while(1) {
//////      if(usart_recv_flag) {
//////        usart_recv_flag = 0;
////        ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);
////        debugf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",gWIZNETINFO.mac[0],gWIZNETINFO.mac[1],gWIZNETINFO.mac[2],\
////        gWIZNETINFO.mac[3],gWIZNETINFO.mac[4],gWIZNETINFO.mac[5]);
////        debugf("SIP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
////        debugf("GAR: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0],gWIZNETINFO.gw[1],gWIZNETINFO.gw[2],gWIZNETINFO.gw[3]);
////        debugf("SUB: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0],gWIZNETINFO.sn[1],gWIZNETINFO.sn[2],gWIZNETINFO.sn[3]);
////        debugf("DNS: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0],gWIZNETINFO.dns[1],gWIZNETINFO.dns[2],gWIZNETINFO.dns[3]);
//////      }
////        HAL_Delay(500);
////
////      /*if(rev) {
////        sprintf((char *)sendbuf,"%d\r\n",rev);
////        send(1, sendbuf, 10);
////        send(1, buf, rev);
////        memset(buf,'\0',rev);
////      }*/
////    }
//////    uint8_t rx_tx_buff_sizes[] = {2, 2, 2, 2, 2, 2, 2, 2};
//////    return wizchip_init(rx_tx_buff_sizes, rx_tx_buff_sizes);
//////
//////    wiz_NetInfo net_info = {
//////        .mac  = { 0xEA, 0x11, 0x22, 0x33, 0x44, 0xEA },
//////        .dhcp = NETINFO_DHCP
//////    };
//////
//////    setSHAR(net_info.mac);
//////    DHCP_init(DHCP_SOCKET, dhcp_buffer);
//////
//////    reg_dhcp_cbfunc(
//////        Callback_IPAssigned,
//////        Callback_IPAssigned,
//////        Callback_IPConflict
//////    );
//////
//////    uint32_t ctr = 10000;
//////    while((!ip_assigned) && (ctr > 0)) {
//////        DHCP_run();
//////        ctr--;
//////    }
//////    if(!ip_assigned) {
//////
//////
//////
//////    	return;
//////    }
//
////}
