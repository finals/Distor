#include <unistd.h>
#include "log.h"

void test_log(MemPool *pool, configure_handler_t *config_handler)
{
    log_runtime_t *log_runtime = NULL;
    log_client_t *log = NULL;
    
    log_runtime = log_runtime_create(pool, config_handler);
    

    
    
}


int main()
{
    MemPool *pool = mem_pool_create(640); //640M
    configure_handler_t *config_handler = load_configure_file(pool, "test.conf");

    test_log(pool, config_handler);

    configure_handler_destroy(pool, config_handler);
    mem_pool_destroy(pool);
    return 0;
}
