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
            uint64_t request_page();
            req_type request_type();
            int request_number_of_pages();
            int request_timestamp();
            void request_init();

            uint64_t read_rdstscp();
        private:
        uint64_t _read_rdtscp();
            int _rand_uniform(int max);
            int _rand_exponential(int lambda);
            uint64_t g_init_timestamp;
            bool g_is_initialized;
            int g_num_pages;
    };
}

