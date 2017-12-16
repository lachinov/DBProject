
#include "generator.h"
#include <math.h>

#ifdef _TEST_GENERATOR
#include <iostream>
#endif

using namespace gen;

generator::generator(int pages) : g_num_pages(pages), g_is_initialized(false), g_init_timestamp(0)
{
}

int generator::request_page()
{
    return _rand_uniform(g_num_pages);
}

req_type generator::request_type()
{
    return (_rand_uniform(g_num_pages) >= (g_num_pages / 2)) ?
        req_type::R_READ : req_type::R_WRITE; 
}

int generator::request_number_of_pages()
{
    /* I am using lambda = 1 */
    return (1 + floor(_rand_exponential(1)));
}

int generator::request_timestamp()
{
init:
    if(g_is_initialized)
        return (read_rdtscp() - g_init_timestamp);
    else {
        request_init();
        goto init;
    }
}

void generator::request_init()
{
    if(!g_is_initialized) {
        g_is_initialized = true;
        g_init_timestamp = read_rdtscp();
    }
}

uint64_t generator::read_rdtscp()
{
    uint32_t a, d, c;

    __asm__ volatile("rdtscp" : "=a" (a),"=d" (d), "=c" (c));
    return (((uint64_t) a) | (((uint64_t) d) << 32));
}

int generator::_rand_uniform(int max)
{
    static std::default_random_engine generator(read_rdtscp());
    std::uniform_int_distribution<int> distribution(0,max);

    return distribution(generator);
}

int generator::_rand_exponential(double lambda) 
{
    static std::default_random_engine generator(read_rdtscp());
    std::exponential_distribution<double> distribution(lambda);

    return distribution(generator);
}


#ifdef _TEST_GENERATOR

using namespace std;
int main() {
    gen::generator g(1000);
    for(int i = 0; i < 10; i++) {
        cout << "page = " << g.request_page() << endl;
        cout << "type = " << g.request_type() << endl;
        cout << "# pg = " << g.request_number_of_pages() << endl;
        cout << " tsc = " << g.request_timestamp() << endl;
    }
}

#endif