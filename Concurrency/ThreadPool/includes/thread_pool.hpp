#include <vector>
#include <thread>
#include "simple_task.hpp"
#include "unbounded_MPMC_queue.hpp"

namespace stud_tpl {

	class thread_pool {
	
	public:
		static thread_pool& instance() {
			static thread_pool static_thread_pool;
			return static_thread_pool;
		}

		template<typename Task>
		static void spawn(Task&& t) {
			auto& tp = instance();
			tp.tasks_queue_.put(std::forward<Task>(t));
		}

	private:
		// reserve 1 thread as an external
		explicit thread_pool(size_t nThreadsCnt = (std::thread::hardware_concurrency() - 1)) : continue_work_flag_{ true }
		{
			create_and_run_workers(nThreadsCnt);
		}


		~thread_pool()
		{
			continue_work_flag_.store(false);
			join_all_workers();
		}

		void create_and_run_workers(size_t nThreadsCnt)
		{
			thread_pool_.reserve(nThreadsCnt);

			for (size_t i = 0; i < nThreadsCnt; ++i)
				thread_pool_.emplace_back([this]() {worker_routine(); });
		}


		void worker_routine()
		{
			while (continue_work_flag_)
			{
				task_type task = std::move(tasks_queue_.get());
				task();
			}
		}

		void join_all_workers()
		{
			for (auto& worker : thread_pool_)
				worker.join();
		}

		std::vector<std::thread> thread_pool_;
		unbounded_mpmc_queue<task_type> tasks_queue_;
		std::atomic<bool> continue_work_flag_;
	};
}