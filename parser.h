#include <vector>
#include <iostream>
#include <string>
#include <iterator>
#include <sstream>
#include <fstream>

namespace csv_parser {
	/* Basically we read from the file line by line */
	class parser {
		public:
		/* The CSV format is
		 * THREAD_ID, PAGE_NUMBER, READ_OR_WRITE, TIMESTAMP */
		typedef enum {
			P_THREAD_ID = 0,
			P_PAGE_NUM,
			P_READ_WRITE,
			P_TIMESTAMP
		} column;
		int operator[](std::size_t index) const
		{
			return std::stoi(m_data[index]);
		}
		std::size_t size() const
		{
			return m_data.size();
		}
		void read_next_row(std::istream& str)
		{
			std::string line;
			std::getline(str, line);
			std::stringstream line_stream(line);
			std::string cell;

			m_data.clear();
			while(std::getline(line_stream, cell, ','))
				m_data.push_back(cell);

			if(!line_stream && cell.empty())
				m_data.push_back("");
		}
		private:
		std::vector<std::string> m_data;
	};
} /*  end namespace csv_parser */

std::istream& operator>>(std::istream& str, csv_parser::parser& data)
{
	data.read_next_row(str);
	return str;
}

#ifdef TEST_PARSER
int main()
{
	std::ifstream file("trace.csv");
	csv_parser::parser row;
	while(file >> row) {
		std::cout << "Element = " << row[3] << "\n";
	}
}
#endif
