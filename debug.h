/*********************************************************************
*  �÷�
*
* int DEBUG_LEVEL = DEBUG_WRN
*
* distor_debug(err, "debug the info: %s, %d", "this information", 100);
*
**********************************************************************/
#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <stdio.h>

#define DEBUG_ERR   5    /* ������� */
#define DEBUG_WRN   4    /* �澯���� */
#define DEBUG_TRC   3    /* ���ٵ��� */
#define DEBUG_INF   2    /* ��Ϣ���� */
#define DEBUG_OFF   1    /* �رյ��� */

//extern int DEBUG_LEVEL;
#ifndef DEBUG_LEVEL 
#define DEBUG_LEVEL DEBUG_OFF
#endif

#define debug_err(msg...) \
do { \
    if(DEBUG_LEVEL >= DEBUG_ERR) { \
        printf(msg); \
    } \
}while(0);

#define debug_wrn(msg...) \
do { \
    if(DEBUG_LEVEL >= DEBUG_WRN) { \
        printf(msg); \
    } \
}while(0);
   
#define debug_trc(msg...) \
do { \
    if(DEBUG_LEVEL >= DEBUG_TRC) { \
        printf(msg); \
    } \
}while(0);

#define debug_inf(msg...) \
do { \
    if(DEBUG_LEVEL >= DEBUG_INF) { \
        printf(msg); \
    } \
}while(0);

#define distor_debug(level, msg...) debug_##level(msg)

#endif
