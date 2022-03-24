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

auto setupMap(uint64_t s) {
  std::unordered_map<std::string, std::map<std::string, std::vector<uint64_t>>> ret;
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
  std::vector<std::map<std::string, std::vector<uint64_t>>> ret;
  for (size_t i = 0; i < s; i++) {
    std::map<std::string, std::vector<uint64_t>> m;
    for (size_t j = 0; j < s; j++) {
      m.emplace(std::to_string(j), std::vector<uint64_t>(j, j));
    }
    ret.push_back(m);
  }
  return ret;
}

template <typename II, typename val_type>
auto parCopyVec(II begin, II end, std::size_t parNum = 4) {
  std::vector<std::future<void>> futures;
  futures.reserve(parNum);
  const auto t = std::distance(begin, end);
  std::vector<val_type> ret(t);

  for (size_t i = 0; i < parNum; i++) {
    futures.push_back(std::async(std::launch::async, [&ret, &begin, i, t, parNum]() {
      std::copy(std::next(begin, (i * t) / parNum), std::next(begin, ((i + 1) * t) / parNum), std::next(ret.begin(), (i * t) / parNum));
    }));
  }

  return ret;
}

template <typename val_type>
auto parCopyMap(const val_type& src, std::size_t parNum = 4) {
  auto v = parCopyVec<decltype(src.begin()), std::pair<std::string, std::map<std::string, std::vector<uint64_t>>>>(src.begin(), src.end(), parNum);
  const auto t = src.size();
  val_type ret;
  ret.reserve(t);
  for (auto&& it : v) {
    ret.insert(std::move(it));
  }

  return ret;
}

int main() {
  const int iter_num = 3;
  const int orig_size = 599;
  const auto orig = setupMap(orig_size);
  for (size_t i = 0; i < iter_num; i++) {
    decltype(setupMap(358)) dest;
    {
      stopWatch t("copy");
      dest = orig;
    }
    dest.clear();
  }
  for (size_t i = 0; i < iter_num; i++) {
    decltype(setupMap(358)) dest;
    {
      stopWatch t("par 2 ");
      dest = parCopyMap(orig, 2);
    }
    dest.clear();
  }

  for (size_t i = 0; i < iter_num; i++) {
    decltype(setupMap(358)) dest;
    {
      stopWatch t("par 4 ");
      dest = parCopyMap(orig, 4);
    }
    dest.clear();
  }
  for (size_t i = 0; i < iter_num; i++) {
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

  const auto origV = setupVec(orig_size);
  for (size_t i = 0; i < iter_num; i++) {
    decltype(setupVec(358)) dest;
    {
      stopWatch t("copy");
      dest = origV;
    }
    dest.clear();
  }
  for (size_t i = 0; i < iter_num; i++) {
    decltype(setupVec(358)) dest;
    {
      stopWatch t("par 2 ");
      dest = parCopyVec<decltype(origV.begin()), std::map<std::string, std::vector<uint64_t>>>(origV.begin(), origV.end(), 2);
    }
    dest.clear();
  }
  for (size_t i = 0; i < iter_num; i++) {
    decltype(setupVec(358)) dest;
    {
      stopWatch t("par 4 ");
      dest = parCopyVec<decltype(origV.begin()), std::map<std::string, std::vector<uint64_t>>>(origV.begin(), origV.end(), 4);
    }
    dest.clear();
  }
  for (size_t i = 0; i < iter_num; i++) {
    decltype(setupVec(358)) dest;
    {
      stopWatch t("par 6 ");
      dest = parCopyVec<decltype(origV.begin()), std::map<std::string, std::vector<uint64_t>>>(origV.begin(), origV.end(), 6);
    }
    dest.clear();
  }
  {
    std::vector a(origV.begin(), origV.end());
    auto p = parCopyVec<decltype(origV.begin()), std::map<std::string, std::vector<uint64_t>>>(origV.begin(), origV.end(), 6);
    std::vector b(p.begin(), p.end());
    if (a != b) {
      std::cout << "failed" << std::endl;
    }
  }
}