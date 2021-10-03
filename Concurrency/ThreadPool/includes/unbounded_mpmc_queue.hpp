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
	
			
			//while construction explanation:
			//If anyone puts task
			//1-st thread or 2-nd thread can take that new task by 2 ways:
			//1-st thread is waking up by condition_variable notifying
			//2-nd thread has just finished other task and also trying to get task
			//and 2-nd thread takes that task before 1-st thread takes it.
			//If its case happens 1-st thread takes task by empty deque
			//according cppreference.com: void pop_front() in std::deque if there are no
			//elements in the container,the behavior is undefined.
			//thats why the 1-st thread after waking up must check buffer on empty
			while (buffer_.empty())
				not_empty_.wait(lock);//wait for pushing task
	
			return take_locked();
		}

		size_t get_waiting_tasks_cnt() const
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

		std::atomic<size_t> waiting_tasks_cnt_;
	};
}