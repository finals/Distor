#include <stdio.h>
#include "../configure_handler.h"

void print_configure(configure_handler_t *handler)
{
    configure_item_t *item;
    configure_section_t *section = handler->section_head;

    while(section) {
        printf("Section: |%s|\n", section->section);
        item = section->item_head;
        while(item) {
            printf("\tKey: |%s|  Value: |%s|\n", item->key, item->value);
            item = item->next;
        }
        section = section->next;
    }
}

void test_configure()
{
    configure_handler_t *config_handler;
    char *filename = "test.conf";
    MemPool *pool = mem_pool_create(640); //640M
    char section_search[] = "[filter:dlo]";
    char key_search[] = "use";

    config_handler = load_configure_file(pool, filename);

    //print_configure(config_handler);

    printf("%s %s:  %s\n", section_search, key_search,
        get_configure_item(config_handler,section_search ,key_search ));


    mem_pool_destroy(pool);
}


int main()
{
    test_configure();

    return 0;
}
