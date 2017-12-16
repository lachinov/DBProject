
#include "generator.h"
#include <math.h>

#ifdef _TEST_GENERATOR
#include <iostream>
#endif

using namespace gen;

generator::generator(int pages) : g_num_pages(pages), g_is_initialized(false), g_init_timestamp(0)
{
}

uint64_t generator::request_page()
{
    return static_cast<uint64_t>(_rand_uniform(g_num_pages));
}

req_type generator::request_type()
{
    return (_rand_uniform(g_num_pages) >= (g_num_pages / 2)) ?
        req_type::R_READ : req_type::R_WRITE; 
}

int generator::request_number_of_pages()
{
    /* I am using lambda = 1 */
    return ceil(_rand_exponential(1));
}

int generator::request_timestamp()
{
init:
    if(g_is_initialized)
        return (_read_rdtscp() - g_init_timestamp);
    else {
        request_init();
        goto init;
    }
}

void generator::request_init()
{
    if(!g_is_initialized) {
        g_is_initialized = true;
        g_init_timestamp = _read_rdtscp();
    }
}

uint64_t generator::_read_rdtscp()
{
    uint32_t a, d, c;

    __asm__ volatile("rdtscp" : "=a" (a),"=d" (d), "=c" (c));
    return (((uint64_t) a) | (((uint64_t) d) << 32));
}

int generator::_rand_uniform(int max)
{
    static std::default_random_engine generator(_read_rdtscp());
    std::uniform_int_distribution<int> distribution(0,max);

    return distribution(generator);
}

int generator::_rand_exponential(int lambda) 
{
    static std::default_random_engine generator(_read_rdtscp());
    std::exponential_distribution<double> distribution(lambda);

    return distribution(generator);
}


#ifdef _TEST_GENERATOR

using namespace std;
int main() {
    gen::generator g(1000);
    for(int i = 0; i < 10; i++)
        cout << g.request_page() << endl;
}

#endif