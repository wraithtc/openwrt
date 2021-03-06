/*
 * Copyright (c) 2015 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 */

#ifndef _PHY_CHN4_REG_MAP_H_
#define _PHY_CHN4_REG_MAP_H_


#ifndef __PHY_CHN4_REG_MAP_BASE_ADDRESS
#define __PHY_CHN4_REG_MAP_BASE_ADDRESS (0x14400)
#endif


// 0x0 (PHY_BB_DUMMY_CHN_BCAST)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_LSB                                       0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_MSB                                       31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_MASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_GET(x)                                    (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_SET(x)                                    (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_RESET                                     0x0
#define PHY_BB_DUMMY_CHN_BCAST_ADDRESS                                         (0x0 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_RSTMASK                                         0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_RESET                                           0x0

// 0x0 (PHY_BB_DUMMY_CHN_BCAST_0)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_0_LSB                                     0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_0_MSB                                     31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_0_MASK                                    0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_0_GET(x)                                  (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_0_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_0_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_0_SET(x)                                  (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_0_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_0_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_0_RESET                                   0x0
#define PHY_BB_DUMMY_CHN_BCAST_0_ADDRESS                                       (0x0 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_0_RSTMASK                                       0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_0_RESET                                         0x0

// 0x4 (PHY_BB_DUMMY_CHN_BCAST_1)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_1_LSB                                     0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_1_MSB                                     31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_1_MASK                                    0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_1_GET(x)                                  (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_1_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_1_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_1_SET(x)                                  (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_1_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_1_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_1_RESET                                   0x0
#define PHY_BB_DUMMY_CHN_BCAST_1_ADDRESS                                       (0x4 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_1_RSTMASK                                       0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_1_RESET                                         0x0

// 0x8 (PHY_BB_DUMMY_CHN_BCAST_2)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_2_LSB                                     0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_2_MSB                                     31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_2_MASK                                    0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_2_GET(x)                                  (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_2_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_2_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_2_SET(x)                                  (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_2_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_2_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_2_RESET                                   0x0
#define PHY_BB_DUMMY_CHN_BCAST_2_ADDRESS                                       (0x8 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_2_RSTMASK                                       0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_2_RESET                                         0x0

// 0xc (PHY_BB_DUMMY_CHN_BCAST_3)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_3_LSB                                     0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_3_MSB                                     31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_3_MASK                                    0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_3_GET(x)                                  (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_3_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_3_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_3_SET(x)                                  (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_3_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_3_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_3_RESET                                   0x0
#define PHY_BB_DUMMY_CHN_BCAST_3_ADDRESS                                       (0xc + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_3_RSTMASK                                       0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_3_RESET                                         0x0

// 0x10 (PHY_BB_DUMMY_CHN_BCAST_4)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_4_LSB                                     0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_4_MSB                                     31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_4_MASK                                    0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_4_GET(x)                                  (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_4_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_4_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_4_SET(x)                                  (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_4_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_4_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_4_RESET                                   0x0
#define PHY_BB_DUMMY_CHN_BCAST_4_ADDRESS                                       (0x10 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_4_RSTMASK                                       0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_4_RESET                                         0x0

// 0x14 (PHY_BB_DUMMY_CHN_BCAST_5)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_5_LSB                                     0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_5_MSB                                     31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_5_MASK                                    0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_5_GET(x)                                  (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_5_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_5_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_5_SET(x)                                  (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_5_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_5_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_5_RESET                                   0x0
#define PHY_BB_DUMMY_CHN_BCAST_5_ADDRESS                                       (0x14 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_5_RSTMASK                                       0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_5_RESET                                         0x0

// 0x18 (PHY_BB_DUMMY_CHN_BCAST_6)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_6_LSB                                     0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_6_MSB                                     31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_6_MASK                                    0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_6_GET(x)                                  (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_6_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_6_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_6_SET(x)                                  (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_6_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_6_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_6_RESET                                   0x0
#define PHY_BB_DUMMY_CHN_BCAST_6_ADDRESS                                       (0x18 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_6_RSTMASK                                       0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_6_RESET                                         0x0

// 0x1c (PHY_BB_DUMMY_CHN_BCAST_7)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_7_LSB                                     0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_7_MSB                                     31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_7_MASK                                    0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_7_GET(x)                                  (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_7_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_7_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_7_SET(x)                                  (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_7_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_7_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_7_RESET                                   0x0
#define PHY_BB_DUMMY_CHN_BCAST_7_ADDRESS                                       (0x1c + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_7_RSTMASK                                       0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_7_RESET                                         0x0

// 0x20 (PHY_BB_DUMMY_CHN_BCAST_8)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_8_LSB                                     0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_8_MSB                                     31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_8_MASK                                    0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_8_GET(x)                                  (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_8_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_8_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_8_SET(x)                                  (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_8_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_8_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_8_RESET                                   0x0
#define PHY_BB_DUMMY_CHN_BCAST_8_ADDRESS                                       (0x20 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_8_RSTMASK                                       0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_8_RESET                                         0x0

// 0x24 (PHY_BB_DUMMY_CHN_BCAST_9)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_9_LSB                                     0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_9_MSB                                     31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_9_MASK                                    0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_9_GET(x)                                  (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_9_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_9_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_9_SET(x)                                  (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_9_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_9_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_9_RESET                                   0x0
#define PHY_BB_DUMMY_CHN_BCAST_9_ADDRESS                                       (0x24 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_9_RSTMASK                                       0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_9_RESET                                         0x0

// 0x28 (PHY_BB_DUMMY_CHN_BCAST_10)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_10_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_10_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_10_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_10_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_10_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_10_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_10_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_10_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_10_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_10_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_10_ADDRESS                                      (0x28 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_10_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_10_RESET                                        0x0

// 0x2c (PHY_BB_DUMMY_CHN_BCAST_11)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_11_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_11_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_11_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_11_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_11_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_11_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_11_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_11_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_11_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_11_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_11_ADDRESS                                      (0x2c + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_11_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_11_RESET                                        0x0

// 0x30 (PHY_BB_DUMMY_CHN_BCAST_12)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_12_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_12_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_12_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_12_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_12_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_12_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_12_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_12_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_12_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_12_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_12_ADDRESS                                      (0x30 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_12_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_12_RESET                                        0x0

// 0x34 (PHY_BB_DUMMY_CHN_BCAST_13)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_13_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_13_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_13_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_13_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_13_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_13_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_13_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_13_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_13_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_13_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_13_ADDRESS                                      (0x34 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_13_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_13_RESET                                        0x0

// 0x38 (PHY_BB_DUMMY_CHN_BCAST_14)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_14_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_14_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_14_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_14_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_14_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_14_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_14_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_14_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_14_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_14_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_14_ADDRESS                                      (0x38 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_14_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_14_RESET                                        0x0

// 0x3c (PHY_BB_DUMMY_CHN_BCAST_15)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_15_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_15_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_15_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_15_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_15_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_15_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_15_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_15_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_15_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_15_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_15_ADDRESS                                      (0x3c + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_15_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_15_RESET                                        0x0

// 0x40 (PHY_BB_DUMMY_CHN_BCAST_16)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_16_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_16_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_16_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_16_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_16_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_16_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_16_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_16_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_16_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_16_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_16_ADDRESS                                      (0x40 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_16_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_16_RESET                                        0x0

// 0x44 (PHY_BB_DUMMY_CHN_BCAST_17)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_17_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_17_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_17_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_17_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_17_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_17_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_17_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_17_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_17_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_17_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_17_ADDRESS                                      (0x44 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_17_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_17_RESET                                        0x0

// 0x48 (PHY_BB_DUMMY_CHN_BCAST_18)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_18_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_18_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_18_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_18_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_18_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_18_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_18_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_18_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_18_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_18_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_18_ADDRESS                                      (0x48 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_18_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_18_RESET                                        0x0

// 0x4c (PHY_BB_DUMMY_CHN_BCAST_19)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_19_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_19_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_19_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_19_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_19_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_19_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_19_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_19_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_19_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_19_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_19_ADDRESS                                      (0x4c + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_19_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_19_RESET                                        0x0

// 0x50 (PHY_BB_DUMMY_CHN_BCAST_20)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_20_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_20_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_20_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_20_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_20_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_20_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_20_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_20_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_20_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_20_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_20_ADDRESS                                      (0x50 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_20_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_20_RESET                                        0x0

// 0x54 (PHY_BB_DUMMY_CHN_BCAST_21)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_21_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_21_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_21_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_21_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_21_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_21_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_21_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_21_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_21_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_21_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_21_ADDRESS                                      (0x54 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_21_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_21_RESET                                        0x0

// 0x58 (PHY_BB_DUMMY_CHN_BCAST_22)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_22_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_22_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_22_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_22_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_22_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_22_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_22_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_22_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_22_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_22_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_22_ADDRESS                                      (0x58 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_22_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_22_RESET                                        0x0

// 0x5c (PHY_BB_DUMMY_CHN_BCAST_23)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_23_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_23_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_23_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_23_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_23_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_23_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_23_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_23_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_23_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_23_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_23_ADDRESS                                      (0x5c + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_23_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_23_RESET                                        0x0

// 0x60 (PHY_BB_DUMMY_CHN_BCAST_24)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_24_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_24_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_24_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_24_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_24_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_24_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_24_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_24_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_24_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_24_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_24_ADDRESS                                      (0x60 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_24_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_24_RESET                                        0x0

// 0x64 (PHY_BB_DUMMY_CHN_BCAST_25)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_25_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_25_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_25_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_25_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_25_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_25_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_25_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_25_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_25_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_25_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_25_ADDRESS                                      (0x64 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_25_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_25_RESET                                        0x0

// 0x68 (PHY_BB_DUMMY_CHN_BCAST_26)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_26_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_26_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_26_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_26_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_26_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_26_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_26_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_26_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_26_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_26_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_26_ADDRESS                                      (0x68 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_26_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_26_RESET                                        0x0

// 0x6c (PHY_BB_DUMMY_CHN_BCAST_27)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_27_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_27_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_27_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_27_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_27_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_27_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_27_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_27_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_27_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_27_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_27_ADDRESS                                      (0x6c + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_27_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_27_RESET                                        0x0

// 0x70 (PHY_BB_DUMMY_CHN_BCAST_28)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_28_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_28_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_28_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_28_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_28_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_28_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_28_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_28_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_28_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_28_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_28_ADDRESS                                      (0x70 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_28_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_28_RESET                                        0x0

// 0x74 (PHY_BB_DUMMY_CHN_BCAST_29)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_29_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_29_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_29_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_29_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_29_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_29_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_29_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_29_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_29_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_29_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_29_ADDRESS                                      (0x74 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_29_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_29_RESET                                        0x0

// 0x78 (PHY_BB_DUMMY_CHN_BCAST_30)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_30_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_30_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_30_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_30_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_30_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_30_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_30_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_30_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_30_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_30_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_30_ADDRESS                                      (0x78 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_30_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_30_RESET                                        0x0

// 0x7c (PHY_BB_DUMMY_CHN_BCAST_31)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_31_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_31_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_31_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_31_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_31_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_31_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_31_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_31_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_31_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_31_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_31_ADDRESS                                      (0x7c + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_31_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_31_RESET                                        0x0

// 0x80 (PHY_BB_DUMMY_CHN_BCAST_32)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_32_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_32_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_32_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_32_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_32_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_32_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_32_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_32_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_32_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_32_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_32_ADDRESS                                      (0x80 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_32_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_32_RESET                                        0x0

// 0x84 (PHY_BB_DUMMY_CHN_BCAST_33)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_33_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_33_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_33_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_33_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_33_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_33_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_33_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_33_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_33_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_33_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_33_ADDRESS                                      (0x84 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_33_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_33_RESET                                        0x0

// 0x88 (PHY_BB_DUMMY_CHN_BCAST_34)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_34_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_34_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_34_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_34_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_34_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_34_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_34_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_34_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_34_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_34_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_34_ADDRESS                                      (0x88 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_34_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_34_RESET                                        0x0

// 0x8c (PHY_BB_DUMMY_CHN_BCAST_35)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_35_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_35_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_35_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_35_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_35_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_35_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_35_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_35_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_35_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_35_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_35_ADDRESS                                      (0x8c + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_35_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_35_RESET                                        0x0

// 0x90 (PHY_BB_DUMMY_CHN_BCAST_36)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_36_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_36_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_36_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_36_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_36_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_36_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_36_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_36_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_36_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_36_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_36_ADDRESS                                      (0x90 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_36_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_36_RESET                                        0x0

// 0x94 (PHY_BB_DUMMY_CHN_BCAST_37)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_37_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_37_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_37_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_37_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_37_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_37_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_37_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_37_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_37_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_37_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_37_ADDRESS                                      (0x94 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_37_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_37_RESET                                        0x0

// 0x98 (PHY_BB_DUMMY_CHN_BCAST_38)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_38_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_38_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_38_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_38_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_38_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_38_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_38_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_38_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_38_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_38_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_38_ADDRESS                                      (0x98 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_38_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_38_RESET                                        0x0

// 0x9c (PHY_BB_DUMMY_CHN_BCAST_39)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_39_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_39_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_39_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_39_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_39_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_39_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_39_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_39_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_39_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_39_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_39_ADDRESS                                      (0x9c + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_39_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_39_RESET                                        0x0

// 0xa0 (PHY_BB_DUMMY_CHN_BCAST_40)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_40_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_40_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_40_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_40_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_40_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_40_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_40_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_40_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_40_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_40_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_40_ADDRESS                                      (0xa0 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_40_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_40_RESET                                        0x0

// 0xa4 (PHY_BB_DUMMY_CHN_BCAST_41)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_41_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_41_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_41_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_41_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_41_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_41_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_41_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_41_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_41_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_41_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_41_ADDRESS                                      (0xa4 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_41_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_41_RESET                                        0x0

// 0xa8 (PHY_BB_DUMMY_CHN_BCAST_42)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_42_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_42_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_42_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_42_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_42_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_42_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_42_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_42_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_42_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_42_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_42_ADDRESS                                      (0xa8 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_42_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_42_RESET                                        0x0

// 0xac (PHY_BB_DUMMY_CHN_BCAST_43)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_43_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_43_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_43_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_43_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_43_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_43_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_43_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_43_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_43_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_43_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_43_ADDRESS                                      (0xac + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_43_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_43_RESET                                        0x0

// 0xb0 (PHY_BB_DUMMY_CHN_BCAST_44)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_44_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_44_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_44_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_44_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_44_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_44_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_44_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_44_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_44_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_44_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_44_ADDRESS                                      (0xb0 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_44_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_44_RESET                                        0x0

// 0xb4 (PHY_BB_DUMMY_CHN_BCAST_45)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_45_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_45_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_45_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_45_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_45_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_45_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_45_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_45_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_45_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_45_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_45_ADDRESS                                      (0xb4 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_45_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_45_RESET                                        0x0

// 0xb8 (PHY_BB_DUMMY_CHN_BCAST_46)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_46_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_46_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_46_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_46_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_46_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_46_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_46_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_46_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_46_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_46_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_46_ADDRESS                                      (0xb8 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_46_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_46_RESET                                        0x0

// 0xbc (PHY_BB_DUMMY_CHN_BCAST_47)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_47_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_47_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_47_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_47_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_47_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_47_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_47_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_47_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_47_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_47_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_47_ADDRESS                                      (0xbc + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_47_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_47_RESET                                        0x0

// 0xc0 (PHY_BB_DUMMY_CHN_BCAST_48)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_48_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_48_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_48_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_48_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_48_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_48_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_48_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_48_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_48_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_48_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_48_ADDRESS                                      (0xc0 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_48_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_48_RESET                                        0x0

// 0xc4 (PHY_BB_DUMMY_CHN_BCAST_49)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_49_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_49_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_49_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_49_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_49_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_49_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_49_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_49_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_49_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_49_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_49_ADDRESS                                      (0xc4 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_49_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_49_RESET                                        0x0

// 0xc8 (PHY_BB_DUMMY_CHN_BCAST_50)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_50_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_50_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_50_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_50_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_50_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_50_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_50_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_50_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_50_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_50_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_50_ADDRESS                                      (0xc8 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_50_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_50_RESET                                        0x0

// 0xcc (PHY_BB_DUMMY_CHN_BCAST_51)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_51_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_51_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_51_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_51_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_51_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_51_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_51_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_51_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_51_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_51_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_51_ADDRESS                                      (0xcc + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_51_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_51_RESET                                        0x0

// 0xd0 (PHY_BB_DUMMY_CHN_BCAST_52)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_52_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_52_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_52_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_52_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_52_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_52_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_52_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_52_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_52_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_52_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_52_ADDRESS                                      (0xd0 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_52_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_52_RESET                                        0x0

// 0xd4 (PHY_BB_DUMMY_CHN_BCAST_53)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_53_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_53_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_53_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_53_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_53_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_53_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_53_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_53_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_53_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_53_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_53_ADDRESS                                      (0xd4 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_53_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_53_RESET                                        0x0

// 0xd8 (PHY_BB_DUMMY_CHN_BCAST_54)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_54_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_54_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_54_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_54_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_54_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_54_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_54_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_54_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_54_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_54_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_54_ADDRESS                                      (0xd8 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_54_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_54_RESET                                        0x0

// 0xdc (PHY_BB_DUMMY_CHN_BCAST_55)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_55_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_55_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_55_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_55_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_55_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_55_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_55_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_55_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_55_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_55_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_55_ADDRESS                                      (0xdc + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_55_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_55_RESET                                        0x0

// 0xe0 (PHY_BB_DUMMY_CHN_BCAST_56)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_56_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_56_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_56_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_56_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_56_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_56_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_56_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_56_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_56_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_56_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_56_ADDRESS                                      (0xe0 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_56_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_56_RESET                                        0x0

// 0xe4 (PHY_BB_DUMMY_CHN_BCAST_57)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_57_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_57_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_57_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_57_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_57_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_57_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_57_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_57_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_57_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_57_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_57_ADDRESS                                      (0xe4 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_57_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_57_RESET                                        0x0

// 0xe8 (PHY_BB_DUMMY_CHN_BCAST_58)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_58_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_58_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_58_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_58_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_58_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_58_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_58_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_58_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_58_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_58_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_58_ADDRESS                                      (0xe8 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_58_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_58_RESET                                        0x0

// 0xec (PHY_BB_DUMMY_CHN_BCAST_59)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_59_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_59_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_59_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_59_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_59_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_59_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_59_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_59_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_59_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_59_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_59_ADDRESS                                      (0xec + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_59_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_59_RESET                                        0x0

// 0xf0 (PHY_BB_DUMMY_CHN_BCAST_60)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_60_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_60_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_60_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_60_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_60_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_60_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_60_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_60_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_60_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_60_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_60_ADDRESS                                      (0xf0 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_60_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_60_RESET                                        0x0

// 0xf4 (PHY_BB_DUMMY_CHN_BCAST_61)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_61_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_61_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_61_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_61_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_61_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_61_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_61_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_61_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_61_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_61_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_61_ADDRESS                                      (0xf4 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_61_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_61_RESET                                        0x0

// 0xf8 (PHY_BB_DUMMY_CHN_BCAST_62)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_62_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_62_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_62_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_62_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_62_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_62_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_62_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_62_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_62_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_62_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_62_ADDRESS                                      (0xf8 + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_62_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_62_RESET                                        0x0

// 0xfc (PHY_BB_DUMMY_CHN_BCAST_63)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_63_LSB                                    0
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_63_MSB                                    31
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_63_MASK                                   0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_63_GET(x)                                 (((x) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_63_MASK) >> PHY_BB_DUMMY_CHN_BCAST_DUMMY_63_LSB)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_63_SET(x)                                 (((0 | (x)) << PHY_BB_DUMMY_CHN_BCAST_DUMMY_63_LSB) & PHY_BB_DUMMY_CHN_BCAST_DUMMY_63_MASK)
#define PHY_BB_DUMMY_CHN_BCAST_DUMMY_63_RESET                                  0x0
#define PHY_BB_DUMMY_CHN_BCAST_63_ADDRESS                                      (0xfc + __PHY_CHN4_REG_MAP_BASE_ADDRESS)
#define PHY_BB_DUMMY_CHN_BCAST_63_RSTMASK                                      0xffffffff
#define PHY_BB_DUMMY_CHN_BCAST_63_RESET                                        0x0



#endif /* _PHY_CHN4_REG_MAP_H_ */
