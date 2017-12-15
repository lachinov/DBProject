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

int main()
{
	std::vector<std::shared_ptr<wrr_queue::queue> > inputs;
	std::vector<uint32_t> weights;

	for (int i = 0; i < thread::types::T_SIZE_ENUM; ++i)
	{
		inputs.push_back(std::make_shared<wrr_queue::queue>());
		weights.push_back(i + 1);
	}

	std::ifstream file("trace.csv");
	csv_parser::parser row;

	auto parse_op = [] (int i) { return static_cast<wrr_queue::mem_operation::type>(i); };

	while(file >> row) {
		static int i = 0;
		inputs[row[csv_parser::parser::column::P_THREAD_ID]]->push(
				wrr_queue::mem_operation(
					row[csv_parser::parser::column::P_PAGE_NUM],
					parse_op(row[csv_parser::parser::column::P_READ_WRITE]),
					row[csv_parser::parser::column::P_TIMESTAMP]));
	}
	wrr_queue::scheduler scheduler(inputs, weights, 10, 100, 10);
	scheduler.run();

	return 0;
}
