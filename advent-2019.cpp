#include <iostream>
#include <list>
#include <vector>

using namespace std;

enum part { ONE, TWO };

list<std::string> input_lines;

int32_t fuel_required_for_mass(uint32_t mass) {
  return mass / 3 - 2;
}

void day0(part partNumber) {
  cout << "WELCOME TO ADVENT 2019\n";

  if (partNumber == TWO) {
    cout << "WELCOME TO ADVENT 2019... PART TWO\n";
  }
}

void day1(part partNumber) {
  uint32_t mass;
  uint32_t total_fuel = 0;

  for(auto &line : input_lines) {
    uint32_t mass = stoi(line);

    int32_t fuel_required_step = fuel_required_for_mass(mass);
    uint32_t total_fuel_required = 0;

    if (partNumber == ONE) {
      total_fuel_required = fuel_required_step;
    }
    else if (partNumber == TWO) {
      std::cout << "step: " << fuel_required_step << "\n";
      while(fuel_required_step > 0) {
        total_fuel_required += fuel_required_step;
        fuel_required_step = fuel_required_for_mass(fuel_required_step);
      }
    }
    std::cout << mass << ": total fuel required: " << total_fuel_required << "\n";

    total_fuel += total_fuel_required;
  }

  std::cout << "total required: " << total_fuel << "\n";
}

int main(const int argc, const char **argv) {
  string line;
  vector<void(*)(part)> days;

  days.push_back(day0);
  days.push_back(day1);

  while (std::cin >> line)
    input_lines.push_back(line);

  uint8_t day = 0;

  if (argc > 1) {
    uint8_t daynum = stoi(argv[1]);

    days[daynum](ONE);
    days[daynum](TWO);
  } else {
    for(auto &dayfunc : days) {
      dayfunc(ONE);
      dayfunc(TWO);
    }
  }
}

