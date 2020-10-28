#include "../cache.h"

int main() {
    /* basic tests */
    int i; 
    log_init();
    log_set_level(LOGGER_INFO);
    log_check_and_mkdir(".");
    log_set_filename("testlog"); 
    log_set_base_path("/home/wcy");
    
    for (i = 0; i < 1000; i++) {
        log_infof("info %d\n", i);
    }
    
}