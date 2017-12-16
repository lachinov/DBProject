
#include "tracer.h"
#include "generator.h"

using namespace tracer;

request::request(int page, req_type type, int n_pages, int tsc) : r_page(page),
    r_type(type), r_n_pages(n_pages), r_tsc(tsc)
{
}

int request::get_page()
{
    return r_page;
}

req_type request::get_op_type()
{
    return r_type;
}

int request::get_number_of_pages()
{
    return r_n_pages;
}

int request::get_tsc()
{
    return r_tsc;
}

application::application() : a_init(false)
{
}

void application::push_request(request *req)
{
    /* make sure that we have the timestamp starting from zero */
    if(!a_init) {
        a_init = true;
        a_requests[0] = req;
        return;
    }
    a_requests[req->get_tsc()] = req;
}

int application::get_size() { return a_requests.size(); };
std::map<int, request*>& application::get_map()
{
    return a_requests;
}

simulation::simulation(int tx_sz, int cp_sz, int comp_sz, std::string file, int requests, int compact) :
    s_tx_buff_size(tx_sz), s_checkpoint_size_per_op(cp_sz), s_compact_size_per_op(comp_sz),
    s_file(file), s_num_requests(requests), s_compact_trigger(compact)
{
}

/* runs the simulation */
void simulation::run()
{
    s_out.open(s_file);

    _process_requests();

    s_out.close();
}

request* simulation::_alloc_request()
{
    gen::generator g(NUMBER_OF_PAGES);
    auto parse_op = [] (int i) { return static_cast<req_type>(i); };

    request *tmp = new request(
            g.request_page(),
            parse_op(g.request_type()),
            g.request_number_of_pages(),
            g.request_timestamp());
            
    return tmp;
}

void simulation::_create_requests()
{
    /* First we try to create NUMBER_OF_REQUESTS.
     * Because that the random number generator may
     * result in the same tsc (which is the key for
     * the map) then later we check if the map size
     * is equal to NUMBER_OF_REQUESTS or not. */
    for(int i = 0; i < s_num_requests; i++) {
        app.push_request(_alloc_request());
    }
    /* So, now that we have tried to insert NUMBER_OF_REQUESTS
     * in the map, some insertions may have suffered colision
     * and the actual map size is smaller than NUMBER_OF_REQUESTS.
     * To avoid overhead of acessing the .size() of the map
     * all the time (causing execution overhead), then we do it
     * here, because it is much probable that the .size() is
     * much closer to the NUMBER_OF_REQUESTS than if we did
     * it in previous loop. 
     * INFO: Sorry for duplicated code */
    while(app.get_size() <= s_num_requests) {
        app.push_request(_alloc_request());
    }
}

void simulation::_process_requests()
{
    int acc_buffers = 0, acc_compact = 0, total_used_buffers = 0;
   
    while(total_used_buffers <= s_num_requests) {
        request *req = _alloc_request(); 

        acc_buffers += req->get_number_of_pages();
        acc_compact += req->get_number_of_pages();

        if(acc_buffers >= s_tx_buff_size)
            acc_buffers -= _do_checkpoint(req->get_number_of_pages());

        _do_commit(req);

        total_used_buffers += req->get_number_of_pages();

        if(acc_compact >= s_compact_trigger) {
            acc_buffers -= _do_compact();
            acc_compact = 0;
        }
        free(req);
    }
    std::cout << acc_buffers << std::endl;
    while(acc_buffers >= 1)
        acc_buffers -= _do_checkpoint(acc_buffers);
    
}

void simulation::_do_commit(request* req)
{
    int n = req->get_number_of_pages();

    for(int i = 0; i < n; i++)
        _write_to_file(
            simulation::threads::S_COMMIT,
            req->get_page(), req->get_op_type(), gen::generator::read_rdtscp()); 
}

int simulation::_do_checkpoint(int buff)
{
    gen::generator g(buff);
    int n_freed_buffers = g.request_number_of_pages() + buff;

    n_freed_buffers = (n_freed_buffers >= s_checkpoint_size_per_op) ?
        s_checkpoint_size_per_op : n_freed_buffers;

    for(int i = 0; i < n_freed_buffers; i++)
        _write_to_file(
            simulation::threads::S_CHECKPOINT,
            i, req_type::R_WRITE, g.read_rdtscp());
    return n_freed_buffers;
}

int simulation::_do_compact()
{
    gen::generator g_reads(s_compact_size_per_op);
    int n_reads = g_reads.request_number_of_pages();

     gen::generator g_writes(n_reads);
    int n_writes = g_writes.request_number_of_pages();

    for(int i = 0; i < n_reads; i++) {
        _write_to_file(
            simulation::threads::S_COMPACTION,
            i, req_type::R_READ, g_reads.read_rdtscp());
        if(i >= n_writes)
        _write_to_file(
            simulation::threads::S_COMPACTION,
            i, req_type::R_WRITE, g_writes.read_rdtscp());
    }
    int total = n_reads - n_writes;
    return (total >= 0) ? total : 0;
}

void simulation::_write_to_file(simulation::threads id, int page, int op, uint64_t time)
{
    s_out << id << "," << page << "," << op << "," << time << "\n";
}


//#########################################



//-----------------------------------------------------------
// Testing section
//-----------------------------------------------------------
#ifdef _TEST_TRACER
int main()
{
    int i = 0;
    simulation sim(100, 2, 10, "parse2.csv", 1000000, 1000);
    sim.run();
}
#endif
