// Copyright (c) 2014 Qualcomm Atheros, Inc.  All rights reserved.
// $ATH_LICENSE_HW_HDR_C$
//
// DO NOT EDIT!  This file is automatically generated
//               These definitions are tied to a particular hardware layout


#ifndef _RATE_TABLE_ENTRY_H_
#define _RATE_TABLE_ENTRY_H_
#if !defined(__ASSEMBLER__)
#endif

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	mprot_required[0], medium_prot_type[3:1], min_mpdu_spacing[13:4], fixed_delim_padding[21:14], reserved[23:22], pcu_data_threshold[27:24], pcu_buf_size[31:28]
//	1-2	struct tx_rate_setting mprot_rate_setting;
//	3-4	struct tx_rate_setting ppdu_rate_setting;
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RATE_TABLE_ENTRY 5

struct rate_table_entry {
    volatile uint32_t mprot_required                  :  1, //[0]
                      medium_prot_type                :  3, //[3:1]
                      min_mpdu_spacing                : 10, //[13:4]
                      fixed_delim_padding             :  8, //[21:14]
                      reserved                        :  2, //[23:22]
                      pcu_data_threshold              :  4, //[27:24]
                      pcu_buf_size                    :  4; //[31:28]
    struct            tx_rate_setting                       mprot_rate_setting;
    struct            tx_rate_setting                       ppdu_rate_setting;
};

/*

mprot_required
			
			Medium protection transmission is required for this PPDU
			transmission <legal all>

medium_prot_type
			
			In case of MU_MIMO transmission, only valid from user0.
			
			
			
			Self Gen Medium Protection type used
			
			0: No protection
			
			1: RTS (legacy)
			
			2: RTS (11ac static bandwidth)
			
			3: RTS (11ac dynamic bandwidth)
			
			4: CTS2Self
			
			5-7: Reserved

min_mpdu_spacing
			
			Minimum number of dwords in an MPDU.  If the actual MPDU
			size is smaller than this, the PCU will insert zero-length
			delimiters after this MPDU to compensate for the difference.
			For example at a PHY rate of 1.3 Gbps if the receiver is
			capable of receiving only 1 MPDU every 16 usec, then this
			field should be set to 649.  This means that if the MPDU
			length is shorter than 2596 bytes then additional zero
			length delimiter and MAC byte padding should be added to get
			to a combined byte count of 2596 bytes.  This does not
			include the non-zero length delimiter.

fixed_delim_padding
			
			A fixed number of zero-length delimiters to add after
			each MPDU.The actual number of zero-length delimiters will
			be maximum of this field and the amount based on the
			min_mpdu_spacing field.

reserved
			
			Generator should set to 0, consumer should ignore. 
			<legal 0>

pcu_data_threshold
			
			The minimum amount of MPDU data present in the TX PCU
			buffer, before TX PCU allows this MPDU data to transfer to
			the PHY. This threshold prevents or reduces data underrun
			conditions during an MPDU transmission. 
			
			If an entire MPDU frame is present in the TX PCU buffer
			that is smaller than this threshold value, this threshold is
			ignored, and MPDU data transfer to the PHY is allowed to
			start.
			
			In units of 256 bytes, except for value 0xF, which means
			that only when the entire frame is present in the TX PCU
			buffer, the transmission is allowed to start.
			
			<legal all>

pcu_buf_size
			
			The minimum TX PCU buffer size set aside for
			transmission to this STA at this rate.
			
			
			
			In a SU-MIMO transmission, the entire PCU buffer size
			will be assigned to this STA and this field is ignored.
			
			 
			
			In a 2 STA MU-MIMO transmission, the SCH takes this
			value, and adds the one from the other STA.
			
			This sum is subtracted from the total TX PCU buffer size
			
			
			
			In a 3 STA MU-MIMO transmission, the SCH does not change
			this value, and forwards it (for user0 and user1) to the TX
			PCU.
			
			
			
			In units of 512 bytes.
			
			<legal all>

struct tx_rate_setting mprot_rate_setting
			
			Field only valid when Mprot_required is set.
			
			Structure containing all the rate information for
			transmitting a medium protection frame at this rate

struct tx_rate_setting ppdu_rate_setting
			
			Field only valid when ppdu_allowed_bw20 is set.
			
			Structure containing all the rate information for
			transmitting the PPDU at this rate
*/


/* Description		RATE_TABLE_ENTRY_0_MPROT_REQUIRED
			
			Medium protection transmission is required for this PPDU
			transmission <legal all>
*/
#define RATE_TABLE_ENTRY_0_MPROT_REQUIRED_OFFSET                     0x00000000
#define RATE_TABLE_ENTRY_0_MPROT_REQUIRED_LSB                        0
#define RATE_TABLE_ENTRY_0_MPROT_REQUIRED_MASK                       0x00000001

/* Description		RATE_TABLE_ENTRY_0_MEDIUM_PROT_TYPE
			
			In case of MU_MIMO transmission, only valid from user0.
			
			
			
			Self Gen Medium Protection type used
			
			0: No protection
			
			1: RTS (legacy)
			
			2: RTS (11ac static bandwidth)
			
			3: RTS (11ac dynamic bandwidth)
			
			4: CTS2Self
			
			5-7: Reserved
*/
#define RATE_TABLE_ENTRY_0_MEDIUM_PROT_TYPE_OFFSET                   0x00000000
#define RATE_TABLE_ENTRY_0_MEDIUM_PROT_TYPE_LSB                      1
#define RATE_TABLE_ENTRY_0_MEDIUM_PROT_TYPE_MASK                     0x0000000e

/* Description		RATE_TABLE_ENTRY_0_MIN_MPDU_SPACING
			
			Minimum number of dwords in an MPDU.  If the actual MPDU
			size is smaller than this, the PCU will insert zero-length
			delimiters after this MPDU to compensate for the difference.
			For example at a PHY rate of 1.3 Gbps if the receiver is
			capable of receiving only 1 MPDU every 16 usec, then this
			field should be set to 649.  This means that if the MPDU
			length is shorter than 2596 bytes then additional zero
			length delimiter and MAC byte padding should be added to get
			to a combined byte count of 2596 bytes.  This does not
			include the non-zero length delimiter.
*/
#define RATE_TABLE_ENTRY_0_MIN_MPDU_SPACING_OFFSET                   0x00000000
#define RATE_TABLE_ENTRY_0_MIN_MPDU_SPACING_LSB                      4
#define RATE_TABLE_ENTRY_0_MIN_MPDU_SPACING_MASK                     0x00003ff0

/* Description		RATE_TABLE_ENTRY_0_FIXED_DELIM_PADDING
			
			A fixed number of zero-length delimiters to add after
			each MPDU.The actual number of zero-length delimiters will
			be maximum of this field and the amount based on the
			min_mpdu_spacing field.
*/
#define RATE_TABLE_ENTRY_0_FIXED_DELIM_PADDING_OFFSET                0x00000000
#define RATE_TABLE_ENTRY_0_FIXED_DELIM_PADDING_LSB                   14
#define RATE_TABLE_ENTRY_0_FIXED_DELIM_PADDING_MASK                  0x003fc000

/* Description		RATE_TABLE_ENTRY_0_RESERVED
			
			Generator should set to 0, consumer should ignore. 
			<legal 0>
*/
#define RATE_TABLE_ENTRY_0_RESERVED_OFFSET                           0x00000000
#define RATE_TABLE_ENTRY_0_RESERVED_LSB                              22
#define RATE_TABLE_ENTRY_0_RESERVED_MASK                             0x00c00000

/* Description		RATE_TABLE_ENTRY_0_PCU_DATA_THRESHOLD
			
			The minimum amount of MPDU data present in the TX PCU
			buffer, before TX PCU allows this MPDU data to transfer to
			the PHY. This threshold prevents or reduces data underrun
			conditions during an MPDU transmission. 
			
			If an entire MPDU frame is present in the TX PCU buffer
			that is smaller than this threshold value, this threshold is
			ignored, and MPDU data transfer to the PHY is allowed to
			start.
			
			In units of 256 bytes, except for value 0xF, which means
			that only when the entire frame is present in the TX PCU
			buffer, the transmission is allowed to start.
			
			<legal all>
*/
#define RATE_TABLE_ENTRY_0_PCU_DATA_THRESHOLD_OFFSET                 0x00000000
#define RATE_TABLE_ENTRY_0_PCU_DATA_THRESHOLD_LSB                    24
#define RATE_TABLE_ENTRY_0_PCU_DATA_THRESHOLD_MASK                   0x0f000000

/* Description		RATE_TABLE_ENTRY_0_PCU_BUF_SIZE
			
			The minimum TX PCU buffer size set aside for
			transmission to this STA at this rate.
			
			
			
			In a SU-MIMO transmission, the entire PCU buffer size
			will be assigned to this STA and this field is ignored.
			
			 
			
			In a 2 STA MU-MIMO transmission, the SCH takes this
			value, and adds the one from the other STA.
			
			This sum is subtracted from the total TX PCU buffer size
			
			
			
			In a 3 STA MU-MIMO transmission, the SCH does not change
			this value, and forwards it (for user0 and user1) to the TX
			PCU.
			
			
			
			In units of 512 bytes.
			
			<legal all>
*/
#define RATE_TABLE_ENTRY_0_PCU_BUF_SIZE_OFFSET                       0x00000000
#define RATE_TABLE_ENTRY_0_PCU_BUF_SIZE_LSB                          28
#define RATE_TABLE_ENTRY_0_PCU_BUF_SIZE_MASK                         0xf0000000
#define RATE_TABLE_ENTRY_1_TX_RATE_SETTING_MPROT_RATE_SETTING_OFFSET 0x00000004
#define RATE_TABLE_ENTRY_1_TX_RATE_SETTING_MPROT_RATE_SETTING_LSB    28
#define RATE_TABLE_ENTRY_1_TX_RATE_SETTING_MPROT_RATE_SETTING_MASK   0xffffffff
#define RATE_TABLE_ENTRY_2_TX_RATE_SETTING_MPROT_RATE_SETTING_OFFSET 0x00000008
#define RATE_TABLE_ENTRY_2_TX_RATE_SETTING_MPROT_RATE_SETTING_LSB    28
#define RATE_TABLE_ENTRY_2_TX_RATE_SETTING_MPROT_RATE_SETTING_MASK   0xffffffff
#define RATE_TABLE_ENTRY_3_TX_RATE_SETTING_PPDU_RATE_SETTING_OFFSET  0x0000000c
#define RATE_TABLE_ENTRY_3_TX_RATE_SETTING_PPDU_RATE_SETTING_LSB     28
#define RATE_TABLE_ENTRY_3_TX_RATE_SETTING_PPDU_RATE_SETTING_MASK    0xffffffff
#define RATE_TABLE_ENTRY_4_TX_RATE_SETTING_PPDU_RATE_SETTING_OFFSET  0x00000010
#define RATE_TABLE_ENTRY_4_TX_RATE_SETTING_PPDU_RATE_SETTING_LSB     28
#define RATE_TABLE_ENTRY_4_TX_RATE_SETTING_PPDU_RATE_SETTING_MASK    0xffffffff


#endif // _RATE_TABLE_ENTRY_H_