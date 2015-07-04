#ifndef SAFE_QUEUE
#define SAFE_QUEUE

// Based on http://stackoverflow.com/questions/15278343/c11-thread-safe-queue

#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>

#define QUEUE_MAX 10000
#define QUEUE_FULL_WARNING_THRESHOLD 1000

template <class T>
class SafeQueue
{
public:
	SafeQueue(void)
		: q()
		, m()
		, c()
	{}

	~SafeQueue(void)
	{}

	void enqueue(T t) {
		std::lock_guard<std::mutex> lock(m);
		if (!q.empty()) {
			auto size(q.size());
			if (size >= QUEUE_MAX)
				return;
			if (size == QUEUE_MAX - QUEUE_FULL_WARNING_THRESHOLD)
				std::cerr << "Warning: queue fills up! Feel free to search the bug in either the driver or usbproxy.\n";
		}
		q.push(t);
		c.notify_one();
	}

	T dequeue(void) {
		std::unique_lock<std::mutex> lock(m);
		while(q.empty()) {
			c.wait(lock);
		}
		T val;
		std::swap(q.front(), val);
		q.pop();
		return val;
	}

private:
	std::queue<T> q;
	mutable std::mutex m;
	std::condition_variable c;
};
#endif
