#ifndef __UTILS_H__
#define __UTILS_H__

/* ȥ����������ľ��ʹ�õľֲ������ľ��� */
#define DISTOR_NOTUSED(V) ((void) V)

/* Distor��־���� */
#define DISTOR_DEBUG      1
#define DISTOR_INFO       2
#define DISTOR_ERROR      4
#define DISTOR_CRITICAL   8

/* ��־�ӿ� */
void distor_log(int level, char *fmt, ...);

/* ���Խӿ� */

#endif
