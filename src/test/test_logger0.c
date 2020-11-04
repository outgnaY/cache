#include "../cache.h"

static pthread_t tids[4];

void *func(void *args) {
    pthread_t tid = (pthread_t)(*((pthread_t *)args));
    printf("tid = %d\n", tid);
    int i;
    for (i = 0; i < 1000000; i++) {
        log_infof("info %d %d\n", tid, i);
        // usleep(500);
    }
    return NULL;
}
int main() {
    // basic tests 
    
    int i; 
    log_init();
    log_set_level(LOGGER_INFO);
    // log_check_and_mkdir(".");
    log_set_base_path("/home/wcy");
    log_set_filename("testlog"); 
    
    for (i = 0; i < 1000000; i++) {
        log_debugf("debug %d\n", i);
        // usleep(500);
    }
    
    // multi-threading tests 
    /*
    time_t start, end;
    log_init();
    log_set_level(LOGGER_INFO);
    log_set_base_path("/home/wcy");
    log_set_filename("testlog");
    
    start = time(NULL);
    int i;
    
    for (i = 0; i < 4; i++) {
        pthread_create(&tids[i], NULL, func, &tids[i]);
    }
    for (i = 0; i < 4; i++) {
        pthread_join(tids[i], NULL);
    }
    end = time(NULL);
    printf("%ld\n", end - start);
    */

}