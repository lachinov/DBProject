#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include "scheduler.h"
#include "parser.h"

namespace thread {
    typedef enum {
        T_COMMIT = 0,
        T_CHECKPOINT,
        T_COMPAT,
        T_SIZE_ENUM
    } types;
}

std::string perform_report(const wrr_queue::scheduler& s)
{
	const uint64_t queues_number = s.get_finishing_time().size();
	std::stringstream ss;
	ss << "thread,finish_time,operations_count,avg_responce_time,iopu\n";

	for (uint64_t i = 0; i < queues_number; ++i)
	{
		const uint64_t finish_time = s.get_finishing_time()[i];
		const uint64_t operation_count = s.get_operations_count()[i];
		const std::vector<uint64_t> &responce_times = s.get_responce_time()[i];

		float avg_responce_time = 0.0f;
		for (uint64_t j = 0; j < operation_count; ++j)
			avg_responce_time += responce_times[j];
		avg_responce_time /= (float)operation_count;

		const float ops_per_time_unit = operation_count / (float)finish_time;

		ss << i << ","<< finish_time << "," << operation_count << "," <<
			avg_responce_time << "," << ops_per_time_unit << "\n";
	}

	return ss.str();
}

std::vector<uint32_t> read_weights(const std::string &filename)
{
	std::vector<uint32_t> ret;
	std::ifstream fin;
	fin.open(filename, std::ios::in);
	std::string buffer;
	while (std::getline(fin, buffer))
		ret.push_back(std::stoul(buffer));

	return ret;
}


void cmd_args_report()
{
	std::cerr << "incorrect cmp arguments\n";
	std::cerr << "-i <input path> -o <output path> -w <weights path> -rt <read time> - wt <write time> -qs <dev queue size>\n";
}

int main(int argsc, char **argsv)
{
	uint32_t arg_parse_state = 0;

	std::string filename_in;
	std::string filename_out;
	std::string filename_weights;
	uint64_t time_read;
	uint64_t time_write;
	uint64_t size_queue;

	if (argsc == 1 || argsc % 2 == 0)
	{
		cmd_args_report();
		return 1;
	}

	for (uint64_t i = 1; i < argsc; ++i)
	{
		if (!std::strcmp(argsv[i], "-i"))
		{
			filename_in = std::string(argsv[++i]);
			arg_parse_state |= 0x1;
		}
		else if (!std::strcmp(argsv[i], "-o"))
		{
			filename_out = std::string(argsv[++i]);
			arg_parse_state |= 0x1 << 1;
		}
		else if (!std::strcmp(argsv[i], "-w"))
		{
			filename_weights = std::string(argsv[++i]);
			arg_parse_state |= 0x1 << 2;
		}
		else if (!std::strcmp(argsv[i], "-rt"))
		{
			time_read= std::stoull(argsv[++i]);
			arg_parse_state |= 0x1 << 3;
		}
		else if (!std::strcmp(argsv[i], "-wt"))
		{
			time_write = std::stoull(argsv[++i]);
			arg_parse_state |= 0x1 << 4;
		}
		else if (!std::strcmp(argsv[i], "-qs"))
		{
			size_queue= std::stoull(argsv[++i]);
			arg_parse_state |= 0x1 << 5;
		}
	}

	if (arg_parse_state != 0x3F)
	{
		cmd_args_report();
		return 1;
	}

	std::vector<uint32_t> weights = read_weights(filename_weights);
	std::vector<std::shared_ptr<wrr_queue::queue> > inputs;

	for (int i = 0; i < weights.size(); ++i)
	{
		inputs.push_back(std::make_shared<wrr_queue::queue>());
	}

	std::ifstream file(filename_in);
	csv_parser::parser row;

	auto parse_op = [] (int i) { return static_cast<wrr_queue::mem_operation::type>(i); };

	while(file >> row) {
		inputs[row[csv_parser::parser::column::P_THREAD_ID]]->push(
				wrr_queue::mem_operation(
					row[csv_parser::parser::column::P_PAGE_NUM],
					parse_op(row[csv_parser::parser::column::P_READ_WRITE]),
					row[csv_parser::parser::column::P_TIMESTAMP]));
	}
	wrr_queue::scheduler scheduler(inputs, weights, time_read, time_write, size_queue);
	scheduler.run();


	std::string out = perform_report(scheduler);

	std::fstream fout;
	fout.open(filename_out, std::ios::out);
	fout << out;
	fout.flush();
	fout.close();

	return 0;
}
