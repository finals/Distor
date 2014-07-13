#include <stdio.h>
#include <string.h>
#include "configure_handler.h"

char *string_strip(char *org, char c, uint32_t *real_len)
{
    uint32_t len = strlen(org);
    char *head = org, *tail = org + len - 1;

    if(len <= 0) return NULL;

    while(head <= tail) {
        if(*head == c || *head == '\r' || *head == '\n') {
            ++head;
        }
        else if(*tail == c || *tail == '\r' || *tail == '\n') {
            --tail; 
        }
        else {
            break;
        }
    }

    if(head > tail) return NULL;

    *(tail+1) = '\0';
    if(real_len) 
        *real_len = tail - head + 1;
        
    return head;
}

int8_t string_split_key_value(char *org, char **key, char **value)
{
    char *pos = NULL;

    if(!org) return STRING_ERR;

    pos = strpbrk(org, "=");
    if(!pos) return STRING_ERR;
    *pos = '\0';
    
    *key = org;
    *value = pos + 1;
    return STRING_OK;
}

void configure_item_destroy(MemPool *pool, configure_item_t *item)
{
    if(item) {
        if(item->key) {
            mem_pool_free(pool, item->key);
        }
        if(item->value) {
            mem_pool_free(pool, item->value);
        }
        mem_pool_free(pool, item);
    }
}

configure_item_t *configure_item_create(MemPool *pool, char *line)
{
    char *key, *value;
    configure_item_t *item = 
        (configure_item_t *)mem_pool_alloc(pool, configure_item_size);
    if(!item) goto err;

    if(STRING_ERR == string_split_key_value(line, &key, &value)) goto err;

    key = string_strip(key, ' ', &item->key_len);
    value = string_strip(value, ' ', &item->value_len);
    if(!key || !value) goto err;

    item->key = (char *)mem_pool_alloc(pool, item->key_len);
    item->value = (char *)mem_pool_alloc(pool, item->value_len);
    if(!item->key || !item->value) goto err;

    strncpy(item->key, key, item->key_len);
    strncpy(item->value, value, item->value_len);

    item->next = NULL;

    return item;

err:
    configure_item_destroy(pool, item);
    return NULL;
}

void configure_section_destroy(MemPool *pool, configure_section_t *section)
{
    configure_item_t *item = NULL, *next = NULL;
    
    if(section) {
        if(section->section) {
            mem_pool_free(pool, section->section);
        }
        if(section->item_head) {
            for(item = section->item_head; item; item = next) {
                next = item->next;
                configure_item_destroy(pool, item);
            }
        }
    }
}

configure_section_t *configure_section_create(MemPool *pool, 
    char *section_name, uint32_t section_len)
{
    configure_section_t *section = 
        (configure_section_t *)mem_pool_alloc(pool, configure_section_size);
    if(!section) goto err;

    section->section = (char *)mem_pool_alloc(pool, section_len);
    if(!section->section) goto err;

    section->section_len = section_len;
    strncpy(section->section, section_name, section_len);

    section->item_head = NULL;
    section->next = NULL;

    return section;
err:
    configure_section_destroy(pool, section);
    return NULL;
}

void configure_handler_destroy(MemPool *pool, configure_handler_t *handler)
{
    configure_section_t *section = NULL, *next = NULL;
    if(handler) {
        section = handler->section_head; 
        while(section) {
            next = section->next;
            configure_section_destroy(pool, section);
            section = next;
        }
        mem_pool_free(pool, handler);
    }
} 

configure_handler_t *load_configure_file(MemPool *pool, char *filename)
{
    FILE *config_fp = NULL;
    char line_buffer[MAX_CONFIG_LINE], *p = NULL;
    uint32_t line_len;
    configure_handler_t *config_handler = NULL;
    configure_section_t *cur_section = NULL;
    configure_item_t * item = NULL;

    config_fp = fopen(filename, "r");
    if(!config_fp) {
        goto err;
    }

    config_handler = 
        (configure_handler_t *)mem_pool_alloc(pool, configure_handler_size);
    if(!config_handler) {
        goto err;
    }
    config_handler->pool = pool;

    while(NULL != fgets(line_buffer, MAX_CONFIG_LINE, config_fp)) {
        p = string_strip(line_buffer, ' ', &line_len);
        
        if(!p || *p == ANNOTATION){
            continue;
        }
        else if(*p == SECTION_LEFT_CHAR &&     /* section */
            *(p + line_len - 1) == SECTION_RIGHT_CHAR){
            cur_section = configure_section_create(pool, p, line_len);
            if(!p) {
                goto err;
            }

            /* 链表尾插入 */
            if(config_handler->section_tail) { 
                config_handler->section_tail->next = cur_section;
                config_handler->section_tail = cur_section;
            }
            else {  /* section链表为空 */
                config_handler->section_head = cur_section;
                config_handler->section_tail = cur_section;
            }
        }
        else {   /* item */
            if(!cur_section) goto err;

            item = configure_item_create(pool, p);
            if(!item) {
                continue;
            }
            if(cur_section->item_head) {
                item->next = cur_section->item_head;
                cur_section->item_head = item;
            }
            else {  /* item链表为空 */
                cur_section->item_head = item;
            }
        }
    }

    return config_handler;

err:
    configure_handler_destroy(pool, config_handler);
    return NULL;
}

char *get_configure_item(configure_handler_t *config_handler, 
    char *section_name, char *key)
{
    uint32_t section_len, key_len;
    configure_section_t *section = config_handler->section_head;
    configure_item_t *item;

    section_name = string_strip(section_name, ' ', &section_len);
    key = string_strip(key, ' ', &key_len);
    if(!section_name || !key) goto err;

    while(section) {
        if(section->section_len == section_len) {
            item = section->item_head;  
            break;
        }
        
        section = section->next;
    }

    while(item) {
        if(item->key_len == key_len) {
            if(0 == strncmp(item->key, key, key_len))
                return item->value;
        }
        item = item->next;
    }

err:

   return NULL;

}

