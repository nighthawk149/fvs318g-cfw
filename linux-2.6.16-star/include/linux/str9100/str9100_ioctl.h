/******************************************************************************
 *
 *  Copyright (c) 2008 Cavium Networks 
 * 
 *  This file is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License, Version 2, as 
 *  published by the Free Software Foundation. 
 *
 *  This file is distributed in the hope that it will be useful, 
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 *  NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this file; if not, write to the Free Software 
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 *  visit http://www.gnu.org/licenses/. 
 *
 *  This file may also be available under a different license from Cavium. 
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/


/* ioctl command for communication with Linux Star Gigabit Switch driver */
typedef enum _str9100_ioctl_cmd_t
{
	STR9100_IOCTL_QOS_CONTROL,        /* QoS */
} str9100_ioctl_cmd_t;


typedef enum _str9100_ioctl_qos_cmd_t
{
	STR9100_IOCTL_QOS_PORT_PRI,
	STR9100_IOCTL_QOS_TOS_ENABLE, 
	STR9100_IOCTL_QOS_TOS_PRIORITY, 
	STR9100_IOCTL_QOS_UDP_PRI,
	STR9100_IOCTL_QOS_UDP_PRI_EN,
	STR9100_IOCTL_QOS_UDP_DEFINED_PORT,
	STR9100_IOCTL_QOS_VLAN_PRI,
	STR9100_IOCTL_QOS_REGEN_PRI,
	STR9100_IOCTL_QOS_TRAFFIC_CLASS,
	STR9100_IOCTL_QOS_SCHEDULE_Q_WEIGHT,
	STR9100_IOCTL_QOS_SCHEDULE_MODE,
	STR9100_IOCTL_QOS_DEFAULT,
} str9100_ioctl_qos_cmd_t;

/* port map value */
typedef enum _str9100_pmap
{
	STR9100_MAC0_PMAP  = 0x1,  /* port map value for MAC0 port */
	STR9100_MAC1_PMAP  = 0x2,  /* port map value for MAC1 port */
	STR9100_CPU_PMAP   = 0x4   /* port map value for CPU port */
} str9100_pmap_t;

typedef enum _str9100_qos_traffic_class_t
{
	STR9100_QOS_TRAFFIC_CLASS1,        /* 1 traffic class */
	STR9100_QOS_TRAFFIC_CLASS2,        /* 2 traffic class */
	STR9100_QOS_TRAFFIC_CLASS4     /* 4 traffic class */
} str9100_qos_traffic_class_t;

typedef enum _str9100_qos_sch_mode_t
{
	STR9100_QOS_SCH_WRR,           /* Weight Round Robin mode (Q3, Q2, Q1, Q0) */
	STR9100_QOS_SCH_SP,            /* Strict Priority(Q3 > Q2 > Q1 > Q0) */
	STR9100_QOS_SCH_MM         /* Mix mode Q3 > WRR(Q2, Q1, Q0) */
} str9100_qos_sch_mode_t;


typedef enum _str9100_qos_queue_weight_t
{
	STR9100_QOS_QUEUE_WEIGHT_1  = 0,
	STR9100_QOS_QUEUE_WEIGHT_2  = 1,
	STR9100_QOS_QUEUE_WEIGHT_4  = 2,
	STR9100_QOS_QUEUE_WEIGHT_8  = 3,
	STR9100_QOS_QUEUE_WEIGHT_16 = 4,
} str9100_qos_queue_weight_t;

/* ioctl command block definition for QoS control */
typedef struct _str9100_qos_ctl_t
{
	str9100_ioctl_cmd_t cmd;
	str9100_ioctl_qos_cmd_t qos_cmd;

	union {
		/* Port priority is always on*/
		struct {
			char cpu;
			char port1;
			char port0;
		} port_pri;

		/* TOS priority*/
		struct {
			char enable;  /* bit map (bit0:Port0, bit1:port1, bit2:CPU) */
			char tos_num;
			char pri;
		} tos_pri;
		/* UDP priority*/
		struct {
			int priority;
			char enable;  /* bit map (bit0:Port0, bit1:port1, bit2:CPU) */
			int port;
		} udp_pri;
		/* VLAN priority*/
		struct {
			char enable; /* bit map (bit0:Port0, bit1:port1, bit2:CPU) */
		} vlan_pri;

		struct {
			char enable;
		} regen_pri;

		str9100_qos_traffic_class_t classes;   /* traffic class */

		struct {
			char q3_weight;
			char q2_weight;
			char q1_weight;
			char q0_weight;
			str9100_qos_sch_mode_t sch;        /* queue scheduling mode */
		} schedule;
	} data;
} str9100_qos_ctl;
