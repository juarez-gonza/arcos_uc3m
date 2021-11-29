#include <iostream>
#include <thread>
#include <vector>

#include <atomic>

class spinlock_mutex {
public:
	spinlock_mutex() :
		flag_{}
	{
	}

	void lock()
	{
		while (flag_.test_and_set(std::memory_order_seq_cst))
			while(flag_.test(std::memory_order_relaxed))
				;
	}

	void unlock()
	{
		flag_.clear(std::memory_order_seq_cst);
	}
private:
	std::atomic_flag flag_;
};

class counter {
public:
  counter() : value_{0}
  {
  }

  void update()
  {
	  mtx.lock();
	  value_++;
	  time_ += 0.25;
	  mtx.unlock();
  }

  void print() const
  {
	  std::cout << "counter = " << value_ << "\n";
  }

private:
  double value_;
  double time_;
  spinlock_mutex mtx;
};

int main(int argc, char *argv[])
{

  using namespace std::chrono;
  auto t1 = high_resolution_clock::now();

  if (argc < 2) {
	  std::cerr << "Uso: " << argv[0] << " [numero de hilos]\n";
	  return 1;
  }

  const int num_threads = atoi(argv[1]);

  counter count;

  std::vector<std::thread> tasks;
  for (int i = 0; i < num_threads; ++i) {
    tasks.emplace_back([&] {
      for (int i = 0; i < 100'000; ++i) {
        count.update();
      }
    });
  }

  for (auto &t : tasks) {
    t.join();
  }

  auto t2 = high_resolution_clock::now();

  auto diff = duration_cast<microseconds>(t2 - t1);

  count.print();
  std::cout << "Time = " << diff.count() << " microseconds\n";

  return 0;
}
