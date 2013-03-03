/****************************************************************************************************
* File:		udm.h
*
* Universal Driver Module for SafeNet crypto hardware.
* This header file contains of the all external UDM definitions,
* and is meant to be included by all applications that use the UDM API.
*
*
* Copyright (c) 2001-2005 SafeNet, Inc. All rights reserved.
*
* The source code provided in this file is licensed perpetually and
* royalty-free to the User for exclusive use as sample software to be used in
* developing derivative code based on SafeNet products and technologies. The
* SafeNet source code shall not be redistributed in part or in whole, nor as
* part of any User derivative product containing source code, without the
* express written consent of SafeNet.
*
*
* Edit History:
*
* $Log: udm.h,v $
* Revision 1.1.1.1  2008-12-24 05:35:44  inter_you
* first init
*
* Revision 1.39  2004/10/11 20:23:38  mwood
* 3140 udm driver packet processing support- added 3140 sa record
* Revision 1.38  2004/09/13 18:34:36  mwood
* Initial 3140 udm code - fw load & cdr ring support, pdr inprogress
* Revision 1.37  2004/09/08 15:25:08  imorris
*	Replaced structure packing pragmas with include files.
* Revision 1.36  2004/08/02 17:49:16  imorris
* Revision 1.35  2004/07/01 19:55:55  hsubbaram
*	Added DEVICEINFO_FEATURES_PKCP_MBOX to feature list in order
*	to accomodate safexcel wireless ip.
* Revision 1.34  2004/06/28 21:46:27  hsubbaram
*	Removed some defs added in previous revision.
* Revision 1.33  2004/06/25 15:28:12  hsubbaram
*	Added defs and structs for wireless ip revision 1.
* Revision 1.32  2004/04/14 18:43:17  hsubbaram
*	Added definitions for SSL & TLS pad modes to support EIP94C.
* Revision 1.31  2004/02/06 15:49:52  hsubbaram
*	QA020315: Added arc4 sa record for EIP92B & appended it to union UDM_SA.
* Revision 1.30  2004/01/07 19:39:40  imorris
*	Added minimal support for UDM_DEVICETYPE_TESTSPI3.
*	Corrected UDM_DEVICE_ID_3140 from 0x0073 t0 0x000b.
* Revision 1.29  2003/10/21 15:17:10  imorris
*	Added SA opcodes for MPPE, SSL, and SRTP.
*	Added copy_header to UDM_SA_ARC4_184X and UDM_SA_ARC4_2142.
* Revision 1.28  2003/10/01 17:12:14  imorris
*	Added PE_DMA_CFG_RDR_BUSID_HOST to PE_DMA_CONFIG bit masks (for 184x).
* Revision 1.27  2003/09/30 22:19:58  jriggs
*	MR74 - Removed all references to ecgx, and replaced them with non-ecgx names.
* Revision 1.26  2003/09/25 19:08:28  imorris
*	Added base_addr_mapped to UDM_DEVICEINFO structure.
* Revision 1.25  2003/09/02 22:50:16  imorris
*	Added DEVICEINFO_PKT_OPS_SRTP definition.
* Revision 1.24  2003/08/27 19:58:58  imorris
*	Added aes_ctr_mode to UDM_SA_REV1. 
* Revision 1.23  2003/08/26 15:05:16  imorris
*	Changed UDM_DEVICE_ID_1841 back to 0x000a (for real this time).
* Revision 1.22  2003/08/20 15:25:03  tposavec
*	added #define PE_DMA_CFG_SEQUENTIAL_MODE
* Revision 1.21  2003/08/19 18:55:04  imorris
* Revision 1.20  2003/08/19 13:31:50  imorris
*	Changed UDM_DEVICE_ID_1841 back to 0x000a.
*	Renamed DEVICEINFO_PKT_FEATURES_REDKEY to DEVICEINFO_PKT_FEATURES_BLACKKEY.
* Revision 1.19  2003/08/13 21:00:34  imorris
*	Added support for UDM_DEVICETYPE_EIP94C and UDM_DEVICETYPE_EIP94C_B.
*	In UDM_PKT_BITS, UDM_PD_REV0 and UDM_PD_REV1, renamed chain to chain_sa_cache.
*	In UDM_SA_REV0 and UDM_SA_REV1, renamed arc4_stateful to arc4_load_state.
*	In UDM_SA_REV1, replaced element reserved3 with ij and srec_arc4.
*	In UDM_DEVICEINFO, added new feature fields crypto_algs, crypto_modes, and
*	pkt_features, and added respective bit-mask defines for each of them.
*	Added preliminary support for UDM_DEVICETYPE_3140.
* Revision 1.18  2003/07/17 21:37:45  jlaudani
*	Removed unused _MEM HPI/UDM have/features define per cgx3.20 MR#139.
* Revision 1.17  2003/07/02 20:41:38  imorris
*	Added structure UDM_SA_ARC4_184X.
*	Renamed structure UDM_SA_ARC4 to UDM_SA_ARC4_2142, and cleaned it up.
*	Added prototypes and udm_hash_sync() and udm_crypto_sync().
*	Added UDM_DRVCMD_HASH_SYNC and UDM_DRVCMD_CRYPTO_SYNC definitions.
*	Increased number of parameters in UDM_DRVCMD structure from 6 to 10.
* Revision 1.16  2003/06/28 01:11:42  jlaudani
*	Sync'd the udm device features bits with the hpi device have bits,
*	not the co-processor features bits. 
* Revision 1.15  2003/06/27 03:38:14  jlaudani
*	Changed UDM_DEVICE_ID_1841 from 0x000a to 0x1841, which is the value
*	that presently supports my VSOCK connection.
* Revision 1.14  2003/06/17 12:23:17  jriggs
*	Added UDM_DRVCMD_SET_PTR_CE and UDM_DRVCMD_CLR_PTR_CE,
*	which are required by the WinCE UDM.
* Revision 1.13  2003/05/06 20:32:22  imorris
*	Added support for UDM_DEVICETYPE_EIP92B.
*	Renamed UDM_DEVICETYPE_EIP92 to UDM_DEVICETYPE_EIP92A.
* Revision 1.12  2003/03/06 20:03:22  imorris
*	Removed all ppi_ functions.
*	Added udm_driver_version function.
* Revision 1.11  2003/02/03 19:03:16  imorris
*	Added udm_buf2_xxx functions.
* Revision 1.10  2003/01/29 22:52:52  imorris
*	Fixed screwed up DEVICEINFO_PKT_OPS_xxx values.
* Revision 1.9  2003/01/29 15:44:23  jlaudani
*	Added the follwoing comment: DEVICEINFO_FEATURES_* DEFINES USED BY THE UDM MUST STAY
*	IN SYNC WITH THE HPI_DI_FEATURE_* LIST USED BY THE CO_PROCESSOR LOCATED WITHIN HPI.H.
*	Also added a define that was necessary to keep things in sync.
* Revision 1.8  2003/01/10 18:11:10  imorris
*	Corrected value of UDM_DEVICE_ID_EIP92.
* Revision 1.7  2003/01/09 19:39:34  imorris
*	Added support for UDM_DEVICETYPE_EIP92.
*	Added UDM_GD and UDM_SD structures (scatter/gather particle descriptors).
* Revision 1.6  2003/01/03 19:08:11  jlaudani
* 	Added DEVICEINFO_LAST_FEATURES_BIT_USED to be used by
*	_hpi_translate_DIfeatures_to_HPIhave() function within hpi.c
* Revision 1.5  2002/12/17 00:00:19  jlaudani
* 	Added the new DI feature; DEVICEINFO_FEATURES_PKCP_Q.
*	Also added a comment describing that the function _hpi_translate_DIfeatures_to_HPIhave()
*	must be updated each time a new feature is added to the features list.
* Revision 1.4  2002/12/13 16:56:16  imorris
* Revision 1.3  2002/12/09 19:37:44  imorris
*	Removed UDM_INITBLOCK. Use INIT_BLOCK in initblk.h from now on.
*	Eliminated functions udm_module_init and udm_module_uninit,
*	as they are now handled internally.
*	Fixed UDM_STATE_RECORD_ARC4, size of sbox was wrong.
* Revision 1.2  2002/10/30 20:29:23  jlaudani
*	Including hpi.h inside udm.h was causing a chicken/egg problem.
* Revision 1.1  2002/10/01 21:53:40  imorris
*	Initial revision (moved to CGX project)
* Revision 1.3  2002/10/01 21:53:40  imorris
* Revision 1.2  2002/08/26 18:32:42  imorris
*	Added control element to UDM_FIRMWARE structure for 2142.
* Revision 1.1  2002/08/20 15:58:06  imorris
* Initial revision
* Revision 1.4  2002/08/06 17:47:34  imorris
*	Added support for UDM_DEVICETYPE_1841.
*	Removed ppi_ functions that had equivalent udm_ function counterparts.
* Revision 1.3  2002/07/19 19:38:40  mwood
*	Added comments to facilitate cgx vs udm initblock translation
* Revision 1.2  2002/07/16 22:32:26  imorris
* Revision 1.1  2002/07/03 21:21:06  imorris
* Initial revision
*	Renamed all names containing "cryptic" and "CRYPTIC" to "udm" and "UDM".
*	Renamed this file from cryptic.h to udm.h
*	Eliminated CGX and ECGX (rev 2) command defs from this header file.
*	Added packed structure support for ARM compiler.
*	Updated DEVICEINFO_xxx bit definitions for UDM_DEVICEINFO elements.
*	UDM_INITBLOCK structure is no longer packed.
* Revision 1.18  2002/03/14 19:56:14  imorris
* Revision 1.17  2002/02/06 20:15:36  imorris
*	Added support for UDM_DEVICETYPE_1141.
* Revision 1.16  2002/01/02 22:12:44  imorris
* Revision 1.15  2001/12/05 22:11:52  imorris
*	Created new structure UDM_FIRMWARE.
*	Added udm_firmware element to UDM_INITBLOCK.
* Revision 1.14  2001/10/10 16:11:22  imorris
*	Fixed definition of UDM_BUSID_INTERNAL.
*	Added support for rev 1 SA and state record.
* Revision 1.13  2001/10/02 20:36:42  jshaul
*	changed device definitions to explicitly mention ESP and AH
* Revision 1.12  2001/09/28 15:26:46  imorris
*	Added definitions for 53XX device.
* Revision 1.11  2001/09/06 15:04:26  imorris
* Revision 1.10  2001/08/07 21:32:28  imorris
*	Corrected spelling of MSC_VER to _MSC_VER.
*	Added definitions of ECGX_UPLOAD and ECXG_DOWNLOAD.
* Revision 1.9  2001/07/02 16:05:18  imorris
*	Changed the whole high-level initblock UDM_INITBLOCK.
*	Moved the device initblock definitions from this file to devices.h.
*	Renamed some items to more closely match the documentation.
* Revision 1.8  2001/06/22 17:46:08  imorris
*	Added ECGX 2.x command definitions.
* Revision 1.7  2001/06/11 19:57:42  imorris
*	Added SafeNet copyright and licence notice.
*	Added structure definition UDM_CD_2142 for 2142-style CGX descriptors.
* Revision 1.6  2001/05/25 20:20:04  imorris
*	Changed definition of "callback" element in the UDM_NOTIFY structure.
* Revision 1.5  2001/05/24 22:47:32  imorris
* Revision 1.4  2001/05/15 19:43:00  imorris
* Revision 1.3  2001/05/11 15:09:39  mthoma
* Revision 1.2  2001/05/10 18:21:22  mthoma
*	MAT made changes needed for NT conditionally compiled base on MSC_VER  
* 17apr01 IDM	Completely redefined UDM_PKT structure.
* 17apr01 IDM	Added packet descriptor control elements to UDM_PKT structure.
* 06apr01 IDM	Renamed structure UDM_SA_1140 to UDM_SA, and added 2142 elements to it.
* 30mar01 IDM	Added API functions udm_emi_read() and udm_emi_write().
* 12jan01 IDM	Revamped.
*
****************************************************************************************************/

#ifndef __UDM_H__
#define __UDM_H__

//#include "std.h"
//#include "cgx.h"

/****************************************************************************************************
* Definitions and macros.
****************************************************************************************************/

/* Until I can change the spelling... */
#ifdef CGX_BIG_ENDIAN
	#define UDM_BIG_ENDIAN
#endif

/* The various device types we support. */
#define UDM_DEVICETYPE_UNDEFINED			(-1)
#define UDM_DEVICETYPE_1140					0
#define UDM_DEVICETYPE_2141					1
#define UDM_DEVICETYPE_2142					2
#define UDM_DEVICETYPE_53XX					3		/* TI Avalanche */
#define UDM_DEVICETYPE_1141					4		/* 1141/1741 vers 1.0 */
#define UDM_DEVICETYPE_1841					5
#define UDM_DEVICETYPE_1141V11				6		/* 1141/1741 vers 1.1 */
#define UDM_DEVICETYPE_EIP92A				7		/* Samsung */
#define UDM_DEVICETYPE_EIP92B				8		/* AMD */
#define UDM_DEVICETYPE_EIP94C				11		/* EIP94C full featured */
#define UDM_DEVICETYPE_EIP94C_B				12		/* EIP94C w/o TRNG,PKA */


#define UDM_DEVICETYPE_3140					13
#define UDM_DEVICETYPE_TESTSPI3				14		/* FPGA test board */

/* These are the device numbers for wireless. */
#define UDM_DEVICETYPE_OMAP1610				9		/* TI OMAP 1610 */
/*#define UDM_DEVICETYPE_OMAP710			10 */	/* TI OMAP 710 */
#define UDM_DEVICETYPE_WL_IP_REV1			20		/* Wireless SafeXcel IP, rev 1 */


/* PCI vendor and device IDs for the various device types. */

#define UDM_VENDOR_ID_1140					0x16ae
#define UDM_DEVICE_ID_1140					0x0001
#define UDM_VENDOR_ID_2141					0x11d4
#define UDM_DEVICE_ID_2141					0x2f44
#define UDM_VENDOR_ID_2142					0x11d4
#define UDM_DEVICE_ID_2142					0x2f54
#define UDM_VENDOR_ID_53XX					0x16ae
#define UDM_DEVICE_ID_53XX					0x0043
#define UDM_VENDOR_ID_1141					0x16ae
#define UDM_DEVICE_ID_1141					0x1141
#define UDM_VENDOR_ID_1841					0x16ae
#define UDM_DEVICE_ID_1841					0x000a
#define UDM_VENDOR_ID_1141V11				0x16ae
#define UDM_DEVICE_ID_1141V11				0x1141
#define UDM_VENDOR_ID_EIP92A				0x16ae
#define UDM_DEVICE_ID_EIP92A				0x0092
#define UDM_VENDOR_ID_EIP92B				0x16ae
#define UDM_DEVICE_ID_EIP92B				0x0292
#define UDM_VENDOR_ID_OMAP1610				0x16ac
#define UDM_DEVICE_ID_OMAP1610				0x0001
#define UDM_VENDOR_ID_OMAP710				0x16ac
#define UDM_DEVICE_ID_OMAP710				0x0010
#define UDM_VENDOR_ID_EIP94C				0x16ae
#define UDM_DEVICE_ID_EIP94C				0x0094
#define UDM_VENDOR_ID_EIP94C_B				0x16ae
#define UDM_DEVICE_ID_EIP94C_B				0x0094
#define UDM_VENDOR_ID_3140					0x16ae
#define UDM_DEVICE_ID_3140					0x000b
#define UDM_VENDOR_ID_TESTSPI3				0x16ae
#define UDM_DEVICE_ID_TESTSPI3				0x0073


/* Status return codes for the UDM API function calls. */
#define UDM_DRVSTAT_SUCCESS						0x0000
#define UDM_DRVSTAT_COMMAND_INVALID				0x0001
#define UDM_DRVSTAT_DEVICE_INVALID				0x0002
#define UDM_DRVSTAT_DEVICE_NOT_FOUND			0x0003
#define UDM_DRVSTAT_DEVICE_NOT_INIT				0x0004
#define UDM_DRVSTAT_CDR_FULL					0x0005
#define UDM_DRVSTAT_PDR_FULL					0x0006
#define UDM_DRVSTAT_MALLOC_ERR					0x0007
#define UDM_DRVSTAT_UPLOAD_ERR					0x0008
#define UDM_DRVSTAT_INIT_FAIL					0x0009
#define UDM_DRVSTAT_CDR_EMPTY					0x000a
#define UDM_DRVSTAT_PDR_EMPTY					0x000b
#define UDM_DRVSTAT_GDR_FULL					0x000c
#define UDM_DRVSTAT_IOCTL_ERR					0x000d
#define UDM_DRVSTAT_USERMODE_API_ERR			0x000e
#define UDM_DRVSTAT_BAD_PARAM_CDR_BUSID			0x0100
#define UDM_DRVSTAT_BAD_PARAM_CDR_ENTRIES		0x0101
#define UDM_DRVSTAT_BAD_PARAM_CDR_POLL_DELAY	0x0102
#define UDM_DRVSTAT_BAD_PARAM_CDR_DELAY_AFTER	0x0103
#define UDM_DRVSTAT_BAD_PARAM_CDR_INT_COUNT		0x0104
#define UDM_DRVSTAT_BAD_PARAM_PDR_BUSID			0x0110
#define UDM_DRVSTAT_BAD_PARAM_PDR_ENTRIES		0x0111
#define UDM_DRVSTAT_BAD_PARAM_PDR_POLL_DELAY	0x0112
#define UDM_DRVSTAT_BAD_PARAM_PDR_DELAY_AFTER	0x0113
#define UDM_DRVSTAT_BAD_PARAM_PDR_INT_COUNT		0x0114
#define UDM_DRVSTAT_BAD_PARAM_PDR_OFFSET		0x0115
#define UDM_DRVSTAT_BAD_PARAM_SA_BUSID			0x0120
#define UDM_DRVSTAT_BAD_PARAM_SA_ENTRIES		0x0121
#define UDM_DRVSTAT_BAD_PARAM_SA_CONFIG			0x0122
#define UDM_DRVSTAT_BAD_PARAM_PAR_SRC_BUSID		0x0130
#define UDM_DRVSTAT_BAD_PARAM_PAR_SRC_SIZE		0x0131
#define UDM_DRVSTAT_BAD_PARAM_PAR_DST_BUSID		0x0132
#define UDM_DRVSTAT_BAD_PARAM_PAR_DST_SIZE		0x0133
#define UDM_DRVSTAT_BAD_PARAM_PAR_CONFIG		0x0134
#define UDM_DRVSTAT_BAD_PARAM_OFFSET			0x0140
#define UDM_DRVSTAT_BAD_PARAM_BUSID			0x0141
#define UDM_DRVSTAT_BAD_PARAM_PKA			0x0142
/* 0x0200 through 0x02ff reserved for safezone UDM errors
 * returned by the device.
 * The error code returned is equal to 
 * (error byte returned by device) | UDM_DRVSTAT_WLUDM_ERROR. */
#define UDM_DRVSTAT_WLUDM_ERROR					0x0200
/* 0x0300 through 0x3ff are reserved for general WLUDM errors. */
#define UDM_DRVSTAT_INVALID_ALGORITHM			0x300
#define UDM_DRVSTAT_INVALID_LENGTH				0x301
#define UDM_DRVSTAT_INVALID_HASH_BYTE_COUNT		0x302
#define UDM_DRVSTAT_INVALID_CRYPTO_MODE			0x303
#define UDM_DRVSTAT_INVALID_BYTES_RETURNED		0x304

#define UDM_DRVSTAT_VSOCK_ERROR					0x8000

#define UDM_DRVSTAT_INTERNAL_ERR				0xffff


/* CGX commands (version 1.x). */
/* These don't exist in the cgx.h header file. */
#define CGX_V1_CREATE_IPSEC_CC					0x0100
#define CGX_V1_DELETE_IPSEC_CC					0x0101
#define CGX_V1_UPDATE_IPSEC_CC					0x0102
#define CGX_V1_CREATE_IKE_CC					0x0103
#define CGX_V1_DELETE_IKE_CC					0x0104
#define CGX_V1_UPDATE_IKE_CC					0x0105
#define CGX_V1_DUMP_IKE_CC						0x0106
#define CGX_V1_DUMP_IPSEC_CC					0x0107
#define CGX_V1_DELETE_IKE_CC_LIST				0x0108
#define CGX_V1_DELETE_IPSEC_CC_LIST				0x0109
#define CGX_V1_GEN_PUB_KEY						0x0200
#define CGX_V1_GEN_NEW_PUB_KEY					0x0201
#define CGX_V1_DERIVE_SHARED_SECRET				0x0202
#define CGX_V1_PUBKEY_ENCRYPT					0x0203
#define CGX_V1_PUBKEY_DECRYPT					0x0204
#define CGX_V1_ENCRYPT_DATA						0x0205
#define CGX_V1_DECRYPT_DATA						0x0206
#define CGX_V1_HASH_DATA						0x0207
#define CGX_V1_LOAD_PRECOMPUTES					0x0208
#define CGX_V1_LOAD_CC_IMAGE					0x0209
#define CGX_V1_GEN_SIGN							0x0300
#define CGX_V1_VERIFY_SIGN						0x0301
#define CGX_V1_GEN_SKEYID_DS					0x0302
#define CGX_V1_GEN_SKEYID_PKE					0x0303
#define CGX_V1_GEN_SKEYID_PSK					0x0304
#define CGX_V1_GEN_KEY_MATERIAL					0x0305
#define CGX_V1_DERIVE_IKE_KEY					0x0306
#define CGX_V1_DERIVE_IPSEC_KEY					0x0307
#define CGX_V1_DERIVE_IKE_IV					0x0308
#define CGX_V1_UPDATE_IKE_IV					0x0309
#define CGX_V1_CALC_PRECOMPUTES					0x030A
#define CGX_V1_HMAC_DATA						0x030B
#define CGX_V1_GEN_DH_PUB_KEY					0x030C
#define CGX_V1_LOAD_IPSEC_KEY					0x030D
#define CGX_V1_NOP								0x0400
#define CGX_V1_DOWNLOAD							0x0404
#define CGX_V1_UPLOAD							0x0405


/* Success status code returned from GCX or Packet processing */
/* This is returned in status of UDM_CGX or UDM_PKT. */
/* Note that all other status values are device-dependent. */
#define UDM_SUCCESS								0x0000

/* Bus ID options. */
#define UDM_BUSID_EMI							0x00000000
#define UDM_BUSID_HOST							0x00000001
#define UDM_BUSID_INTERNAL						0x00000002
#define UDM_BUSID_DISABLED						0x80000000

/* Bit masks for packet engine DMA config register (PE_DMA_CONFIG). */
#define PE_DMA_CFG_RESET_PE						0x00000001
#define PE_DMA_CFG_RESET_PDR					0x00000002
#define PE_DMA_CFG_RESET_SCAT_GATH				0x00000004
#define PE_DMA_CFG_ENABLE_FAILSAFE				0x00000008
#define PE_DMA_CFG_PDR_BUSID_HOST				0x00000010
#define PE_DMA_CFG_CDR_BUSID_HOST				0x00000040
#define PE_DMA_CFG_RDR_BUSID_HOST				0x00000040		/* 184x */
#define PE_DMA_CFG_PE_MODE						0x00000100
#define PE_DMA_CFG_SA_PRECEDES_PKT				0x00000200
#define PE_DMA_CFG_PKT_FOLLOWS_DESC				0x00000400
#define PE_DMA_CFG_CGX_LOCKOUT					0x00000800
#define PE_DMA_CFG_GATH_BUSID_HOST				0x00001000
#define PE_DMA_CFG_SCAT_BUSID_HOST				0x00004000
#define PE_DMA_CFG_ENDIAN_SWAP_DESC				0x00010000
#define PE_DMA_CFG_ENDIAN_SWAP_SA				0x00020000
#define PE_DMA_CFG_ENDIAN_SWAP_PKT				0x00040000
#define PE_DMA_CFG_ENDIAN_SWAP_PART				0x00080000
#define PE_DMA_CFG_SUPRESS_PDR_OWN				0x00100000
#define PE_DMA_CFG_SEQUENTIAL_MODE				0x02000000

/* Bit masks for 2141 host interrupt registers (HUSTAT, HMSTAT, HICLR, HIMASK). */
#define HINT_CMD								0x0001
#define HINT_CONTEXT0							0x0002
#define HINT_CONTEXT1							0x0004
#define HINT_DSP								0x0008
#define HINT_HERR								0x0010

/* Bit masks for 2141 H/E Control Register (HECNTL). */
#define HECNTL_ENCRYPT							0x0000
#define HECNTL_DECRYPT							0x0001
#define HECNTL_HASH								0x0002
#define HECNTL_HASH_DECRYPT						0x0005
#define HECNTL_HASH_ENCRYPT						0x0006
#define HECNTL_IDLE								0x0007
#define HECNTL_DES								0x0000
#define HECNTL_TDES								0x0008
#define HECNTL_ECB								0x0000
#define HECNTL_CBC								0x0010
#define HECNTL_OFB								0x0020
#define HECNTL_CFB64							0x0030
#define HECNTL_CFB8								0x0070
#define HECNTL_CFB1								0x00b0
#define HECNTL_MD5								0x0000
#define HECNTL_SHA								0x0100
#define HECNTL_CONSTANTS						0x0200
#define HECNTL_LOAD_COUNT						0x0400
#define HECNTL_OUTER							0x0800
#define HECNTL_FINAL							0x1000
#define HECNTL_CONTEXT1							0x2000
#define HECNTL_OFFSET2							0x4000
#define HECNTL_DECRYPT_KEY						0x8000

/* Bit masks for 2141 H/E Pad Control Register (HEPADCNTL). */
#define HEPADCNTL_ZERO							0x0000
#define HEPADCNTL_IPSEC							0x0001
#define HEPADCNTL_PKCS7							0x0002
#define HEPADCNTL_CONSTANT						0x0003
#define HEPADCNTL_MUTABLE_NONE					0x0000
#define HEPADCNTL_MUTABLE_IPV4					0x0040
#define HEPADCNTL_MUTABLE_IPV6					0x0080

/* Misc. flags and transforms used in the 2141 SA command field. */
#define SA_FLAG_TRUNCATE_ICV					0x01
#define SA_FLAG_CHAIN_IV						0x02
#define SA_FLAG_COPY_IV							0x04
#define SA_FLAG_AH_INBND_ZERO_ICV				0x04
#define SA_FLAG_AH_OUTBND_ZERO_ICV				0x08
#define	SA_XFORM_ENCRYPT						0x00
#define	SA_XFORM_DECRYPT						0x10
#define	SA_XFORM_HASH							0x20
#define	SA_XFORM_HMAC							0x30
#define	SA_XFORM_AH_INBND						0x40
#define	SA_XFORM_AH_OUTBND						0x50
#define	SA_XFORM_ESP_INBND						0x60
#define	SA_XFORM_ESP_OUTBND						0x70
#define	SA_XFORM_ESP_NULL_AUTH_INBND			0x80
#define	SA_XFORM_ESP_NULL_AUTH_OUTBND			0x90
#define	SA_XFORM_ESP_NULL_CRYPTO_INBND			0xa0
#define	SA_XFORM_ESP_NULL_CRYPTO_OUTBND			0xb0
#define	SA_XFORM_ENC_HASH						0xc0
#define	SA_XFORM_HASH_DEC						0xd0

/* Values for rev 0 and rev 1 SA elements. */
#define SA_OPCODE_ENCRYPT						0x00000000
#define SA_OPCODE_ENCRYPT_HASH					0x00000001
#define SA_OPCODE_HASH							0x00000003
#define SA_OPCODE_DECRYPT						0x00000008
#define SA_OPCODE_HASH_DECRYPT					0x00000009
#define SA_OPCODE_ESP_OUTBOUND					0x00000010
#define SA_OPCODE_AH_OUTBOUND					0x00000011
#define SA_OPCODE_MPPE_OUTBOUND					0x00000013
#define SA_OPCODE_SSL_OUTBOUND					0x00000014
#define SA_OPCODE_SRTP_OUTBOUND					0x00000017
#define SA_OPCODE_ESP_INBOUND					0x00000018
#define SA_OPCODE_AH_INBOUND					0x00000019
#define SA_OPCODE_MPPE_INBOUND					0x0000001b
#define SA_OPCODE_SSL_INBOUND					0x0000001c
#define SA_OPCODE_SRTP_INBOUND					0x0000001f
#define SA_PAD_IPSEC							0x00000000
#define SA_PAD_PKCS7							0x00000001
#define SA_PAD_CONSTANT							0x00000002
#define SA_PAD_ZERO								0x00000003
#define SA_CRYPTO_DES							0x00000000
#define SA_CRYPTO_TDES							0x00000001
#define SA_CRYPTO_ARC4							0x00000002
#define SA_CRYPTO_AES							0x00000003
#define SA_CRYPTO_NULL							0x0000000f
#define SA_HASH_MD5								0x00000000
#define SA_HASH_SHA1							0x00000001
#define SA_HASH_NULL							0x0000000f
#define SA_COMP_DEFLATE							0x00000000
#define SA_COMP_NULL							0x00000007
#define SA_IV_REUSE								0x00000000
#define SA_IV_INPUT								0x00000001
#define SA_IV_STATE								0x00000002
#define SA_HASH_SA								0x00000000
#define SA_HASH_STATE							0x00000002
#define SA_HASH_NO_LOAD							0x00000003
#define SA_CRYPTO_MODE_ECB						0x00000000
#define SA_CRYPTO_MODE_CBC						0x00000001
#define SA_CRYPTO_MODE_OFB						0x00000002
#define SA_CRYPTO_MODE_CFB						0x00000003
#define SA_CRYPTO_FEEDBACK_64					0x00000000
#define SA_CRYPTO_FEEDBACK_8					0x00000001
#define SA_CRYPTO_FEEDBACK_1					0x00000002

#define UDM_LEN_IV								8			/* length of DES IV (in bytes) */

/* Values for SA caching */
#define UDM_SA_CACHE_DISABLED					0x0
#define UDM_SA_CACHE_ENABLED					0x1

/****************************************************************************************************
* DEVICEINFO object.
* This object is used by the driver function udm_device_info().
*
* device_num
*	Identifies the device number of the device.
*
* device_type
*	Identifies the type of this device.
*	It will be one of the UDM_DEVICETYPE_xxxx definitions.
*
* base_addr
*	Starting address of the device's bus memory space (physical).
*	This is the raw hardware address, reported only for diagnostic and
*	informational purposes. It should not be used by the application.
*
* base_addr_mapped
*	Starting address of the device's bus memory space (mapped/virtual).
*	This is an address that can be used to directly access the device
*	from the user's application. However, not all platforms support this
*	feature in user space. If this value is reported as NULL, this feature
*	is not supported on the platform.
*
* addr_len
*	Size of bus address space for this device, in bytes.
*
* vendor_id
*	The PCI vendor ID from the device's PCI config space (if applicable).
*
* device_id
*	The PCI device ID from the device's PCI config space (if applicable).
*
* features
*	A bitmap of features supported by this device,
*	defined by DEVICEINFO_FEATURES_xxxx.
*
* crypto_algs
*	A bitmap of crypto algorithms supported by this device,
*	defined by DEVICEINFO_CRYPTO_ALGS_xxxx.
*
* crypto_modes
*	A bitmap of crypto modes supported by this device,
*	defined by DEVICEINFO_CRYPTO_MODES_xxxx.
*
* crypto_modes
*	A bitmap of crypto feedback widths supported by this device,
*	defined by DEVICEINFO_CRYPTO_FEEDBACK_xxxx.
*
* hash_algs
*	A bitmap of hash algorithms supported by this device,
*	defined by DEVICEINFO_HASH_ALGS_xxxx.
*
* comp_algs
*	A bitmap of compression algorithms supported by this device,
*	defined by DEVICEINFO_COMP_ALGS_xxxx.
*
* pkt_ops
*	A bitmap of packet operations supported by this device,
*	defined by DEVICEINFO_PKT_OPS_xxxx.
*
* pkt_features
*	A bitmap of packet features supported by this device,
*	defined by DEVICEINFO_PKT_FEATURES_xxxx.
*
****************************************************************************************************/

typedef unsigned int		UINT32;
typedef void *				VPTR;
typedef unsigned short		UINT16;
typedef unsigned char		BYTE;

typedef struct {
	UINT32 device_num;
	UINT32 device_type;				/* one of the UDM_DEVICETYPE_xxxx definitions */
	UINT32 base_addr;				/* base memory address (hardware) */
	VPTR base_addr_mapped;			/* base memory address (virtual/mapped) */
	UINT32 addr_len;				/* size of memory space, in bytes */
	UINT32 vendor_id;				/* PCI vendor id */
	UINT32 device_id;				/* PCI device id */
	UINT32 features;				/* bits defined by DEVICEINFO_FEATURES_xxxx */
	UINT32 crypto_algs;				/* bits defined by DEVICEINFO_CRYPTO_ALGS_xxxx */
	UINT32 crypto_modes;			/* bits defined by DEVICEINFO_CRYPTO_MODES_xxxx */
	UINT32 crypto_feedback;			/* bits defined by DEVICEINFO_CRYPTO_FEEDBACK_xxxx */
	UINT32 crypto_pad;				/* bits defined by DEVICEINFO_PAD_xxxx */
	UINT32 hash_algs;				/* bits defined by DEVICEINFO_HASH_ALGS_xxxx */
	UINT32 comp_algs;				/* bits defined by DEVICEINFO_COMP_ALGS_xxxx */
	UINT32 pkt_ops;					/* bits defined by DEVICEINFO_PKT_OPS_xxxx */
	UINT32 pkt_features;			/* bits defined by DEVICEINFO_PKT_FEATURES_xxxx */
} UDM_DEVICEINFO;


/* ### NOTE ### 
 *	DEVICEINFO_FEATURES_* DEFINES USED BY THE UDM MUST STAY IN SYNC 
 *	WITH THE HPI_DEVICE_HAVE_* LIST USED BY THE HPI
 *	LOCATED WITHIN HPI.H: 
 *	#define	HPI_DEVICE_HAVE_NO_HW			0x00000000L
 *	#define	HPI_DEVICE_HAVE_PE				0x00000001L
 *	#define	HPI_DEVICE_HAVE_HE				0x00000002L
 *	#define	HPI_DEVICE_HAVE_RNG				0x00000004L
 *	#define	HPI_DEVICE_HAVE_PKCP			0x00000008L
 *	#define	HPI_DEVICE_HAVE_KMR				0x00000010L
 *	#define	HPI_DEVICE_HAVE_KCR				0x00000020L
 *	#define	HPI_DEVICE_HAVE_BUS				0x00000040L
 *	#define	HPI_DEVICE_HAVE_PKCP_PKE_EXP	0x00000100L
 *	#define	HPI_DEVICE_HAVE_PKCP_PKE_CRT	0x00000200L
 *	#define HPI_DEVICE_HAVE_PKCP_Q			0x00000400L
 *	#define HPI_DEVICE_HAVE_PKCP_MBOX		0x00000800L
 *	#define HPI_DEVICE_HAVE_LAST_BIT_USED	HPI_DEVICE_HAVE_PKCP_MBOX
 */
 
#define DEVICEINFO_FEATURES_PE				0x00000001	
#define DEVICEINFO_FEATURES_HE				0x00000002	
#define DEVICEINFO_FEATURES_RNG				0x00000004	
#define DEVICEINFO_FEATURES_PKCP			0x00000008	
#define DEVICEINFO_FEATURES_KMR				0x00000010	
#define DEVICEINFO_FEATURES_KCR				0x00000020
#define DEVICEINFO_FEATURES_BUS				0x00000040
#define DEVICEINFO_FEATURES_PKCP_PKE		0x00000100
#define DEVICEINFO_FEATURES_PKCP_PKECRT		0x00000200
#define DEVICEINFO_FEATURES_PKE_Q			0x00000400
#define DEVICEINFO_FEATURES_PKCP_MBOX		0x00000800
#define DEVICEINFO_FEATURES_LAST_BIT_USED	DEVICEINFO_FEATURES_PKCP_MBOX

#define DEVICEINFO_CRYPTO_ALGS_DES			0x00000001	/* SINGLE-DES encryption algorithm supported */
#define DEVICEINFO_CRYPTO_ALGS_TDES			0x00000002	/* TRIPLE-DES encryption algorithm supported */
#define DEVICEINFO_CRYPTO_ALGS_AES			0x00000004	/* AES encryption algorithm supported */
#define DEVICEINFO_CRYPTO_ALGS_ARCFOUR		0x00000008	/* ARCFOUR encryption algorithm supported */
#define DEVICEINFO_CRYPTO_ALGS_RC5			0x00000010	/* RC-5 encryption algorithm supported */
#define DEVICEINFO_CRYPTO_MODES_ECB			0x00000001	/* ECB encryption mode supported */
#define DEVICEINFO_CRYPTO_MODES_CBC			0x00000002	/* CBC encryption mode supported */
#define DEVICEINFO_CRYPTO_MODES_OFB			0x00000004	/* OFB encryption mode supported */
#define DEVICEINFO_CRYPTO_MODES_CFB			0x00000008	/* CFB encryption mode supported */
#define DEVICEINFO_CRYPTO_MODES_CTR			0x00000010	/* CTR encryption mode supported */
#define DEVICEINFO_CRYPTO_MODES_ICM			0x00000020	/* ICM encryption mode supported */
#define DEVICEINFO_CRYPTO_FEEDBACK_1		0x00000001	/* 1-bit encryption feedback supported */
#define DEVICEINFO_CRYPTO_FEEDBACK_8		0x00000002	/* 8-bit encryption feedback supported */
#define DEVICEINFO_CRYPTO_FEEDBACK_64		0x00000004	/* 64-bit encryption feedback supported */
#define DEVICEINFO_CRYPTO_FEEDBACK_128		0x00000008	/* 128-bit encryption feedback supported */
#define DEVICEINFO_CRYPTO_PAD_ZERO			0x00000001	/* zero pad supported */
#define DEVICEINFO_CRYPTO_PAD_CONST			0x00000002	/* constant pad supported */
#define DEVICEINFO_CRYPTO_PAD_IPSEC			0x00000004	/* IPSEC pad supported */
#define DEVICEINFO_CRYPTO_PAD_PKCS7			0x00000008	/* PKCS#7 pad supported */
#define DEVICEINFO_CRYPTO_PAD_SSL			0x00000010	/* SSL pad supported */
#define DEVICEINFO_CRYPTO_PAD_TLS			0x00000020	/* TLS pad supported */
#define DEVICEINFO_HASH_ALGS_SHA1			0x00000001	/* SHA-1 hash algorithm supported */
#define DEVICEINFO_HASH_ALGS_MD5			0x00000002	/* MD-5 hash algorithm supported */
#define DEVICEINFO_HASH_ALGS_MD2			0x00000004	/* MD-2 hash algorithm supported */
#define DEVICEINFO_HASH_ALGS_RIPEMD128		0x00000008	/* RIPEMD-128 hash algorithm supported */
#define DEVICEINFO_HASH_ALGS_RIPEMD160		0x00000010	/* RIPEMD-160 hash algorithm supported */
#define DEVICEINFO_COMP_ALGS_DEFLATE		0x00000001	/* DEFLATE compression algorithm supported */
#define DEVICEINFO_PKT_OPS_ESP				0x00000001	/* ESP transform is supported */
#define DEVICEINFO_PKT_OPS_AH				0x00000002	/* AH transform is supported */
#define DEVICEINFO_PKT_OPS_MPPE				0x00000004	/* MPPE transform is supported */
#define DEVICEINFO_PKT_OPS_SSL				0x00000008	/* SSL transform is supported */
#define DEVICEINFO_PKT_OPS_TLS				0x00000010	/* TLS transform is supported */
#define DEVICEINFO_PKT_OPS_WTLS				0x00000020	/* WTLS transform is supported */
#define DEVICEINFO_PKT_OPS_IPCOMP			0x00000040	/* IPCOMP transform is supported */
#define DEVICEINFO_PKT_OPS_SRTP				0x00000080	/* sRTP transform is supported */
#define DEVICEINFO_PKT_FEATURES_BLACKKEY	0x00000001	/* black keys are supported */
#define DEVICEINFO_PKT_FEATURES_SA_CACHE	0x00000002	/* SA cache is supported */
#define DEVICEINFO_PKT_FEATURES_CHAIN		0x00000004	/* packet chaining is supported */


/****************************************************************************************************
* NOTIFY object.
* Used within the INIT_BLOCK object. These o/s and platform-dependent parameters specify
* the method for notifying the host that a specific event has occurred, e.g., the completed
* processing of a packet or CGX command.
*
*
* process_id
*	The process ID of the process where the signal will be sent.
*
* signal_number
*	Signal number to send for notification to the process identified by process_id.
*	If zero, no signal will be sent.
*
* callback
*	Function to call for notification.
*	If NULL, no callback will be made.
*
****************************************************************************************************/
typedef struct {
	UINT32 process_id;
	UINT32 signal_number;
	void (*callback)(int device_num);
} UDM_NOTIFY;


/****************************************************************************************************
* UDM_FIRMWARE object.
* Used with the INIT_BLOCK object. This provides a way for the application to supply its own
* firmware for the device, overriding the default firmware that is built into the UDM.
* Within this structure are substructures for each device type. Only the substructure
* pertaining to the particular device type is used, and the rest are ignored.
*
* _2141
*	Info for 2141 firmware.
*
*	firmware
*		Binary firmware image, as an array of UINT32.
*		If NULL, the default built-in firmware image will be used.
*
*	firmware_len
*		Length of firmware image, specified in bytes.
*
* _2142
*	Info for 2142 firmware.
*
*	control
*		Bit-mapped control options, as follows:
*
*		UDM_FIRMWARE_CONTROL_SKIP_LOADER
*			If set, do not load	any loader firmware.
*
*		UDM_FIRMWARE_CONTROL_SKIP_FIRMWARE
*			If set, do not load any firmware banks.
*			
*		UDM_FIRMWARE_CONTROL_SKIP_DIRECTBOOT
*			If set, skip the direct boot procedure.
*
*	loader
*		Pointer to binary loader image, as an array of UINT32.
*		If NULL, the default built-in loader image will be used, if required.
*
*	loader_len
*		Length of loader image, specified in bytes.
*
*	firmware_bank
*		Array of UINT32 pointers to binary firmware image banks. For each
*		bank, the actual firmware image is preceeded by two UINT32 values
*		of length info. If the first pointer (firmware_bank[0]) is NULL,
*		the built-in firmware banks will be used, if required. If the number
*		of specified firmware banks is less than NR_FIRMWARE_BANKS_2142,
*		the remaining firmware_bank pointers should be set to NULL.
*
****************************************************************************************************/
#define UDM_FIRMWARE_CONTROL_SKIP_LOADER		0x00000001
#define UDM_FIRMWARE_CONTROL_SKIP_FIRMWARE		0x00000002
#define UDM_FIRMWARE_CONTROL_SKIP_DIRECTBOOT	0x00000004
#define NR_FIRMWARE_BANKS_2142					10
typedef struct {
	struct {
		UINT32 *firmware;
		UINT32 firmware_len;
	} _2141;
	struct {
		UINT32 control;
		UINT32 *loader;
		UINT32 loader_len;
		UINT32 *firmware_bank[NR_FIRMWARE_BANKS_2142];
	} _2142;
	struct {
		UINT32 *firmware;
		UINT32 firmware_len;
	} _3x40;	
} UDM_FIRMWARE;

#if 0
/****************************************************************************************************
* CGX object.
* Used by the driver functions udm_cgx_xxx().
****************************************************************************************************/

typedef struct {
	int cmd;								/* CGX command */
	volatile UINT32 arg[CGX_MAX_ARGS];		/* CGX argument list */
	volatile UINT32 status;					/* status result of this command, after processing */
	VPTR user_handle;						/* free-form user data, not touched by UDM */
} UDM_CGX;
#endif

/****************************************************************************************************
* PKT object.
* Used by the driver functions udm_pkt_xxx().
****************************************************************************************************/

typedef struct {
	UINT32 control_status;					/* control, next_header/pad, status, pad_control */
	VPTR src;								/* source pointer */
	VPTR dst;								/* destination pointer */
	VPTR sa;								/* pointer to sa for this packet */
	UINT32 len_control2;					/* length, control2, bypass_offset */
	VPTR user_handle;						/* free-form user data, not touched by UDM */
	VPTR srec;								/* pointer to state record for this packet */
} UDM_PKT;


/* Same as UDM_PKT, but using bitfields for the control words. */
typedef struct {
	UINT32 ready1:1;
	UINT32 done1:1;
	UINT32 load_sa_digests:1;
	UINT32 init_stateful_arc4:1;
	UINT32 hash_final:1;
	UINT32 chain_sa_cache:1;
	UINT32 sa_busid:2;
	UINT32 next_header:8;
	UINT32 status:8;
	UINT32 pad_control:8;
	VPTR src;
	VPTR dst;
	VPTR sa;
	UINT32 len:20;
	UINT32 reserved2:2;
	UINT32 ready2:1;
	UINT32 done2:1;
	UINT32 bypass_offset:8;
	VPTR user_handle;
	VPTR srec;
} UDM_PKT_BITS;

#if 0
/* Turn on structure packing. */
#include "pack_begin.h"
#endif

/* Gather particle descriptor. */
typedef struct {
	UINT32 addr;
	UINT32 ready:1;
	UINT32 done:1;
	UINT32 reserved:14;
	UINT32 len:16;
} UDM_GD;

/* Scatter particle descriptor. */
typedef struct {
	UINT32 addr;
	UINT32 ready:1;
	UINT32 done:1;
	UINT32 reserved:30;
} UDM_SD;


/* Packet Descriptor Ring entry (114x, 214x, et al). */
typedef struct {
	volatile UINT32 ready1:1;
	volatile UINT32 done1:1;
	volatile UINT32 load_sa_digests:1;
	volatile UINT32 init_stateful_arc4:1;
	volatile UINT32 hash_final:1;
	volatile UINT32 chain_sa_cache:1;
	volatile UINT32 sa_busid:2;
	volatile UINT32 next_header:8;
	volatile UINT32 status:8;
	volatile UINT32 pad_control:8;
	volatile UINT32 src;
	volatile UINT32 dst;
	volatile UINT32 sa;
	volatile UINT32 len:20;					/* only 16 LSBs used with 2141 */
	volatile UINT32 reserved2:2;
	volatile UINT32 ready2:1;
	volatile UINT32 done2:1;
	volatile UINT32 bypass_offset:8;
} UDM_PD_REV0;

/* Packet Descriptor Ring entry (new for the 1841). */
typedef struct {
	volatile UINT32 ready1:1;
	volatile UINT32 done1:1;
	volatile UINT32 load_sa_digests:1;
	volatile UINT32 init_stateful_arc4:1;
	volatile UINT32 hash_final:1;
	volatile UINT32 chain_sa_cache:1;
	volatile UINT32 sa_busid:2;
	volatile UINT32 next_header:8;
	volatile UINT32 status:8;
	volatile UINT32 pad_control:8;
	volatile UINT32 src;
	volatile UINT32 dst;
	volatile UINT32 sa;
	volatile UINT32 user;
	volatile UINT32 len:20;
	volatile UINT32 reserved2:2;
	volatile UINT32 ready2:1;
	volatile UINT32 done2:1;
	volatile UINT32 bypass_offset:8;
} UDM_PD_REV1;

/* This is for convenience. */
typedef union {
	UDM_PD_REV0 rev0;
	UDM_PD_REV1 rev1;
} UDM_PDX;

#if 0
/* CGX Descriptor ring entry (2141). */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
#ifdef CGX_BIG_ENDIAN
	volatile UINT32 status:16;
	volatile UINT32 reserved2:8;
	volatile UINT32 reserved1:4;
	volatile UINT32 done:1;
	volatile UINT32 last:1;
	volatile UINT32 first:1;
	volatile UINT32 own:1;
	volatile UINT16 cmd;
	volatile UINT16 dp2;
#else
	volatile UINT32 own:1;
	volatile UINT32 first:1;
	volatile UINT32 last:1;
	volatile UINT32 done:1;
	volatile UINT32 reserved1:4;
	volatile UINT32 reserved2:8;
	volatile UINT32 status:16;
	volatile UINT16 dp2;
	volatile UINT16 cmd;
#endif
	volatile UINT32 arg[CGX_MAX_ARGS];
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_CD_2141;

/* CGX Descriptor ring entry (2142). */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
#ifdef CGX_BIG_ENDIAN
	volatile UINT32 status:16;
	volatile UINT32 dp1:4;
	volatile UINT32 dev_num:4;
	volatile UINT32 use_endian:1;
	volatile UINT32 reserved2:1;
	volatile UINT32 chain:1;
	volatile UINT32 reserved1:3;
	volatile UINT32 done:1;
	volatile UINT32 own:1;
	volatile UINT16 cmd;
	volatile UINT16 dp2;
#else
	volatile UINT32 own:1;
	volatile UINT32 done:1;
	volatile UINT32 reserved1:3;
	volatile UINT32 chain:1;
	volatile UINT32 reserved2:1;
	volatile UINT32 use_endian:1;
	volatile UINT32 dev_num:4;
	volatile UINT32 dp1:4;
	volatile UINT32 status:16;
	volatile UINT16 dp2;
	volatile UINT16 cmd;
#endif
	volatile UINT32 arg[CGX_MAX_ARGS];
	volatile UINT16 endian[CGX_MAX_ARGS];
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_CD_2142;
#endif

#define PACKED_STRUCTURE_ATTRIBUTE_1
#define PACKED_STRUCTURE_ATTRIBUTE_2
#if 1
#if 0
/* This is for convenience only. */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
#ifdef CGX_BIG_ENDIAN
	PACKED_STRUCTURE_ATTRIBUTE_1 union {
		PACKED_STRUCTURE_ATTRIBUTE_1 struct {
			volatile UINT32 status:16;
			volatile UINT32 dp1:4;
			volatile UINT32 dev_num:4;
			volatile UINT32 reserved1:4;
			volatile UINT32 done:1;
			volatile UINT32 last:1;
			volatile UINT32 first:1;
			volatile UINT32 own:1;
		} PACKED_STRUCTURE_ATTRIBUTE_2	_2141;
		PACKED_STRUCTURE_ATTRIBUTE_1 struct {
			volatile UINT32 status:16;
			volatile UINT32 dp1:4;
			volatile UINT32 dev_num:4;
			volatile UINT32 use_endian:1;
			volatile UINT32 reserved2:1;
			volatile UINT32 chain:1;
			volatile UINT32 reserved1:3;
			volatile UINT32 done:1;
			volatile UINT32 own:1;
		} PACKED_STRUCTURE_ATTRIBUTE_2	_2142;
	} PACKED_STRUCTURE_ATTRIBUTE_2 control;
	volatile UINT16 cmd;
	volatile UINT16 dp2;
#else
	PACKED_STRUCTURE_ATTRIBUTE_1 union {
		PACKED_STRUCTURE_ATTRIBUTE_1 struct {
			volatile UINT32 own:1;
			volatile UINT32 first:1;
			volatile UINT32 last:1;
			volatile UINT32 done:1;
			volatile UINT32 reserved1:4;
			volatile UINT32 dev_num:4;
			volatile UINT32 dp1:4;
			volatile UINT32 status:16;
		} PACKED_STRUCTURE_ATTRIBUTE_2	_2141;
		PACKED_STRUCTURE_ATTRIBUTE_1 struct {
			volatile UINT32 own:1;
			volatile UINT32 done:1;
			volatile UINT32 reserved1:3;
			volatile UINT32 chain:1;
			volatile UINT32 reserved2:1;
			volatile UINT32 use_endian:1;
			volatile UINT32 dev_num:4;
			volatile UINT32 dp1:4;
			volatile UINT32 status:16;
		} PACKED_STRUCTURE_ATTRIBUTE_2	_2142;
	} PACKED_STRUCTURE_ATTRIBUTE_2 control;
	volatile UINT16 dp2;
	volatile UINT16 cmd;
#endif
	volatile UINT32 arg[CGX_MAX_ARGS];
	volatile UINT16 endian[CGX_MAX_ARGS];
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_CD;
#endif

/* State record rev 0 (used with SA rev 0). */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
	volatile BYTE iv[8];				/* saved data iv */
	volatile UINT32 hash_count;			/* saved hash count */
	volatile BYTE inner[20];			/* saved inner hash digest */
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_STATE_RECORD_REV0;

/* State record rev 1 (used with SA rev 1). */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
	volatile BYTE iv[16];				/* saved data iv */
	volatile UINT32 hash_count;			/* saved hash count */
	volatile BYTE inner[20];			/* saved inner hash digest */
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_STATE_RECORD_REV1;

/* State record ARC4. */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
	volatile BYTE sbox[256];			/* sbox data */
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_STATE_RECORD_ARC4;

/* This is for convenience. */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 union {
	UDM_STATE_RECORD_REV0 rev0;
	UDM_STATE_RECORD_REV1 rev1;
	UDM_STATE_RECORD_ARC4 arc4;
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_STATE_RECORD;

/* SA record (rev 0). */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
#ifdef CGX_BIG_ENDIAN
	UINT32 output_scatter:1;
	UINT32 input_gather:1;
	UINT32 save_hash:1;
	UINT32 save_iv:1;
	UINT32 hash_loading:2;
	UINT32 iv_loading:2;
	UINT32 output_busid:2;
	UINT32 input_busid:2;
	UINT32 header_proc:1;
	UINT32 comp_algo:3;
	UINT32 hash_algo:4;
	UINT32 crypto_algo:4;
	UINT32 crypto_pad:2;
	UINT32 opcode:6;
	UINT32 reserved2:1;
	UINT32 arc4_save_state:1;
	UINT32 arc4_load_state:1;
	UINT32 arc4_aes_key_len:5;
	UINT32 offset:8;
	UINT32 rev:1;
	UINT32 reserved1:1;
	UINT32 aes_decrypt_key:1;
	UINT32 hmac:1;
	UINT32 crypto_feedback:2;
	UINT32 crypto_mode:2;
	UINT32 srec_busid:2;
	UINT32 mutable_bits:1;
	UINT32 ipv6:1;
	UINT32 copy_pad:1;
	UINT32 copy_payload:1;
	UINT32 copy_header:1;
	UINT32 use_red_keys:1;
#else
	UINT32 opcode:6;
	UINT32 crypto_pad:2;
	UINT32 crypto_algo:4;
	UINT32 hash_algo:4;
	UINT32 comp_algo:3;
	UINT32 header_proc:1;
	UINT32 input_busid:2;
	UINT32 output_busid:2;
	UINT32 iv_loading:2;
	UINT32 hash_loading:2;
	UINT32 save_iv:1;
	UINT32 save_hash:1;
	UINT32 input_gather:1;
	UINT32 output_scatter:1;
	UINT32 use_red_keys:1;
	UINT32 copy_header:1;
	UINT32 copy_payload:1;
	UINT32 copy_pad:1;
	UINT32 ipv6:1;
	UINT32 mutable_bits:1;
	UINT32 srec_busid:2;
	UINT32 crypto_mode:2;
	UINT32 crypto_feedback:2;
	UINT32 hmac:1;
	UINT32 aes_decrypt_key:1;
	UINT32 reserved1:1;
	UINT32 rev:1;
	UINT32 offset:8;
	UINT32 arc4_aes_key_len:5;
	UINT32 arc4_load_state:1;
	UINT32 arc4_save_state:1;
	UINT32 reserved2:1;
#endif
	BYTE salt[8];						/* salt for black key */
	BYTE key1[8];						/* key 1 for des or triple-des */
	BYTE key2[8];						/* key 2 for triple-des */
	BYTE key3[8];						/* key 3 for triple-des */
	BYTE inner[20];						/* inner precompute for HMAC */
	BYTE outer[20];						/* outer precompute for HMAC */
	UINT32 spi;							/* security parameters index */
	volatile UINT32 seq;				/* sequence number */
	BYTE seq_mask[8];					/* sequence number mask */
	UINT32 cpi_size;
	volatile UINT32 srec;				/* pointer to state record */
	UINT32 reserved3[4];
	volatile UINT32 management0;		/* management field 0 */
	volatile UINT32 management1;		/* management field 1 */
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_SA_REV0;

/* SA record (rev 1). */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
#ifdef CGX_BIG_ENDIAN
	UINT32 output_scatter:1;
	UINT32 input_gather:1;
	UINT32 save_hash:1;
	UINT32 save_iv:1;
	UINT32 hash_loading:2;
	UINT32 iv_loading:2;
	UINT32 output_busid:2;
	UINT32 input_busid:2;
	UINT32 header_proc:1;
	UINT32 comp_algo:3;
	UINT32 hash_algo:4;
	UINT32 crypto_algo:4;
	UINT32 crypto_pad:2;
	UINT32 opcode:6;
	UINT32 aes_ctr_mode:1;
	UINT32 arc4_save_state:1;
	UINT32 arc4_load_state:1;
	UINT32 arc4_aes_key_len:5;
	UINT32 offset:8;
	UINT32 rev:1;
	UINT32 reserved1:1;
	UINT32 aes_decrypt_key:1;
	UINT32 hmac:1;
	UINT32 crypto_feedback:2;
	UINT32 crypto_mode:2;
	UINT32 srec_busid:2;
	UINT32 mutable_bits:1;
	UINT32 ipv6:1;
	UINT32 copy_pad:1;
	UINT32 copy_payload:1;
	UINT32 copy_header:1;
	UINT32 use_red_keys:1;
#else
	UINT32 opcode:6;
	UINT32 crypto_pad:2;
	UINT32 crypto_algo:4;
	UINT32 hash_algo:4;
	UINT32 comp_algo:3;
	UINT32 header_proc:1;
	UINT32 input_busid:2;
	UINT32 output_busid:2;
	UINT32 iv_loading:2;
	UINT32 hash_loading:2;
	UINT32 save_iv:1;
	UINT32 save_hash:1;
	UINT32 input_gather:1;
	UINT32 output_scatter:1;
	UINT32 use_red_keys:1;
	UINT32 copy_header:1;
	UINT32 copy_payload:1;
	UINT32 copy_pad:1;
	UINT32 ipv6:1;
	UINT32 mutable_bits:1;
	UINT32 srec_busid:2;
	UINT32 crypto_mode:2;
	UINT32 crypto_feedback:2;
	UINT32 hmac:1;
	UINT32 aes_decrypt_key:1;
	UINT32 reserved1:1;
	UINT32 rev:1;
	UINT32 offset:8;
	UINT32 arc4_aes_key_len:5;
	UINT32 arc4_load_state:1;
	UINT32 arc4_save_state:1;
	UINT32 aes_ctr_mode:1;
#endif
	BYTE salt[8];						/* salt for black key */
	BYTE key1[8];						/* key 1 for AES or triple-DES or DES */
	BYTE key2[8];						/* key 2 for AES or triple-DES */
	BYTE key3[8];						/* key 3 for AES or triple-DES */
	BYTE key4[8];						/* key 4 for AES */
	BYTE inner[20];						/* inner precompute for HMAC */
	BYTE outer[20];						/* outer precompute for HMAC */
	UINT32 spi;							/* security parameters index */
	volatile UINT32 seq;				/* sequence number */
	BYTE seq_mask[8];					/* sequence number mask */
	UINT32 cpi_size;
	volatile UINT32 srec;				/* pointer to state record */
	UINT32 ij;
	volatile UINT32 srec_arc4;			/* pointer to ARC4 state record */
	volatile UINT32 management0;		/* management field 0 */
	volatile UINT32 management1;		/* management field 1 */
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_SA_REV1;

/* SA record for MPPE/ARC4 (for the 184x devices). */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
#ifdef CGX_BIG_ENDIAN
	UINT32 output_scatter:1;
	UINT32 input_gather:1;
	UINT32 reserved3:6;
	UINT32 output_busid:2;
	UINT32 input_busid:2;
	UINT32 header_proc:1;
	UINT32 reserved2:7;
	UINT32 crypto_algo:4;
	UINT32 reserved1:2;
	UINT32 opcode:6;
	UINT32 reserved6:1;
	UINT32 arc4_save_state:1;
	UINT32 arc4_load_state:1;
	UINT32 key_len:5;
	UINT32 reserved5:10;
	UINT32 aes_decrypt_key:1;
	UINT32 reserved4:11;
	UINT32 copy_header:1;
	UINT32 use_red_keys:1;
#else
	UINT32 opcode:6;
	UINT32 reserved1:2;
	UINT32 crypto_algo:4;
	UINT32 reserved2:7;
	UINT32 header_proc:1;
	UINT32 input_busid:2;
	UINT32 output_busid:2;
	UINT32 reserved3:6;
	UINT32 input_gather:1;
	UINT32 output_scatter:1;
	UINT32 use_red_keys:1;
	UINT32 copy_header:1;
	UINT32 reserved4:11;
	UINT32 aes_decrypt_key:1;
	UINT32 reserved5:10;
	UINT32 key_len:5;
	UINT32 arc4_load_state:1;
	UINT32 arc4_save_state:1;
	UINT32 reserved6:1;
#endif
	BYTE session_key_salt[8];
	BYTE session_key1[8];
	BYTE session_key2[8];
	BYTE start_key1[8];
	BYTE start_key2[8];
	UINT32 reserved7[4];
	UINT32 ij;
	UINT32 reserved8[1];
	BYTE state_salt[8];
	UINT32 mppe_count_header;
	UINT32 reserved9[11];
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_SA_ARC4_184X;

/* SA record for MPPE/ARC4 (for the 2142 device). */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
#ifdef CGX_BIG_ENDIAN
	UINT32 output_scatter:1;
	UINT32 input_gather:1;
	UINT32 reserved3:6;
	UINT32 output_busid:2;
	UINT32 input_busid:2;
	UINT32 header_proc:1;
	UINT32 comp_algo:3;
	UINT32 reserved2:4;
	UINT32 crypto_algo:4;
	UINT32 reserved1:2;
	UINT32 opcode:6;
	UINT32 reserved6:1;
	UINT32 arc4_save_state:1;
	UINT32 arc4_load_state:1;
	UINT32 key_len:5;
	UINT32 reserved5:16;
	UINT32 srec_busid:2;
	UINT32 reserved4:4;
	UINT32 copy_header:1;
	UINT32 use_red_keys:1;
#else
	UINT32 opcode:6;
	UINT32 reserved1:2;
	UINT32 crypto_algo:4;
	UINT32 reserved2:4;
	UINT32 comp_algo:3;
	UINT32 header_proc:1;
	UINT32 input_busid:2;
	UINT32 output_busid:2;
	UINT32 reserved3:6;
	UINT32 input_gather:1;
	UINT32 output_scatter:1;
	UINT32 use_red_keys:1;
	UINT32 copy_header:1;
	UINT32 reserved4:4;
	UINT32 srec_busid:2;
	UINT32 reserved5:16;
	UINT32 key_len:5;
	UINT32 arc4_load_state:1;
	UINT32 arc4_save_state:1;
	UINT32 reserved6:1;
#endif
	BYTE session_key_salt[8];
	BYTE session_key1[8];
	BYTE session_key2[8];
	BYTE session_key3[8];
	BYTE start_key_salt[8];
	BYTE start_key1[8];
	BYTE start_key2[8];
	UINT32 ij;
	BYTE state_salt[8];
	UINT32 srec0;
	UINT32 srec1;
	UINT32 mppe_count_header;
	UINT32 cccp_reset_counter;
	UINT32 reserved7[7];
	UINT32 management0;
	UINT32 management1;
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_SA_ARC4_2142;

/* SA record for ARC4 (for the EIP92B devices). */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
#ifdef CGX_BIG_ENDIAN
	UINT32 reserved3:8;
	UINT32 output_busid:2;
	UINT32 input_busid:2;
	UINT32 reserved2:8;
	UINT32 crypto_algo:4;
	UINT32 reserved1:2;
	UINT32 opcode:6;
	UINT32 reserved5:1;
	UINT32 arc4_save_state:1;
	UINT32 arc4_load_state:1;
	UINT32 key_len:5;
	UINT32 reserved4:24;
#else
	UINT32 opcode:6;
	UINT32 reserved1:2;
	UINT32 crypto_algo:4;
	UINT32 reserved2:8;
	UINT32 input_busid:2;
	UINT32 output_busid:2;
	UINT32 reserved3:8;
	UINT32 reserved4:24;
	UINT32 key_len:5;
	UINT32 arc4_load_state:1;
	UINT32 arc4_save_state:1;
	UINT32 reserved5:1;
#endif
	BYTE reserved6[8];
	BYTE current_key1[8];
	BYTE current_key2[8];
	UINT32 reserved7[4];
	UINT32 ij;
	volatile UINT32 srec_arc4;			/* pointer to ARC4 state record */
	UINT32 reserved8[16];
	volatile UINT32 management0;		/* management field 0 */
	volatile UINT32 management1;		/* management field 1 */
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_SA_ARC4_EIP92B;

/* SA record (sa3x40). */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
#ifdef CGX_BIG_ENDIAN
	UINT32 reservedc0_2:2;	/*output_scatter:1; input_gather:1;*/
	UINT32 save_hash:1;
	UINT32 save_iv:1;
	UINT32 hash_loading:2;
	UINT32 iv_loading:2;
	UINT32 output_busid:2;	/*not used by hw, for udm error checking*/
	UINT32 input_busid:2;	/*not used by hw, for udm error checking*/
	UINT32 header_proc:1;
	UINT32 reservedc0_1:3;	/*comp_algo:3;*/
	UINT32 hash_algo:4;
	UINT32 crypto_algo:4;
	UINT32 crypto_pad:2;
	UINT32 opcode:6;
	/*command 1*/
	UINT32 aes_ctr_mode:1;
	UINT32 reservedc1_5:2; 	/*arc4_save_state:1; arc4_load_state:1;*/
	UINT32 arc4_aes_key_len:5;
	UINT32 offset:8;
	UINT32 reservedc1_4:2;	/*rev:1; reserved1:1;*/
	UINT32 aes_decrypt_key:1;
	UINT32 hmac:1;
	UINT32 reservedc1_3:2;	/*crypto_feedback:2;*/
	UINT32 crypto_mode:2;
	UINT32 reservedc1_2:1;	/*srec_busid:2;*/
	UINT32 ext_seq_no:1;
	UINT32 mutable_bits:1;
	UINT32 ipv6:1;
	UINT32 copy_pad:1;
	UINT32 copy_payload:1;
	UINT32 copy_header:1;
	UINT32 reservedc1_1:1;	/*use_red_keys:1;*/
#else
	/*command 0*/
	UINT32 opcode:6;	
	UINT32 crypto_pad:2;
	UINT32 crypto_algo:4;
	UINT32 hash_algo:4;
	UINT32 reservedc0_1:3;	/*comp_algo:3;*/
	UINT32 header_proc:1;
	UINT32 input_busid:2;	/*not used by hw, for udm error checking*/
	UINT32 output_busid:2;	/*not used by hw, for udm error checking*/
	UINT32 iv_loading:2;
	UINT32 hash_loading:2;
	UINT32 save_iv:1;
	UINT32 save_hash:1;
	UINT32 reservedc0_2:2;	/*input_gather:1; output_scatter:1;*/
	/*command 1*/
	UINT32 reservedc1_1:1;	/*use_red_keys:1;*/
	UINT32 copy_header:1;
	UINT32 copy_payload:1;
	UINT32 copy_pad:1;
	UINT32 ipv6:1;
	UINT32 mutable_bits:1;
	UINT32 ext_seq_no:1;	/*srec_busid:2;*/
	UINT32 reservedc1_2:1;
	UINT32 crypto_mode:2;
	UINT32 reservedc1_3:2;	/*crypto_feedback:2;*/
	UINT32 hmac:1;
	UINT32 aes_decrypt_key:1;
	UINT32 reservedc1_4:2;	/*reserved1:1; rev:1;*/
	UINT32 offset:8;
	UINT32 arc4_aes_key_len:5;
	UINT32 reservedc1_5:2; 	/*arc4_load_state:1; arc4_save_state:1;*/
	UINT32 aes_ctr_mode:1;
#endif
	UINT32 reserved1;	/*BYTE salt[8];*/
	UINT32 aes_ctr_nounce;
	BYTE key1[8];		/* key 1 for AES or triple-DES or DES */
	BYTE key2[8];		/* key 2 for AES or triple-DES */
	BYTE key3[8];		/* key 3 for AES or triple-DES */
	BYTE key4[8];		/* key 4 for AES */
	BYTE inner[20];		/* inner precompute for HMAC */
	BYTE outer[20];		/* outer precompute for HMAC */
	UINT32 spi;		/* security parameters index */
	UINT32 checksum;	/* 3x40 high assurance mode checksum*/
	volatile UINT32 seq[2];	/* sequence number */
	BYTE seq_mask[16];	/* sequence number mask */
	UINT32 reserved2[2];
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_SA_3X40;


/* This is for convenience. */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 union {
	UDM_SA_REV0 rev0;
	UDM_SA_REV1 rev1;
	UDM_SA_ARC4_184X arc4_184x;
	UDM_SA_ARC4_2142 arc4_2142;
	UDM_SA_ARC4_EIP92B arc4_eip92b;
	UDM_SA_3X40 sa3x40;
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_SA;

/* SA record for 2141. Note that this is also referred to in the documentation as the CC (crypto context). */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
#ifdef CGX_BIG_ENDIAN
	UINT16 control;								/* value for H/E control register */
	BYTE misc;									/* filler */
	BYTE cmd;									/* transform opcode and misc flags */
	UINT16 offset;								/* hash/encrypt offset */
	UINT16 pad_control;							/* value for H/E pad control register */
#else
	BYTE cmd;									/* transform opcode and misc flags */
/*	BYTE truncate_icv:1; */
/*	BYTE chain_iv:1; */
/*	BYTE ah_ib_zero_icv_read_iv:1; */
/*	BYTE ah_ob_zero_icv:1; */
/*	BYTE xform:4; */
	BYTE misc;									/* filler */
	UINT16 control;								/* value for H/E control register */
	UINT16 pad_control;							/* value for H/E pad control register */
	UINT16 offset;								/* hash/encrypt offset */
#endif
	BYTE key3[8];								/* key 3 for triple-des */
	BYTE key2[8];								/* key 2 for des or triple-des */
	BYTE key1[8];								/* key 1 for des or triple-des */
	BYTE iv_key[8];								/* iv for black key decryption */
	BYTE iv_data[8];							/* iv for data ciphering */
	BYTE inner[20];								/* inner precompute for HMAC */
	BYTE outer[20];								/* outer precompute for HMAC */
	UINT32 hash_count;							/* starting hash count */
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_SA_2141;


/* Chip info object, returned by the packet engine firmware. */
/* Note that this is a superset of the CGX chipinfo structure. */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
	UINT16 pktengine_rev;							/* packet engine firmware revision number */
	UINT16 pktengine_cksm;							/* packet engine firmware checksum */
	UINT16 hw_vsn;									/* hardware version number */
	UINT16 sw_vsn;									/* software version number */
	UINT16 kcr_max;									/* number of key cache registers */
	UINT16 pcdb[3];									/* the current PCDB bits */
	UINT16 serial_number[10];						/* unique chip serial number */
} PACKED_STRUCTURE_ATTRIBUTE_2	UDM_CHIPINFO_PKTENGINE;


/* expmod command parameters */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
  unsigned int *res;
  unsigned int ressize;
  unsigned int *a;
  unsigned int asize;
  unsigned int *p;
  unsigned int psize;
  unsigned int *m;
  unsigned int msize;
} PACKED_STRUCTURE_ATTRIBUTE_2 EXPMOD_PARAM_BLK;

/* expcrtmod command parameters */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
  unsigned int *res;
  unsigned int ressize;
  unsigned int *a;
  unsigned int asize;
  unsigned int *p;
  unsigned int psize;
  unsigned int *q;
  unsigned int qsize;
  unsigned int *dp;
  unsigned int dpsize;
  unsigned int *dq;
  unsigned int dqsize;
  unsigned int *qinv;
  unsigned int usize;  
} PACKED_STRUCTURE_ATTRIBUTE_2 EXPCRTMOD_PARAM_BLK;

/* Parameter block for the crypto command (all parameters used), also
   for update/final digest and random commands, a subset of the parameters
   is used */
typedef PACKED_STRUCTURE_ATTRIBUTE_1 struct {
  unsigned char *output;
  unsigned int size;
} PACKED_STRUCTURE_ATTRIBUTE_2 RANDOM_PARAM_BLK;

#if 0
/* Turn off structure packing. */
#include "pack_end.h"
#endif

#endif


/****************************************************************************************************
* API function prototypes (direct function calls).
****************************************************************************************************/

struct _INIT_BLOCK;

/* These are all of the UDM API functions. */
int udm_driver_version (UINT32 *vers);
int udm_device_info (int device_num, UDM_DEVICEINFO *info);
int udm_device_init (int device_num, struct _INIT_BLOCK *iblk);
int udm_device_uninit (int device_num);
/* The function udm_get_initblk() is for auto-genaration of initblock */
int udm_get_initblk (int device_num, struct _INIT_BLOCK *iblk, int * psg_flag);
/* The function udm_get_random() is for auto generation of a random number :- From safenetssl.c ! */
#if 0
int udm_get_random(int device_num, RANDOM_PARAM_BLK * arg);
/* The function below is for a^p mod m (modular exponentiation) */
int udm_expmod (int device_num, EXPMOD_PARAM_BLK * arg);
/* The function below is for a^p mod m with crt */
int udm_expcrtmod(int device_num, EXPCRTMOD_PARAM_BLK * arg);
#endif
#if 0
int udm_cgx_put (int device_num, UDM_CGX *cgx);
int udm_cgx_get (int device_num, UDM_CGX *cgx);
int udm_cgx_sync (int device_num, UDM_CGX *cgx);
#endif
int udm_cgx_ready (UINT32 *ready);
int udm_pkt_put (int device_num, UDM_PKT pkt[], UINT32 *cnt);
int udm_pkt_get (int device_num, UDM_PKT pkt[], UINT32 *cnt);
int udm_pkt_sync (int device_num, UDM_PKT *pkt);
int udm_pkt_ready (UINT32 *ready);
int udm_bus_read (int device_num, void *buf, int offset, int len);
int udm_bus_write (int device_num, void *buf, int offset, int len);
int udm_buf_alloc (int device_num, void **buf, int len);
int udm_buf_copy_in (int device_num, void *buf, int offset, int len);
int udm_buf_copy_out (int device_num, void *buf, int offset, int len);
int udm_buf2_alloc (int device_num, void **handle, void **addr, int len);
int udm_buf2_free (void *handle);
int udm_buf2_copy_in (void *handle, void *buf, int offset, int len);
int udm_buf2_copy_out (void *handle, void *buf, int offset, int len);
#if 0
int udm_hash_sync (int device_num, hash_cntxt *hc, void *src, int len, int final, int chunk_sz);
int udm_crypto_sync (int device_num, AES_crypto_cntxt *cc, secretkey *tk,
	void *src, int src_len, void *dst, int approximate_dst_len,
	int pad_flag, int pad_const, int enc_dec_flag, int chunk_sz);
#endif
int udm_pcicfg_read (int device_num, int offset, UINT32 *data);
int udm_pcicfg_write (int device_num, int offset, UINT32 data);


/* Additional definitions for Windows NT/2000 platform. */
/* Table of ptrs to driver functions for the kernel-mode interface. */
typedef struct {
	int (*_udm_driver_version) (UINT32 *vers);
	int (*_udm_device_info) (int device_num, UDM_DEVICEINFO *info);
	int (*_udm_device_init) (int device_num, struct _INIT_BLOCK *iblk);
	int (*_udm_device_uninit) (int device_num);
	int (*_udm_get_initblk) (int device_num, struct _INIT_BLOCK *iblk, int * psg_flag);
	#if 0
	int (*_udm_expmod) (int userland, int device_num,EXPMOD_PARAM_BLK * arg);
	int (*_udm_expcrtmod) (int userland, int device_num, EXPCRTMOD_PARAM_BLK * arg);
	int (*_udm_get_random) (int userland, int device_num, RANDOM_PARAM_BLK * arg);
	#endif
#if 0
	int (*_udm_cgx_put) (int device_num, UDM_CGX *cgx);
	int (*_udm_cgx_get) (int device_num, UDM_CGX *cgx);
	int (*_udm_cgx_sync) (int device_num, UDM_CGX *cgx);
#endif
	int (*_udm_cgx_ready) (UINT32 *ready);
	int (*_udm_pkt_put) (int device_num, UDM_PKT pkt[], UINT32 *cnt);
	int (*_udm_pkt_get) (int device_num, UDM_PKT pkt[], UINT32 *cnt);
	int (*_udm_pkt_sync) (int device_num, UDM_PKT *pkt);
	int (*_udm_pkt_ready) (UINT32 *ready);
	int (*_udm_bus_read) (int device_num, void *buf, int offset, int len);
	int (*_udm_bus_write) (int device_num, void *buf, int offset, int len);
	int (*_udm_buf_alloc) (int device_num, void **buf, int len);
	int (*_udm_buf_copy_in) (int device_num, void *buf, int offset, int len);
	int (*_udm_buf_copy_out) (int device_num, void *buf, int offset, int len);
	int (*_udm_buf2_alloc) (int device_num, void **handle, void **addr, int len);
	int (*_udm_buf2_free) (void *handle);
	int (*_udm_buf2_copy_in) (void *handle, void *buf, int offset, int len);
	int (*_udm_buf2_copy_out) (void *handle, void *buf, int offset, int len);
	#if 0
	int (*_udm_hash_sync) (int device_num, hash_cntxt *hc, void *src, int len, int final, int chunk_sz);
	int (*_udm_crypto_sync) (int device_num, AES_crypto_cntxt *cc, secretkey *tk, void *src, int src_len, void *dst, int approximate_dst_len, int pad_flag, int pad_const, int enc_dec_flag, int chunk_sz);
	#endif
	int (*_udm_pcicfg_read) (int device_num, int offset, UINT32 *data);
	int (*_udm_pcicfg_write) (int device_num, int offset, UINT32 data);
} UDM_FCN_TABLE;

int udm_get_fcn_ptrs (UDM_FCN_TABLE *ptr_tbl, int tbl_bytes);

#if defined (WLUDM_MAINTAIN_STATISTICS)

typedef struct WLUDM_STATISTICS_ {
	int crypto_sync_pass;
	int crypto_sync_fail;
	int hash_sync_pass;
	int hash_sync_fail;
	int bus_read_pass;
	int bus_read_fail;
	int bus_write_pass;
	int bus_write_fail;
} WLUDM_STATISTICS;

int udm_reset_statistics (void);
int udm_get_statistics (WLUDM_STATISTICS *stats);

#endif	/* WLUDM_MAINTAIN_STATISTICS */


/****************************************************************************************************
* User-mode API data objects and definitions.
****************************************************************************************************/

/* Commands for the user-mode driver interface (placed in cmd of UDM_DRVCMD structure). */
#define UDM_DRVCMD_DRIVER_VERSION			0x0014
#define UDM_DRVCMD_DEVICE_INFO				0x0000
#define UDM_DRVCMD_DEVICE_INIT				0x0001
#define UDM_DRVCMD_DEVICE_UNINIT			0x0002
#define UDM_DRVCMD_CGX_PUT					0x0003
#define UDM_DRVCMD_CGX_GET					0x0004
#define UDM_DRVCMD_CGX_SYNC					0x0005
#define UDM_DRVCMD_CGX_READY				0x0006
#define UDM_DRVCMD_PKT_PUT					0x0007
#define UDM_DRVCMD_PKT_GET					0x0008
#define UDM_DRVCMD_PKT_SYNC					0x0009
#define UDM_DRVCMD_PKT_READY				0x000a
#define UDM_DRVCMD_BUS_READ					0x000b
#define UDM_DRVCMD_BUS_WRITE				0x000c
#define UDM_DRVCMD_BUF_ALLOC				0x000d
#define UDM_DRVCMD_BUF_COPY_IN				0x000e
#define UDM_DRVCMD_BUF_COPY_OUT				0x000f
#define UDM_DRVCMD_BUF2_ALLOC				0x0010
#define UDM_DRVCMD_BUF2_FREE				0x0011
#define UDM_DRVCMD_BUF2_COPY_IN				0x0012
#define UDM_DRVCMD_BUF2_COPY_OUT			0x0013
#define UDM_DRVCMD_HASH_SYNC				0x0015
#define UDM_DRVCMD_CRYPTO_SYNC				0x0016
#define UDM_DRVCMD_PCICFG_READ				0x0017
#define UDM_DRVCMD_PCICFG_WRITE				0x0018
#define UDM_DRVCMD_GET_PTRS_NT				0x0040
#define UDM_DRVCMD_SET_PTR_CE				0x0041
#define UDM_DRVCMD_CLR_PTR_CE				0x0042
#define UDM_DRVCMD_GET_INITBLK				0x0043
#define UDM_DRVCMD_GET_RND				0x0044
#define UDM_DRVCMD_GET_RAND             0x0045
#define UDM_DRVCMD_EXPMOD               0x0046
#define UDM_DRVCMD_EXPCRTMOD                0x0047


/* This structure is a "wrapper" for all user-mode driver interface calls. */
/* It is not used for the direct function calls. */
typedef struct {
	UINT32 device_num;						/* device number */
	UINT32 cmd;								/* driver command (UDM_DRVCMD_xxxx) */
	VPTR param1;							/* optional parameter #1 */
	VPTR param2;							/* optional parameter #2 */
	VPTR param3;							/* optional parameter #3 */
	VPTR param4;							/* optional parameter #4 */
	VPTR param5;							/* optional parameter #5 */
	VPTR param6;							/* optional parameter #6 */
	VPTR param7;							/* optional parameter #7 */
	VPTR param8;							/* optional parameter #8 */
	VPTR param9;							/* optional parameter #9 */
	VPTR param10;							/* optional parameter #10 */
	UINT32 status;							/* returned status of driver call (UDM_DRVSTAT_xxxx) */
} UDM_DRVCMD;


/* File device number for NT driver interface. */
/* This is a more or less arbitrary value above the range reserved for MS */
/* device drivers - it is supposed to be unique among loaded drivers but */
/* may not matter at all since drivers are opened by name no DEVICE no. */
#define UDM_FILE_DEVICE_NT					0x8401

typedef struct udm_crypt_params {
	int op;             /* encrypt or decrypt*/
	int algo;           /* DES, TDES or AES*/
	int crypto_mode;    /* ECB, CBC*/
	int data_length;
	int key_length;
	int iv_length;
	const char *key;
	const char *input;
	char *output;
	char *iv;
	#if 0
	UDM_SA_REV1 *sa;
	UDM_STATE_RECORD_REV1 *srec;
	#endif 
} UDM_CRYPT_PARAMS;


typedef struct udm_hash_params {
	int algo; /* SHA1, MD5*/
	int data_length;
	int hmac;
	char *hmac_key;
	int key_length;
	char *input;
	char *output;
	int hash_loading;
	int hash_final;
	char *inner_digest;
	#if 0
	UDM_SA_REV1 *sa;
	UDM_STATE_RECORD_REV1 *srec;
	#endif
} UDM_HASH_PARAMS;

#endif /* __UDM_H__ */


