#ifndef __UTILS_H__
#define __UTILS_H__

/* 去掉编译器对木有使用的局部变量的警告 */
#define DISTOR_NOTUSED(V) ((void) V)

/* Distor日志级别 */
#define DISTOR_DEBUG      1
#define DISTOR_INFO       2
#define DISTOR_ERROR      4
#define DISTOR_CRITICAL   8

/* 日志接口 */
void distor_log(int level, char *fmt, ...);

/* 调试接口 */

#endif
