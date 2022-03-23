#include <algorithm>
#include <future>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

class stopWatch {
 private:
  decltype(std::chrono::high_resolution_clock::now()) start = std::chrono::high_resolution_clock::now();
  std::string _s;

 public:
  stopWatch(){};
  stopWatch(std::string s) : _s(s){};
  ~stopWatch() {
    const auto finish = std::chrono::high_resolution_clock::now();
    const auto durationMilliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    std::cout << (_s.empty() ? "" : _s + " ") << durationMilliSeconds << " ms" << std::endl;
  }
};

using some_map_type = std::unordered_map<std::string, std::map<std::string, std::vector<uint64_t>>>;
using some_vec_type = std::vector<std::map<std::string, std::vector<uint64_t>>>;

auto setupMap(uint64_t s) {
  some_map_type ret;
  for (size_t i = 0; i < s; i++) {
    decltype(ret.begin()->second) m;
    for (size_t j = 0; j < s; j++) {
      m.emplace(std::to_string(j), std::vector<uint64_t>(j, j));
    }
    ret.emplace(std::to_string(i), m);
  }
  return ret;
}
auto setupVec(uint64_t s) {
  some_vec_type ret;
  for (size_t i = 0; i < s; i++) {
    std::map<std::string, std::vector<uint64_t>> m;
    for (size_t j = 0; j < s; j++) {
      m.emplace(std::to_string(j), std::vector<uint64_t>(j, j));
    }
    ret.push_back(m);
  }
  return ret;
}

some_map_type parCopyMap(const some_map_type& src, std::size_t parNum = 4) {
  std::vector<std::future<void>> futures;
  futures.reserve(parNum);
  const auto t = src.size();
  some_map_type ret;
  ret.reserve(t);

  for (size_t i = 0; i < parNum; i++) {
    futures.push_back(std::async(std::launch::async, [&ret, &src, i, t, parNum]() {
      ret.insert(std::next(src.cbegin(), (i * t) / parNum), std::next(src.cbegin(), ((i + 1) * t) / parNum));
    }));
  }

  return ret;
}

auto parCopyVec(const some_vec_type& src, std::size_t parNum = 4) {
  std::vector<std::future<void>> futures;
  futures.reserve(parNum);
  const auto t = src.size();
  some_vec_type ret(t);

  for (size_t i = 0; i < parNum; i++) {
    futures.push_back(std::async(std::launch::async, [&ret, &src, i, t, parNum]() {
      std::copy(std::next(src.cbegin(), (i * t) / parNum), std::next(src.cbegin(), ((i + 1) * t) / parNum), std::next(ret.begin(), (i * t) / parNum));
    }));
  }

  return ret;
}

int main() {
  const auto orig = setupMap(658);
  for (size_t i = 0; i < 10; i++) {
    decltype(setupMap(358)) dest;
    {
      stopWatch t("copy");
      dest = some_map_type(orig.begin(), orig.end());
    }
    dest.clear();
  }
  for (size_t i = 0; i < 10; i++) {
    decltype(setupMap(358)) dest;
    {
      stopWatch t("par 2 ");
      dest = parCopyMap(orig, 2);
    }
    dest.clear();
  }

  for (size_t i = 0; i < 10; i++) {
    decltype(setupMap(358)) dest;
    {
      stopWatch t("par 4 ");
      dest = parCopyMap(orig, 4);
    }
    dest.clear();
  }
  for (size_t i = 0; i < 10; i++) {
    decltype(setupMap(358)) dest;
    {
      stopWatch t("par 6 ");
      dest = parCopyMap(orig, 6);
    }
    dest.clear();
  }
  {
    std::map a(orig.begin(), orig.end());
    auto p = parCopyMap(orig, 6);
    std::map b(p.begin(), p.end());
    if (a != b) {
      std::cout << "failed" << std::endl;
    }
  }

  const auto origV = setupVec(658);
  for (size_t i = 0; i < 10; i++) {
    decltype(setupVec(358)) dest;
    {
      stopWatch t("copy");
      dest = some_vec_type(origV.begin(), origV.end());
    }
    dest.clear();
  }
  for (size_t i = 0; i < 10; i++) {
    decltype(setupVec(358)) dest;
    {
      stopWatch t("par 2 ");
      dest = parCopyVec(origV, 2);
    }
    dest.clear();
  }
  for (size_t i = 0; i < 10; i++) {
    decltype(setupVec(358)) dest;
    {
      stopWatch t("par 4 ");
      dest = parCopyVec(origV, 4);
    }
    dest.clear();
  }
  for (size_t i = 0; i < 10; i++) {
    decltype(setupVec(358)) dest;
    {
      stopWatch t("par 6 ");
      dest = parCopyVec(origV, 6);
    }
    dest.clear();
  }
  {
    std::vector a(origV.begin(), origV.end());
    auto p = parCopyVec(origV, 6);
    std::vector b(p.begin(), p.end());
    if (a != b) {
      std::cout << "failed" << std::endl;
    }
  }
}