/*------------------------------------------------------*/
/* Error code                                           */
/*                                                      */
/* QtError.h                                            */
/*                                                      */
/* Copyright (C) QTEC Inc.                              */
/* All rights reserved                                  */
/*                                                      */
/* Author                                               */
/*      zhubin (zhubin@qtec.cn)                         */
/*                                                      */
/* History                                              */
/*    2017/02/15  Create                                */
/*                                                      */
/*------------------------------------------------------*/

#ifndef QTERROR_H
#define QTERROR_H

/*
 * To add error code to your module, you need to do the following:
 *
 * 1) Add a module offset code.  Add yours to the bottom of the list 
 *    right below this comment, adding 10000.
 *
 * 2) In your module, define a header file which uses addition.  An examples below:
 *
 *    #define QT_ERROR_MYMODULE_MYERROR1 (QT_ERROR_MODULE_MYMODULE + 1)
 *
 */

#define QT_ERROR_MODULE_BASE      10000
#define QT_ERROR_MODULE_NETWORK   20000
#define QT_ERROR_MODULE_DB        30000


#define QT_FAILED(rv) (rv != 0)
#define QT_SUCCEEDED(rv) (rv == 0)

/* Standard "it worked" return value */
#define QT_OK                              0

/* Returned when a function fails */
#define QT_ERROR_FAILURE                   (QtResult)(QT_ERROR_MODULE_BASE + 1)

/* Returned when an instance is not initialized */
#define QT_ERROR_NOT_INITIALIZED           (QtResult)(QT_ERROR_MODULE_BASE + 2)

/* Returned when an instance is already initialized */
#define QT_ERROR_ALREADY_INITIALIZED       (QtResult)(QT_ERROR_MODULE_BASE + 3)

/* Returned by a not implemented function */
#define QT_ERROR_NOT_IMPLEMENTED           (QtResult)(QT_ERROR_MODULE_BASE + 4)

#define QT_ERROR_NULL_POINTER              (QtResult)(QT_ERROR_MODULE_BASE + 5)

/* Returned when an unexpected error occurs */
#define QT_ERROR_UNEXPECTED                (QtResult)(QT_ERROR_MODULE_BASE + 6)

/* Returned when a memory allocation fails */
#define QT_ERROR_OUT_OF_MEMORY             (QtResult)(QT_ERROR_MODULE_BASE + 7)

/* Returned when an illegal value is passed */
#define QT_ERROR_INVALID_ARG               (QtResult)(QT_ERROR_MODULE_BASE + 8)

/* Returned when an operation can't complete due to an unavailable resource */
#define QT_ERROR_NOT_AVAILABLE             (QtResult)(QT_ERROR_MODULE_BASE + 9)

#define QT_ERROR_WOULD_BLOCK               (QtResult)(QT_ERROR_MODULE_BASE + 10)

#define QT_ERROR_NOT_FOUND                 (QtResult)(QT_ERROR_MODULE_BASE + 11)

#define QT_ERROR_FOUND                     (QtResult)(QT_ERROR_MODULE_BASE + 12)

#define QT_ERROR_PARTIAL_DATA              (QtResult)(QT_ERROR_MODULE_BASE + 13)

#define QT_ERROR_TIMEOUT                   (QtResult)(QT_ERROR_MODULE_BASE + 14)

#define QT_ERROR_VOIP_NOSOUCEMCS           (QtResult)(QT_ERROR_MODULE_BASE + 15)

#define QT_ERROR_SELECT_NBRMCS             (QtResult)(QT_ERROR_MODULE_BASE + 16)

#define QT_ERROR_SELECT_DQTCS              (QtResult)(QT_ERROR_MODULE_BASE + 17)

#define QT_ERROR_PACKAGE_DROP              (QtResult)(QT_ERROR_MODULE_BASE + 18)

//for RTSP describe confirm
#define QT_ERROR_CAPACITY_LIMIT			   (QtResult)(QT_ERROR_MODULE_BASE + 19)

#define QT_ERROR_LOAD_CAL				   (QtResult)(QT_ERROR_MODULE_BASE + 20)



#endif // QTERROR_H
