#include "csv_reader.hpp"

std::vector<BenchmarckResult> read_csv(){
  std::ifstream csv("benchMark.csv");
  if(!csv.is_open()){
    std::cerr << "Error opening benchMarch.csv\n";
  }
  std::string line;

  std::getline(csv, line);

  std::vector<BenchmarckResult> results;
  while(std::getline(csv, line)){
    std::stringstream ss(line);

    BenchmarckResult result;
    std::getline(ss, result.algorithm, ',');
    std::getline(ss, result.testCase, ',');

    ss >> result.n;
    ss.ignore(1, ',');
    ss >> result.timeUs;

    results.push_back(result);
  }
  return results;
}
