/*
 * Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of
 * Cypress Semiconductor Corporation. All Rights Reserved.
 *
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/**
 * @file
 * A small DHCP server.
 * Allows multiple clients.
 *
 * Original source obtained on 8th April 2011 from : Public Domain source - Written by Richard Bronson - http://sourceforge.net/projects/sedhcp/
 * Heavily modified
 *
 */


#include "lwip/sockets.h"  /* equivalent of <sys/socket.h> */
#include <string.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define DHCP_STACK_SIZE               (800)

/* BOOTP operations */
#define BOOTP_OP_REQUEST                (1)
#define BOOTP_OP_REPLY                  (2)

/* DHCP commands */
#define DHCPDISCOVER                    (1)
#define DHCPOFFER                       (2)
#define DHCPREQUEST                     (3)
#define DHCPDECLINE                     (4)
#define DHCPACK                         (5)
#define DHCPNAK                         (6)
#define DHCPRELEASE                     (7)
#define DHCPINFORM                      (8)

/* UDP port numbers for DHCP server and client */
#define IPPORT_DHCPS                   (67)
#define IPPORT_DHCPC                   (68)

/* DHCP socket timeout value in milliseconds. Modify this to make thread exiting more responsive */
#define DHCP_SOCKET_TIMEOUT     500



/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/* DHCP data structure */
typedef struct
{
    uint8_t  opcode;                     /* packet opcode type */
    uint8_t  hardware_type;              /* hardware addr type */
    uint8_t  hardware_addr_len;          /* hardware addr length */
    uint8_t  hops;                       /* gateway hops */
    uint32_t transaction_id;             /* transaction ID */
    uint16_t second_elapsed;             /* seconds since boot began */
    uint16_t flags;
    uint8_t  client_ip_addr[4];          /* client IP address */
    uint8_t  your_ip_addr[4];            /* 'your' IP address */
    uint8_t  server_ip_addr[4];          /* server IP address */
    uint8_t  gateway_ip_addr[4];         /* gateway IP address */
    uint8_t  client_hardware_addr[16];   /* client hardware address */
    uint8_t  legacy[192];
    uint8_t  magic[4];
    uint8_t  options[275];               /* options area */
    /* as of RFC2131 it is variable length */
} dhcp_header_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/
static unsigned char * find_option( dhcp_header_t* request, unsigned char option_num );
static void dhcp_thread( void * thread_input );

/******************************************************
 *               Variable Definitions
 ******************************************************/
static char server_ip_addr_option_buff_default[6] = { 54, 4 };

static char             new_ip_addr[4];
static uint16_t         next_available_ip_addr;
static char             subnet_option_buff[]          = { 1, 4, 255, 255, 255, 0 };
static char             server_ip_addr_option_buff[6];
static char             dns_server_ip_addr_option_buff[]  = { 54, 4, 8, 8, 8, 8 };
static char             mtu_option_buff[]             = { 26, 2, 1500>>8, 1500&0xff };
static char             dhcp_offer_option_buff[]      = { 53, 1, DHCPOFFER };
static char             dhcp_ack_option_buff[]        = { 53, 1, DHCPACK };
static char             dhcp_nak_option_buff[]        = { 53, 1, DHCPNAK };
static char             lease_time_option_buff[]      = { 51, 4, 0x00, 0x01, 0x51, 0x80 }; /* 1 day lease */
static char             dhcp_magic_cookie[]           = { 0x63, 0x82, 0x53, 0x63 };
static volatile char    dhcp_quit_flag = 0;
static TaskHandle_t     dhcp_thread_handle;
static dhcp_header_t    dhcp_header_buff;

/******************************************************
 *               Function Definitions
 ******************************************************/

void start_dhcp_server( uint32_t local_addr )
{
    char a, b, c, d;
    d = (local_addr >> 24) & 0xFF;
    c = (local_addr >> 16) & 0xFF;
    b = (local_addr >> 8) & 0xFF;
    a = (local_addr) & 0xFF;
    new_ip_addr[0] = a;
    new_ip_addr[1] = b;
    new_ip_addr[2] = c;
    new_ip_addr[3] = 0;
    next_available_ip_addr = (c << 8) + 100;
    memcpy(server_ip_addr_option_buff, server_ip_addr_option_buff_default, sizeof(server_ip_addr_option_buff));
    server_ip_addr_option_buff[2] = a;
    server_ip_addr_option_buff[3] = b;
    server_ip_addr_option_buff[4] = c;
    server_ip_addr_option_buff[5] = d;
    xTaskCreate( dhcp_thread, "DHCP thread", DHCP_STACK_SIZE/sizeof( portSTACK_TYPE ), (void*)local_addr, DEFAULT_THREAD_PRIO, &dhcp_thread_handle);
}

void quit_dhcp_server( void )
{
    dhcp_quit_flag = 1;
}

/**
 *  Implements a very simple DHCP server.
 *
 *  Server will always offer next available address to a DISCOVER command
 *  Server will NAK any REQUEST command which is not requesting the next available address
 *  Server will ACK any REQUEST command which is for the next available address, and then increment the next available address
 *
 * @param my_addr : local IP address for binding of server port.
 */

static void dhcp_thread( void * thread_input )
{
    char*                 option_ptr;
    static dhcp_header_t* dhcp_header_ptr = &dhcp_header_buff;
    struct netconn *conn;
    struct netbuf *buf;
    err_t err;
    int   slen;
    char  *mem;
    //uint16_t rxlen;
    ip_addr_t dst_ip = IPADDR4_INIT(0xffffffff);

    conn = netconn_new(NETCONN_UDP);
    if( conn == NULL)
    {
        return;
    }
    netconn_bind(conn, IP_ADDR_ANY, IPPORT_DHCPS);
    conn->recv_timeout = DHCP_SOCKET_TIMEOUT;

#if 0 /* Only for testing of multicast join*/
    {
        #include "lwip/netif.h"

        ip4_addr_t multiaddr;
        IP4_ADDR(&multiaddr, 224, 5, 6, 7);

        err = netconn_join_leave_group(conn, &multiaddr, &netif_default->ip_addr, NETCONN_JOIN);
        if( err != ERR_OK )
        {
            return 1;
        }
    }
#endif

    /* Loop endlessly */
    dhcp_quit_flag = 0;
    while ( dhcp_quit_flag == 0 )
    {
        err = netconn_recv(conn, &buf);
        if (err == ERR_OK)
        {
            /*  no need netconn_connect here, since the netbuf contains the address */
            netbuf_copy(buf,(char *) dhcp_header_ptr, sizeof(dhcp_header_buff));
            netbuf_delete(buf);
            //netbuf_data(buf, &dhcp_header_ptr, &rxlen);

            switch ( dhcp_header_ptr->options[2] )
            {
                case DHCPDISCOVER:
                    {
                        /* Discover command - send back OFFER response */
                        dhcp_header_ptr->opcode = BOOTP_OP_REPLY;

                        /* Clear the DHCP options list */
                        memset( &dhcp_header_ptr->options, 0, sizeof( dhcp_header_ptr->options ) );

                        /* Create the IP address for the Offer */
                        new_ip_addr[3] = next_available_ip_addr & 0xff;
                        memcpy( &dhcp_header_ptr->your_ip_addr, new_ip_addr, 4 );

                        /* Copy the magic DHCP number */
                        memcpy( dhcp_header_ptr->magic, dhcp_magic_cookie, 4 );

                        /* Add options */
                        option_ptr = (char *) &dhcp_header_ptr->options;
                        memcpy( option_ptr, dhcp_offer_option_buff, 3 );       /* DHCP message type */
                        option_ptr += 3;
                        memcpy( option_ptr, server_ip_addr_option_buff, 6 );   /* Server identifier */
                        option_ptr += 6;
                        memcpy( option_ptr, lease_time_option_buff, 6 );       /* Lease Time */
                        option_ptr += 6;
                        memcpy( option_ptr, subnet_option_buff, 6 );           /* Subnet Mask */
                        option_ptr += 6;
                        memcpy( option_ptr, dns_server_ip_addr_option_buff, 6 );   /* DNS server */
                        option_ptr[0] = 6; /* DNS server id */
                        option_ptr += 6;
                        memcpy( option_ptr, mtu_option_buff, 4 );              /* Interface MTU */
                        option_ptr += 4;
                        option_ptr[0] = 0xff; /* end options */
                        option_ptr++;

                        /* Send packet */
                        slen = (option_ptr - (char*)&dhcp_header_buff);
                        buf = netbuf_new();
                        mem = (char *)netbuf_alloc(buf, slen);
                        memcpy(mem, dhcp_header_ptr, slen);
                        err = netconn_sendto(conn, buf,&dst_ip, IPPORT_DHCPC );
                        netbuf_delete(buf);
                    }
                    break;

                case DHCPREQUEST:
                    {
                        /* REQUEST command - send back ACK or NAK */
                        unsigned char* requested_address;
                        uint32_t*      req_addr_ptr;
                        uint32_t*      newip_ptr;
                        //uint32_t*      server_id_req;

                        /* Check that the REQUEST is for this server */

                        dhcp_header_ptr->opcode = BOOTP_OP_REPLY;

                        /* Locate the requested address in the options */
                        requested_address = find_option( dhcp_header_ptr, 50 );

                        /* Copy requested address */
                        memcpy( &dhcp_header_ptr->your_ip_addr, requested_address, 4 );

                        /* Blank options list */
                        memset( &dhcp_header_ptr->options, 0, sizeof( dhcp_header_ptr->options ) );

                        /* Copy DHCP magic number into packet */
                        memcpy( dhcp_header_ptr->magic, dhcp_magic_cookie, 4 );

                        option_ptr = (char *) &dhcp_header_ptr->options;

                        /* Check if Request if for next available IP address */
                        req_addr_ptr = (uint32_t*) dhcp_header_ptr->your_ip_addr;
                        newip_ptr = (uint32_t*) new_ip_addr;
                        if ( *req_addr_ptr != ( ( *newip_ptr & 0x0000ffff ) | ( ( next_available_ip_addr & 0xff ) << 24 ) |
                                                ( ( next_available_ip_addr & 0xff00 ) << 8 ) ) )
                        {
                            /* Request is not for next available IP - force client to take next available IP by sending NAK */
                            /* Add appropriate options */
                            memcpy( option_ptr, dhcp_nak_option_buff, 3 );  /* DHCP message type */
                            option_ptr += 3;
                            memcpy( option_ptr, server_ip_addr_option_buff, 6 ); /* Server identifier */
                            option_ptr += 6;
                            memset( &dhcp_header_ptr->your_ip_addr, 0, sizeof( dhcp_header_ptr->your_ip_addr ) ); /* Clear 'your address' field */
                        }
                        else
                        {
                            /* Request is not for next available IP - force client to take next available IP by sending NAK
                             * Add appropriate options
                             */
                            memcpy( option_ptr, dhcp_ack_option_buff, 3 );       /* DHCP message type */
                            option_ptr += 3;
                            memcpy( option_ptr, server_ip_addr_option_buff, 6 ); /* Server identifier */
                            option_ptr += 6;
                            memcpy( option_ptr, lease_time_option_buff, 6 );     /* Lease Time */
                            option_ptr += 6;
                            memcpy( option_ptr, subnet_option_buff, 6 );         /* Subnet Mask */
                            option_ptr += 6;
                            memcpy( option_ptr, dns_server_ip_addr_option_buff, 6 ); /* DNS server */
                            option_ptr[0] = 6; /* DNS server id */
                            option_ptr += 6;
                            memcpy( option_ptr, mtu_option_buff, 4 );            /* Interface MTU */
                            option_ptr += 4;
                            /* WPRINT_APP_INFO(("Assigned new IP address %d.%d.%d.%d\n",
                                                (uint8_t)new_ip_addr[0], (uint8_t)new_ip_addr[1], 
                                                next_available_ip_addr>>8, next_available_ip_addr&0xff )); */
                            /* Increment IP address */
                            next_available_ip_addr++;
                            if ( ( next_available_ip_addr & 0xff ) == 0xff ) /* Handle low byte rollover */
                            {
                                next_available_ip_addr += 101;
                            }
                            if ( ( next_available_ip_addr >> 8 ) == 0xff ) /* Handle high byte rollover */
                            {
                                next_available_ip_addr += ( 2 << 8 );
                            }
                        }
                        option_ptr[0] = 0xff; /* end options */
                        option_ptr++;

                        /* Send packet */
                        slen = (option_ptr - (char*)&dhcp_header_buff);
                        buf = netbuf_new();
                        mem = (char *)netbuf_alloc(buf, slen);
                        memcpy(mem, dhcp_header_ptr, slen);
                        err = netconn_sendto(conn, buf,&dst_ip, IPPORT_DHCPC );
                        netbuf_delete(buf);
                    }
                    break;

                default:
                    break;
            }
        }
    }

    /* Delete DHCP socket */
    netconn_delete(conn);

    /* Clean up this startup thread */
    vTaskDelete( dhcp_thread_handle );
}


/**
 *  Finds a specified DHCP option
 *
 *  Searches the given DHCP request and returns a pointer to the
 *  specified DHCP option data, or NULL if not found
 *
 * @param request :    The DHCP request structure
 * @param option_num : Which DHCP option number to find
 *
 * @return Pointer to the DHCP option data, or NULL if not found
 */

static unsigned char * find_option( dhcp_header_t* request, unsigned char option_num )
{
    unsigned char* option_ptr = (unsigned char*) request->options;
    while ( ( option_ptr[0] != 0xff ) &&
            ( option_ptr[0] != option_num ) &&
            ( option_ptr < ( (unsigned char*) request ) + sizeof( dhcp_header_t ) ) )
    {
        option_ptr += option_ptr[1] + 2;
    }
    if ( option_ptr[0] == option_num )
    {
        return &option_ptr[2];
    }
    return NULL;

}

