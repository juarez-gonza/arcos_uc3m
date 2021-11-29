#include <iostream>
#include <thread>
#include <vector>

#include <mutex>

class counter {
public:
  counter() :
	  value_{0}
  {
  }

  void update()
  {
	  std::lock_guard<std::mutex> guard(mtx);
	  value_++;
  }

  void print() const
  {
	  std::cout << "counter = " << value_ << "\n";
  }

private:
  double value_;
  std::mutex mtx;
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
