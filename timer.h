#pragma once

#include <stdint.h>
#include <vector>
#include <algorithm>

namespace wrr_queue
{
	class time_dependent
	{
	public:
		// wait for this object
		virtual void wait() = 0;
		// report time of the next event
		// 0 - now or 
		virtual uint64_t next_event() const = 0;

		virtual bool is_available() const = 0;
	protected:
	};

	class timer
	{
	public:
		timer() : m_time(0ull){;}

		uint64_t report_time() const
		{
			return m_time;
		}

		void timer_tick()
		{
			++m_time;
		}

		void wait(const uint64_t time)
		{
			m_time += time;
		}


		void wait(const time_dependent* time)
		{
			uint64_t event_time = time->next_event();
			m_time = std::max(m_time, event_time);
		}

		void wait_all(const std::vector<time_dependent*> &events)
		{
			uint64_t event_time = m_time;
			for(auto v : events)
				if (v->is_available())
					event_time = std::max(event_time, v->next_event());

			m_time = std::max(m_time, event_time);
		}

		void wait_any(std::vector<time_dependent*> &events)
		{
			uint64_t event_time = m_time;
			for (auto v : events)
				if (v->is_available())
				{
					event_time = v->next_event();
					break;
				}

			for (auto v : events)
				if (v->is_available())
					event_time = std::min(event_time, v->next_event());

			m_time = std::max(m_time, event_time);
		}
	private:
		uint64_t m_time;
	};

	

}