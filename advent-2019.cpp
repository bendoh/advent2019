#include <stdio.h>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <regex>
#include <cstdlib>

using namespace std;

list<std::string> input_lines;
bool is_interactive = false;
bool is_visual = false;

FILE *terminal;

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
    std::cout <<
      "Mass=" << mass <<
      "; fuel required for mass: " << fuel_for_mass <<
      "; fuel required for fuel: " << fuel_for_fuel <<
      "\n";

    total_fuel_part2 += fuel_for_mass + fuel_for_fuel;
  }

  std::cout << "total fuel needed for mass: " << total_fuel_part1 << "\n";
  std::cout << "total fuel needed for mass + fuel: " << total_fuel_part2 << "\n";
}

void print_program(vector<uint32_t> program) {
  cout << "\033[2J";
  cout << "\033[H";
  for(uint32_t i = 0; i < 8; i++) {
    printf("%8d  ", i);
  }
  for(uint32_t i = 0; i < program.size(); i++) {
    if (i % 8 == 0)
      cout << "\n";
    printf("%8d  ", program[i]);
  }
  cout << "\n";
}


/** Day2 **/
uint32_t day2_intcode_processor(vector<uint32_t> program) {
  uint32_t opcode = 0;
  uint32_t pc = 0;
  string input;

  while(opcode != 99) {
    if (is_visual || is_interactive) {
      print_program(program);
    }

    if (is_interactive) {
      fgetc(terminal);
    }

    opcode = program[pc];

    pc++;

    if(opcode == 1) { // Addition
      uint32_t arg1 = program[program[pc++]];
      uint32_t arg2 = program[program[pc++]];
      uint32_t dest = program[pc++];
      program[dest] = arg1 + arg2;
    }

    if(opcode == 2) { // Multiplication
      uint32_t arg1 = program[program[pc++]];
      uint32_t arg2 = program[program[pc++]];
      uint32_t dest = program[pc++];
      program[dest] = arg1 * arg2;
    }


    opcode = program[pc];
  }

  return program[0];
}

void day2() {
  vector<uint32_t> program_template;

  for (auto &line : input_lines) {
    vector<string> line_opcodes = regex_split(line, ",");

    for (auto &opcode : line_opcodes) {
      program_template.push_back(stoi(opcode));
    }
  }

  uint32_t noun = 12;
  uint32_t verb = 2;

  vector<uint32_t> program = program_template;
  program[1] = noun;
  program[2] = verb;

  uint32_t part1_result = day2_intcode_processor(program);

  printf("With noun=12 & verb=2: %i\n", part1_result);

  uint32_t part2_search = 19690720;

  for (noun = 0; noun < 99; noun++) {
    for (verb = 0; verb < 99; verb++) {
      program = program_template;
      program[1] = noun;
      program[2] = verb;
      uint32_t result = day2_intcode_processor(program);

      if (is_visual)
        printf("(%d, %d) = %d\n", noun, verb, result);

      if (result == part2_search) {
        printf("Found %d! %d * 100 + %d = %d\n", part2_search, noun, verb, noun * 100 + verb);
        return;
      }
    }
  }
}

int main(const int argc, const char **argv) {
  string line;
  vector<void(*)()> days;

  days.push_back(day0);
  days.push_back(day1);
  days.push_back(day2);

  if (strcmp(getenv("AOC_INTERACTIVE"), "ON") == 0) {
    is_interactive = true;
    cout << "Doing these interactively! Enjoy...\n";
    terminal = fopen("/dev/tty", "r");

    if (!terminal) {
      cout << "Couldn't open terminal! Why?\n";
      return 1;
    }
  }

  const char *env_visual = getenv("AOC_VISUAL");

  if (env_visual != NULL && strcmp(env_visual, "ON") == 0) {
    is_visual = true;
  }

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

