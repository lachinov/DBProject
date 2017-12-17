#include <string>
#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <chrono>
#include <map>
#include <math.h>
#include <fstream>
#include <iostream>

#define NUMBER_OF_PAGES (100000)

#define SIMULATION_TIME (60 * 60 * 24 * 1000) // 24 hours in miliseconds

namespace tracer {

uint64_t get_tsc()
{
    uint32_t a, d, c;

    __asm__ volatile("rdtscp" : "=a" (a),"=d" (d), "=c" (c));
    return (((uint64_t) a) | (((uint64_t) d) << 32));
}


    typedef enum {
        R_READ = 0,
        R_WRITE
    } req_type;

    class request
    {
        public:
            /* Constructor of a single request made by an application
             *
             * @page page number of the request
             * @type read or write
             * @n_pages number of pages this request wants
             * @tsc timestamp of the operation */
            request(int page, req_type type, int n_pages, int tsc);

            int get_page();
            req_type get_op_type();
            int get_number_of_pages();
            int get_tsc();
        private:
            int r_page;
            req_type r_type;
            int r_n_pages;
            int r_tsc;
    };

    class simulation
    {
        public:
            /* types of threads that we are simulating */
            typedef enum {
                S_COMMIT = 0,
                S_CHECKPOINT,
                S_COMPACTION
            } threads;
            /* constructor with the sizes parameters
             *
             * @tx_sz transaction buffer size
             * @cp_sz checkpoint size per operation
             * @comp_sz compaction size per operation
             * @file filename where the traces are going to be stored
             * @requests number of requests to simulate
             * @compact thresold to start compaction*/
            simulation(int tx_sz, int cp_sz, int comp_sz, std::string file, int requests, int compact);
            /* runs the simulation */
            void run();
        private:
            request* _alloc_request();
            
            void _create_requests();

            void _process_requests();

            void _do_commit(request* req);

            int _do_checkpoint(int buff);

            int _do_compact();

            void _write_to_file(simulation::threads id, int page, int op, uint64_t time);

            /* maximum transaction buffer size (page #) */
            int s_tx_buff_size;
            /* maximum number of pages checkpointed per op */
            int s_checkpoint_size_per_op;
            /* maximum number of pages compacted per op */
            int s_compact_size_per_op;
            /* compaction trigger, when the number of used
             * buffers reach this value, then it triggers
             * the compaction operation. */
            int s_compact_trigger;
            /* path to the file that is going to save the outputs */
            std::string s_file;
            /* number of requests to simulate */
            int s_num_requests;

            std::ofstream s_out;
    };
}