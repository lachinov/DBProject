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

#define NUMBER_OF_REQUESTS (1000000)

#define NUMBER_OF_PAGES (100000)

#define SIMULATION_TIME (60 * 60 * 24 * 1000) // 24 hours in miliseconds

namespace tracer {
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

    class application
    {
        public:
            application();
            void push_request(tracer::request *req);
            int get_size();
            std::map<int, tracer::request*>& get_map();
        private:
            std::map<int, tracer::request*> a_requests;
            bool a_init;
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
             * @tx_sz transactino buffer size
             * @cp_sz checkpoint size per operation
             * @comp_sz compaction size per operation
             * @file filename where the traces are going to be stored
             * @requests number of requests to simulate */
            simulation(int tx_sz, int cp_sz, int comp_sz, std::string file, int requests);
            /* runs the simulation */
            void run();
        private:
            request* _alloc_request();
            
            void _create_requests();

            void _process_requests();

            void _do_commit(request* req);

            int _do_checkpoint(int buff);

            int _do_compact();

            void _write_to_file(simulation::threads id, int page, int op, int time);

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
            /* app that stores the requests */
            application app;

            std::ofstream s_out;
    };
}