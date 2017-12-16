
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

simulation::simulation(int tx_sz, int cp_sz, int comp_sz, std::string file, int requests) :
    s_tx_buff_size(tx_sz), s_checkpoint_size_per_op(cp_sz), s_compact_size_per_op(comp_sz),
    s_file(file), s_num_requests(requests)
{
}

/* runs the simulation */
void simulation::run()
{
    _create_requests();
    _process_requests();
}

request* simulation::_alloc_request()
{
    gen::generator g;
    /*request *tmp = new request(
            g.request_page(),
            g.request_type(),
            g.request_number_of_pages(),
            g.request_timestamp());*/
    request *tmp;
    return tmp;
}

void simulation::_create_requests()
{
    /* First we try to create NUMBER_OF_REQUESTS.
     * Because that the random number generator may
     * result in the same tsc (which is the key for
     * the map) then later we check if the map size
     * is equal to NUMBER_OF_REQUESTS or not. */
    for(int i = 0; i < NUMBER_OF_REQUESTS; i++) {
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
    while(app.get_size() <= NUMBER_OF_REQUESTS) {
        app.push_request(_alloc_request());
    }
}

void simulation::_process_requests()
{
    int acc_buffers;
    for(auto it = app.get_map().begin(); it != app.get_map().end(); ++it) {
        std::cout << it->first << std::endl;

    }
}

void simulation::_do_checkpoint()
{

}

void simulation::_do_commit()
{
}


//#########################################



//-----------------------------------------------------------
// Testing section
//-----------------------------------------------------------
#ifdef _TEST_GENERATOR
int main()
{
    int i = 0;
    while (i++ < 10)
    ;
       // std::cout << generator::request_page() << std::endl;
}
#endif
