#include <stdint.h>
#include <random>

namespace gen {
    
    typedef enum {
        R_READ = 0,
        R_WRITE
    } req_type;

class generator {
        public:
            generator(int pages);
            int request_page();
            req_type request_type();
            int request_number_of_pages();
            int request_timestamp();
            static uint64_t read_rdtscp();
            void request_init();
            
        private:
            
            int _rand_uniform(int max);
            int _rand_exponential(double lambda);
            uint64_t g_init_timestamp;
            bool g_is_initialized;
            int g_num_pages;
    };
}

