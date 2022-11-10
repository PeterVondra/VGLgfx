#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>

namespace Utils
{
	class ThreadPool
	{
	public:
		ThreadPool(const unsigned int p_NumThreads);

		template<typename F, typename... Args>
		auto enqueue(F&& f, Args&&... args)->std::future<typename std::result_of<F(Args...)>::type>;

		void wait()
		{
			{
				std::unique_lock<std::mutex> lock(m);
				stop = true;
			}
			v.notify_all();
			for (std::thread& worker : m_Workers)
				worker.join();
		}

		~ThreadPool();

	private:
		const unsigned int m_NumThreads;

		std::vector<std::thread> m_Workers;

		std::mutex m;
		std::condition_variable v;
		std::queue<std::function<void()>> work;

		std::vector<std::future<void>> finished;

		bool stop;
	};


	inline ThreadPool::ThreadPool(const unsigned int p_NumThreads) : m_NumThreads(p_NumThreads), stop(false)
	{
		for (size_t i = 0; i < p_NumThreads; ++i) {
			m_Workers.emplace_back([this] {
				for (;;) {
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(this->m);
						this->v.wait(lock, [this] { return this->stop || !this->work.empty(); });

						if (this->stop && this->work.empty())
							return;
						task = std::move(this->work.front());
						this->work.pop();
					}
					task();
				}
				});
		}
	}

	template<typename F, typename... Args>
	auto ThreadPool::enqueue(F&& f, Args&&... args)
		->std::future<typename std::result_of<F(Args...)>::type>
	{
		using return_type = typename std::result_of<F(Args...)>::type;
		auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(m);

			work.emplace([task]() { (*task)(); });
		}

		v.notify_one();
		return res;
	}

	inline ThreadPool::~ThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(m);
			stop = true;
		}

		v.notify_all();
		for (std::thread& worker : m_Workers)
			worker.join();
	}
}

