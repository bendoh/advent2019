#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <regex>

using namespace std;

list<std::string> input_lines;

int32_t fuel_required_for_mass(uint32_t mass) {
  return mass / 3 - 2;
}

std::vector<std::string> regex_split(const std::string str, const std::string regex_str)
{
    std::regex regexz(regex_str);
    std::vector<std::string> list(std::sregex_token_iterator(str.begin(), str.end(), regexz, -1),
                                  std::sregex_token_iterator());
    return list;
}

void day0() {
  cout << "WELCOME TO ADVENT 2019\n";
}

void day1() {
  uint32_t mass;
  uint32_t total_fuel_part1 = 0;
  uint32_t total_fuel_part2 = 0;

  for(auto &line : input_lines) {
    uint32_t mass = stoi(line);

    int32_t fuel_for_mass = fuel_required_for_mass(mass);
    int32_t fuel_for_fuel = 0, fuel_step = fuel_for_mass;

    total_fuel_part1 += fuel_for_mass;

    while(fuel_step > 0) {
      fuel_for_fuel = fuel_step;
      fuel_step = fuel_required_for_mass(fuel_step);
    }
    std::cout << mass << ": total fuel required: " << total_fuel_required << "\n";

    total_fuel_part2 += fuel_for_mass + fuel_for_fuel;
  }

  std::cout << "total fuel needed for mass: " << total_fuel_part1 << "\n";
  std::cout << "total fuel needed for mass + fuel: " << total_fuel_part2 << "\n";
}

void print_program(vector<uint32_t> program) {
  for(uint32_t i = 0; i < program.size(); i++) {
    cout << "[" << to_string(i) << "] " << to_string (program[i]) << "  ";
  }
  cout << "\n";
}

void day2() {
  vector<uint32_t> program_template;

  for (auto &line : input_lines) {
    vector<string> line_opcodes = regex_split(line, ",");

    for (auto &opcode : line_opcodes) {
      program.push_back(stoi(opcode));
    }
  }

  uint32_t noun = 0;
  uint32_t verb = 0;

  uint32_t opcode = 0;
  uint32_t pc = 0;

  while(opcode != 99) {
    print_program(program);
    opcode = program[pc];

    cout << "PC: " << to_string(pc) << " opcode: " << to_string(opcode) << "\n";

    pc++;

    if(opcode == 1) { // Addition
      uint32_t arg1 = program[program[pc++]];
      uint32_t arg2 = program[program[pc++]];
      uint32_t dest = program[pc++];

      cout << "ADD: " << to_string(arg1) << " + " << to_string(arg2) << " = " << to_string(arg1+arg2) << " -> " << to_string(dest) << "\n";
      program[dest] = arg1 + arg2;
    }

    if(opcode == 2) { // Multiplication, 3 args
      uint32_t arg1 = program[program[pc++]];
      uint32_t arg2 = program[program[pc++]];
      uint32_t dest = program[pc++];

      cout << "MULTIPLY: " << to_string(arg1) << " * " << to_string(arg2) << " = " << to_string(arg1*arg2) << " -> " << to_string(dest) << "\n";
      program[dest] = arg1 * arg2;
    }


    opcode = program[pc];
  }
  print_program(program);
}

int main(const int argc, const char **argv) {
  string line;
  vector<void(*)(part)> days;

  days.push_back(day0);
  days.push_back(day1);
  days.push_back(day2);

  while (std::cin >> line)
    input_lines.push_back(line);

  if (argc > 1) {
    uint32_t daynum = stoi(argv[1]);

    days[daynum]();
  } else {
    for(auto &dayfunc : days) {
      dayfunc();
    }
  }
}

