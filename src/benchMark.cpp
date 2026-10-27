#include <ratio>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <ratio>
#include <chrono>
#include <random>
#include <iostream>

void bubble(std::vector<int>& a) {
  int n = (int)a.size();

  for (int i = n - 1; i > 0; --i) {
    bool swapped = false;

    for (int j = 0; j < i; ++j) {
      if (a[j] > a[j + 1]) {
        std::swap(a[j], a[j + 1]);
        swapped = true;
      }
    }

    if (!swapped) break;
  }
}

void selection(std::vector<int>& a) {
  int n = (int)a.size();

  for (int i = 0; i < n - 1; ++i) {
    int minimum = i;

    for (int j = i + 1; j < n; ++j) {
      if (a[j] < a[minimum])
        minimum = j;
    }

    if (minimum != i)
      std::swap(a[i], a[minimum]);
  }
}

void insertion(std::vector<int>& a) {
  int n = (int)a.size();

  for (int i = 1; i < n; ++i) {
    int j = i;

    while (j > 0 && a[j - 1] > a[j]) {
      std::swap(a[j - 1], a[j]);
        --j;
    }
  }
}

using sortFonction = void (*)(std::vector<int>&);
struct Algorithm{
  std::string name;
  sortFonction sort;
};

void loadBenchmark(){
  std::vector<Algorithm> algorithms = {
    {"Bubble", bubble},
    {"Selection", selection},
    {"Insertion", insertion}
  };

  std::vector<int> sizes = {100, 250, 500, 1000, 1500, 2000, 3000, 4000};
  std::vector<std::string> cases = { "best", "worst", "random"};

  std::ofstream csv("benchMark.csv");
  csv << "algorithm,case,n,time_us\n";

  for(int n :sizes){
    std::vector<int> sorted(n);
    std::iota(sorted.begin(), sorted.end(), 1);

    std::vector<int> reversed = sorted;
    std::reverse(reversed.begin(), reversed.end());

    std::default_random_engine engine(45);

    std::vector<int> random = sorted;
    std::shuffle(random.begin(), random.end(), engine);

    std::vector<std::vector<int>> inputs = {sorted, reversed, random};

    for(int c = 0; c < (int)cases.size(); c++){
      for(const auto& algorithm : algorithms){
        int repetitions = 5;
        double totalMiniseconds = 0;

        for(int i = 0; i < repetitions; i++){
          std::vector<int> data = inputs[c];

          auto begin = std::chrono::steady_clock::now();
          algorithm.sort(data);
          auto end = std::chrono::steady_clock::now();

          totalMiniseconds += std::chrono::duration<double, std::micro> (end - begin).count();

          if(!std::is_sorted(data.begin(), data.end())){
            csv << "Error de tri:"
                << algorithm.name << "\n";
          }
        }

        double average = totalMiniseconds / repetitions;

        csv << algorithm.name << ','
            << cases[c] << ','
            << n << ','
            << average << '\n';
      }
    }
  }
  std::cout << "Resultats ecrits dans benchmark.csv\n";
}
