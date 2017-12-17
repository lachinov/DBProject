#include <cstring>
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

void simulation::_process_requests()
{
    int acc_buffers = 0, acc_compact = 0, total_used_buffers = 0;

    while(total_used_buffers <= s_num_requests) {
        request *req = _alloc_request();

        acc_buffers += req->get_number_of_pages();
        acc_compact += req->get_number_of_pages();

        /* here we check if we need to do checkpoint */
        if(acc_buffers >= s_tx_buff_size)
            acc_buffers -= _do_checkpoint();

        /* performs the commit operation */
        _do_commit(req);

        total_used_buffers += req->get_number_of_pages();

        /* here we check if the compaction thread must be started */
        if(acc_compact >= s_compact_trigger) {
            acc_buffers -= _do_compact();
            acc_compact = 0;
        }
        free(req);
    }

    /* now we must check if we still have pages in the cp list
     * that must be checkpointed (the remainder pages)*/
    while(acc_buffers >= 1)
        acc_buffers -= _do_checkpoint();
}

void simulation::_do_commit(request* req)
{
    int n = req->get_number_of_pages();

    for(int i = 0; i < n; i++) {
        _write_to_file(
                simulation::threads::S_COMMIT,
                (req->get_page()+i), req->get_op_type(), gen::generator::read_rdtscp());
        cp_list[req->get_page() + i] = req->get_page() + i;
    }
}

int simulation::_do_checkpoint()
{
    gen::generator g(cp_list.size());
    int i = 0;
    int n_freed_buffers = g.request_number_of_pages();

    n_freed_buffers = (n_freed_buffers >= s_checkpoint_size_per_op) ?
        s_checkpoint_size_per_op : n_freed_buffers;
    /* sanity check */
    if(n_freed_buffers > cp_list.size())
        n_freed_buffers = cp_list.size();

    for(auto it = cp_list.begin(); i < n_freed_buffers; i++, ++it) {
        _write_to_file(
                simulation::threads::S_CHECKPOINT,
                it->first, req_type::R_WRITE, g.read_rdtscp());
        cp_list.erase(it);
    }
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

//-----------------------------------------------------------
// Generator
//-----------------------------------------------------------
void cmd_info()
{
    std::cerr << "incorrect cmp arguments\n";
    std::cerr << "-tx <transaction size> -cp <checkpoint size>" \
        " -cmp <compaction size> -file <output file> - req <number of requests> -trigger <compact trigger>\n";
}

int main(int argc, char **argv)
{
    uint32_t arg_parse_state = 0;
    int tx_size =0, cp_size = 0, cmp_size, requests = 0, trigger = 0;
    std::string filename;

    if(argc == 1 || argc % 2 == 0) {
        cmd_info();
        return 1;
    }

    for(int i = 1; i < argc; ++i)
    {
        if(!std::strcmp(argv[i], "-tx")) {
            tx_size = std::stoi(argv[++i]);
            arg_parse_state |= 0x1;

        } else if (!std::strcmp(argv[i], "-cp")) {
            cp_size = std::stoi(argv[++i]);
            arg_parse_state |= 0x1 << 1;

        } else if (!std::strcmp(argv[i], "-cmp")) {
            cmp_size = std::stoi(argv[++i]);
            arg_parse_state |= 0x1 << 2;

        } else if (!std::strcmp(argv[i], "-file")) {
            filename = std::string(argv[++i]);
            arg_parse_state |= 0x1 << 3;

        } else if (!std::strcmp(argv[i], "-req")) {
            requests = std::stoi(argv[++i]);
            arg_parse_state |= 0x1 << 4;

        } else if (!std::strcmp(argv[i], "-trigger")) {
            trigger = std::stoi(argv[++i]);
            arg_parse_state |= 0x1 << 5;
        }
    }

    if(arg_parse_state != 0x3F) {
        cmd_info();
        return 1;
    }

    simulation sim(tx_size, cp_size, cmp_size, filename, requests, trigger);
    sim.run();
}
