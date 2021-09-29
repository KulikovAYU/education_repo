#include <mutex>
#include <deque>

namespace stud_tpl
{

	template<typename T>
	class unbounded_mpmc_queue
	{
	public:
	
		// produce  data to the worker thread
		void put(T task)
		{
			std::unique_lock<std::mutex> lock(is_busy_);
			buffer_.push_back(std::move(task));
			++waiting_tasks_cnt_;
			not_empty_.notify_all();
		}
	
		// consume data to the worker thread
		T get()
		{
			std::unique_lock<std::mutex> lock(is_busy_);
	
			//wait for pushing task
			//while because 1-st thread can take but 
			//2-nd thread can put task
			while (buffer_.empty())
				not_empty_.wait(lock);
	
			return take_locked();
		}

		std::atomic<std::size_t> get_waiting_tasks_cnt() const 
		{
			return waiting_tasks_cnt_;
		}
	
	private:
		T take_locked()
		{
			T front = std::move(buffer_.front());
			buffer_.pop_front();
			--waiting_tasks_cnt_;
			return front;
		}
	
	
	
		std::deque<T> buffer_;
		std::condition_variable not_empty_;
		std::mutex is_busy_;

		std::atomic<std::size_t> waiting_tasks_cnt_;
	};
}