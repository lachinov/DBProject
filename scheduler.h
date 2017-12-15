#pragma once

#include <queue>
#include <memory>
#include <cassert>
#include "timer.h"


namespace wrr_queue
{

	class mem_operation
	{
	public:
		typedef enum {
			OP_READ = 0,
			OP_WRITE
		} type;

		mem_operation(uint64_t page, type op_type, uint64_t time = 0ull) : m_page(page), m_type(op_type), m_time(time) { ; }
		mem_operation(const mem_operation &copy) : m_page(copy.m_page), m_type(copy.m_type), m_time(copy.m_time) { ; }
		mem_operation(const mem_operation &copy, uint64_t time) : m_page(copy.m_page), m_type(copy.m_type), m_time(time) { ;}

		static bool cmp(const mem_operation &left, const mem_operation &right) { return left.m_time > right.m_time; }

		type get_type() const
		{
			return m_type;
		}

		uint64_t get_page() const
		{
			return m_page;
		}

		uint64_t get_time() const
		{
			return m_time;
		}

	private:
		type m_type;
		uint64_t m_page;
		uint64_t m_time;
	};


	class cmp
	{
	public:
		bool operator()(const mem_operation &left, const mem_operation &right) const
		{
			return mem_operation::cmp(left, right);
		}
	};

	class queue : public time_dependent
	{
	public:
		queue() { ; }

		virtual void wait()
		{
			;
		}
		// report time of the next event
		// 0 - now or 
		virtual uint64_t next_event() const
		{
			return m_queue.top().get_time();
		}
		virtual bool is_available() const
		{
			return m_queue.size();
		}

		uint64_t size() const
		{
			return m_queue.size();
		}

		void push(const mem_operation &op)
		{
			m_queue.push(op);
		}

		mem_operation pop()
		{
			mem_operation ret = m_queue.top();
			m_queue.pop();
			return ret;
		}

	private:
		std::priority_queue<mem_operation, std::vector<mem_operation>, cmp> m_queue;
	};


	class scheduler : public timer
	{
	public:
		scheduler(std::vector<std::shared_ptr<queue> > &input_queues, std::vector<uint32_t> &weights,
			uint64_t device_read_time, uint64_t device_write_time, uint64_t device_queue_size) :
			m_device_read_time(device_read_time), m_device_write_time(device_write_time), m_device_queue_size(device_queue_size),
			m_device_queue(std::make_shared<queue>())
		{
			assert(input_queues.size() == weights.size());
			m_input_queues = input_queues;
			m_weights = weights;

			m_queue_finishing_time.resize(weights.size());
			m_queue_operations_count.resize(weights.size());
			m_queue_responce_time.resize(weights.size());
		}


		void run()
		{
			bool stop = false;
			uint32_t max_weight = *std::max_element(m_weights.begin(), m_weights.end());
			std::vector<time_dependent*> waiter_list;

			for(auto v : m_input_queues)
				waiter_list.push_back(v.get());

			while (!stop)
			{
				m_service_round_weights = m_weights;
				//service round
				for (uint32_t i = 0; i < max_weight; ++i)
				{
					for (uint32_t j = 0; j < m_input_queues.size(); ++j)
					{
						if (m_service_round_weights[j] == 0 || !m_input_queues[j]->is_available() ||
							m_input_queues[j]->next_event() > this->report_time())
							continue;
						--m_service_round_weights[j];

						mem_operation op = m_input_queues[j]->pop();

						uint64_t op_time = op.get_type() == mem_operation::OP_READ ? m_device_read_time : m_device_write_time;

						if (m_device_queue->size() > m_device_queue_size)
						{
							this->wait(m_device_queue.get());
							m_device_queue->pop();
						}
						uint64_t finish_time = op_time + this->report_time();
						m_device_queue->push(mem_operation(op, finish_time));

						m_queue_responce_time[j].push_back(finish_time - op.get_time());
						++m_queue_operations_count[j];
						if (m_input_queues[j]->size() == 0)
							m_queue_finishing_time[j] = this->report_time();

						this->wait_any(waiter_list);
					}
				}
				//end of service round
				bool _stop = true;

				for (auto q : m_input_queues)
					_stop &= !q->is_available();
				stop = _stop;
			}
		}

	private:

		std::shared_ptr<queue> m_device_queue;
		std::vector<std::shared_ptr<queue> > m_input_queues;
		std::vector<uint32_t> m_weights;
		std::vector<uint32_t> m_service_round_weights;

		bool m_stop;

		uint64_t m_device_read_time;
		uint64_t m_device_write_time;
		uint64_t m_device_queue_size;


		std::vector<uint64_t> m_queue_finishing_time;
		std::vector<uint64_t> m_queue_operations_count;
		std::vector<std::vector<uint64_t> > m_queue_responce_time;
	};
}