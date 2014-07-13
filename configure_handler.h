#ifndef __CONFIGURE_HANDLER__
#define __CONFIGURE_HANDLER__

#include <stdint.h>
#include "mem_pool.h"

#define STRING_ERR -1
#define STRING_OK   1

#define MAX_CONFIG_LINE 256

#define SECTION_LEFT_CHAR  '['
#define SECTION_RIGHT_CHAR ']'
#define ANNOTATION         '#'

typedef struct configure_item_s {
    char *key;
    char *value;
    uint32_t key_len;        /* 查找时先比较长度 */
    uint32_t value_len;
    struct configure_item_s *next;
}configure_item_t;
#define configure_item_size (sizeof(configure_item_t))


typedef struct configure_section_s {
    char *section;
    uint32_t section_len;
    uint32_t spare0;
    configure_item_t *item_head;
    struct configure_section_s *next;
}configure_section_t;
#define configure_section_size (sizeof(configure_section_t))


typedef struct configure_handler_s {
    configure_section_t *section_head;
    configure_section_t *section_tail;
    MemPool *pool;
}configure_handler_t;
#define configure_handler_size (sizeof(configure_section_t))

configure_handler_t *load_configure_file(MemPool *pool, char *filename);
char *get_configure_item(configure_handler_t *config_handler, 
    char *section, char *key);

#endif  /* __CONFIGURE_HANDLER__ */
