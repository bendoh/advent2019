#include <numeric>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <regex>
#include <cstdlib>
#include <functional>
#include <algorithm>
#include <stdlib.h>
#include <math.h>
#include <tgmath.h>
#include <chrono>
#include <tuple>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine/olcPixelGameEngine.h"

using namespace std;

vector<std::string> input_lines;
bool is_interactive = false;

// Are we running in a window or just in a terminal?
bool is_visual = false;

bool debug_intcode;
uint8_t day16_phases = 100;

// Border in SCREEN space
uint16_t border = 10;

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


const char *trim_ws = " \t\n\r\f\v";

uint16_t map_scale = 5;

// trim from end of string (right)
inline std::string& rtrim(std::string& s, const char* t = trim_ws)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}


/** Day 3! **/
struct Point {
  int32_t x = 0;
  int32_t y = 0;
  int32_t z = 0;

  int32_t manhattan_distance() { return abs(x) + abs(y) + abs(z); }

  Point() : Point(0, 0, 0) {}

  Point(int32_t _x, int32_t _y) {
    x = _x;
    y = _y;
  }
  Point(int32_t _x, int32_t _y, int32_t _z) : Point(_x, _y) {
    z = _z;
  }

  bool operator==(const Point &o) { return x == o.x && y == o.y && z == o.z; }
};


struct Intersection {
  Point point = { 0, 0 };
  uint32_t distance = 0;
};


struct Asteroid {
  Point point = { 0, 0 };
  double distance = 0;

  bool operator<(const Asteroid rhs) const {
      return distance < rhs.distance;
  }
};

void parse_environment() {
  const char *env_interactive = getenv("AOC_INTERACTIVE");

  if (env_interactive != NULL && strcmp(env_interactive, "ON") == 0) {
    is_interactive = true;
    cout << "Doing these interactively! Enjoy...\n";
    terminal = fopen("/dev/tty", "r");

    if (!terminal) {
      cout << "Couldn't open terminal! Why?\n";
      exit(1);
    }
  }

  const char *env_visual = getenv("AOC_VISUAL");

  if (env_visual != NULL && strcmp(env_visual, "ON") == 0) {
    is_visual = true;
  }

  const char *env_debug_intcode = getenv("AOC_DEBUG_INTCODE");

  if (env_debug_intcode != NULL && strcmp(env_debug_intcode, "ON") == 0) {
    debug_intcode = true;
    cout << "Debugging Intcode programs!\n";
  }

  const char *env_day16_phases = getenv("AOC_DAY16_PHASES");

  if (env_day16_phases != NULL) {
    uint8_t int_value = atoi(env_day16_phases);

    if (int_value > 0) {
      day16_phases = int_value;

      printf("Overridding day 16 phases to %d\n", day16_phases);
    }
  }
}


class Advent2019 : public olc::PixelGameEngine
{
public:
  string error_state;

  map<uint8_t, string>results;

  string day0() {
    day_complete = true;
    return "WELCOME TO ADVENT 2019\n";
  }

  string day1() {
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

    char result[255];
    snprintf(result, 255,
      "total fuel needed for all mass: %d\ntotal fuel needed for all mass + all fuel: %d",
      total_fuel_part1,
      total_fuel_part2
    );

    day_complete = true;
    return string(result);
  }

  /** Day2 **/
  void print_program(vector<int32_t> program, uint32_t pc) {
    cout << "\033[2J";
    cout << "\033[H\033[7m         ";
    for(uint32_t i = 0; i < 8; i++) {
      printf("%02x        ", i);
    }
    cout << "\033[0m";
    for(uint32_t i = 0; i < program.size(); i++) {
      if (i % 8 == 0) {
        cout << "\n";
        printf("\033[7m%02x\033[0m ", i);
      }

      if (i == pc) {
        printf("\033[5m%8d\033[0m  ", program[i]);
      }
      else {
        printf("%8d  ", program[i]);
      }
    }
    printf("\nPC is at %d\n", pc);

  }

  int32_t day2_intcode_processor(vector<int32_t> program) {
    uint32_t opcode = 0;
    uint32_t pc = 0;
    string input;

    while(opcode != 99) {
      if (debug_intcode) {
        print_program(program, pc);
      }

      if (is_interactive) {
        fgetc(terminal);
      }

      opcode = program[pc];

      pc++;

      if(opcode == 1) { // Addition
        int32_t arg1 = program[program[pc++]];
        int32_t arg2 = program[program[pc++]];
        uint32_t dest = program[pc++];
        program[dest] = arg1 + arg2;
      }

      if(opcode == 2) { // Multiplication
        int32_t arg1 = program[program[pc++]];
        int32_t arg2 = program[program[pc++]];
        uint32_t dest = program[pc++];
        program[dest] = arg1 * arg2;
      }

      opcode = program[pc];
    }

    return program[0];
  }

  string day2() {
    vector<int32_t> program_template;

    for (auto &line : input_lines) {
      vector<string> line_opcodes = regex_split(line, ",");

      for (auto &opcode : line_opcodes) {
        program_template.push_back(stoi(opcode));
      }
    }

    uint32_t noun = 12;
    uint32_t verb = 2;

    vector<int32_t> program = program_template;
    program[1] = noun;
    program[2] = verb;

    int32_t result_1 = day2_intcode_processor(program);
    char part1_result[255], part2_result[255];
    snprintf(part1_result, 255, "Part 1: With noun=12 & verb=2: %i", result_1);
    cout << part1_result << "\n";

    uint32_t part2_search = 19690720;

    for (noun = 0; noun < 99; noun++) {
      for (verb = 0; verb < 99; verb++) {
        program = program_template;
        program[1] = noun;
        program[2] = verb;
        int32_t result_2 = day2_intcode_processor(program);

        if (is_visual)
          printf("(%d, %d) = %d\n", noun, verb, result_2);

        if (result_2 == part2_search) {
          snprintf(part2_result, 255, "Part 2: Found %d! %d * 100 + %d = %d", part2_search, noun, verb, noun * 100 + verb);
          cout << part2_result << "\n";
          break;
        }
      }
    }

    day_complete = true;
    return string(part1_result) + "\n" + string(part2_result);
  }

  const uint16_t day3_speed = 100;

  void draw_wire(vector<Point> wire, olc::Pixel p) {

    printf("Day time is %f, wire size is %lu\n", day_time, wire.size());

    size_t num_segments = max((size_t) 1, min(wire.size() - 1, (size_t) (day_time * day3_speed)));

    printf("Drawing %lu segments, first point is (%d, %d)\n", num_segments, wire[0].x, wire[0].y);

    for(uint32_t i = 0; i < num_segments; i++) {
      olc::vi2d u((wire[i].x - offset.x) / scale.x, (wire[i].y - offset.y) / scale.y);
      olc::vi2d v((wire[i+1].x - offset.x) / scale.x, (wire[i+1].y - offset.y) / scale.y);
      if (i == 0) {
        FillCircle(u.x, u.y, 6, olc::CYAN);
      }

//      printf("Drawing from (%d, %d) -> (%d, %d) to (%d, %d) -> (%d, %d)\n", wire[i].x, wire[i].y, u.x, u.y, wire[i+1].x, wire[i+1].y, v.x, v.y);

      DrawLine(u, v, p);
    }
  }

  olc::vi2d offset;
  olc::vf2d scale;

  // Figure out the proper scale and offset to fit the world space
  // perfectly in our window
  void fit_world(vector<vector<Point>> wires) {
    olc::vi2d southwest(0, 0), northeast(0, 0);

    for (auto &wire: wires) {
      for(uint32_t i = 0; i < wire.size(); i++) {
        Point u = wire[i];

        if(u.x < southwest.x) southwest.x = u.x;
        if(u.y < southwest.y) southwest.y = u.y;
        if(u.x > northeast.x) northeast.x = u.x;
        if(u.y > northeast.y) northeast.y = u.y;
      }
    }

    uint32_t distanceX = (northeast.x - southwest.x),
             distanceY = (northeast.y - southwest.y);

    // Scale as separate (X,Y) to conform to whatever screen dimensions we have
    scale = olc::vf2d(
      abs((float) distanceX / (float) (ScreenWidth() - border * 2)),
      abs((float) distanceY / (float) (ScreenHeight() - border * 2))
    );

    // Offset in WORLD space
    offset = olc::vi2d(southwest.x - border * scale.y, southwest.y - border * scale.y);
  }

  string day3() {
    vector<vector<Point>> wires;
    int num_wires = input_lines.size();

    for (uint8_t wire_num = 0; wire_num < num_wires; wire_num++) {
      string line = input_lines[wire_num];

      vector<string> directions = regex_split(line, ",");
      vector<Point> points;
      Point point;
      points.push_back(point);
      Point next = point;

      for (string &step : directions) {
        char dircode = step[0];
        int distance = stoi(step.substr(1));

        switch (dircode) {
          case 'R':
            next.x += distance;
            break;
          case 'U':
            next.y -= distance;
            break;
          case 'D':
            next.y += distance;
            break;
          case 'L':
            next.x -= distance;
            break;
        }

        points.push_back(next);
        point = next;
      }

      wires.push_back(points);
    }

    // Search for intersections by checking each vertex of wire1
    // for a crossing with wire2
    list<Intersection> crossings;

    size_t max_segments = max(wires[0].size(), wires[1].size());
    size_t num_segments = max((size_t) 1, min(max_segments, (size_t) (day_time * day3_speed)));

    fit_world(wires);
    draw_wire(wires[0], olc::RED);
    draw_wire(wires[1], olc::BLUE);

    uint32_t distance_a = 0, distance_b = 0;

    for(uint32_t i = 0; i < min(num_segments, wires[0].size()) - 1; i++) {
      Point u1 = wires[0][i];
      Point u2 = wires[0][i+1];

      distance_a += abs(u1.x - u2.x) + abs(u1.y - u2.y);
      distance_b = 0;

      for(uint32_t j = 0; j < min(num_segments, wires[1].size()) - 1; j++) {
        Point v1 = wires[1][j];
        Point v2 = wires[1][j+1];

        distance_b += abs(v1.x - v2.x) + abs(v1.y - v2.y);
        if(u1.x == u2.x && v1.y == v2.y) {
          int32_t Ux = u1.x, Vy = v1.y;
          Intersection crossing;
          crossing.point.x = Ux;
          crossing.point.y = Vy;
          crossing.distance = distance_a + distance_b;

          int32_t uy1, uy2;
          if(u1.y < u2.y) {
            uy1 = u1.y;
            uy2 = u2.y;
          } else {
            uy2 = u1.y;
            uy1 = u2.y;
          }

          int32_t vx1, vx2;
          if(v1.x < v2.x) {
            vx1 = v1.x;
            vx2 = v2.x;
          } else {
            vx2 = v1.x;
            vx1 = v2.x;
          }

          if (uy1 <= Vy && Vy <= uy2 &&  // U crosses Vy
              vx1 <= Ux && Ux <= vx2) { // V crosses Ux
            crossing.distance -= abs(u2.y - Vy) + abs(v2.x - Ux);
            DrawCircle((Ux - offset.x) / scale.x, (Vy - offset.y) / scale.y, 5, olc::GREEN);
            crossings.push_back(crossing);
          }
        }
        else if(u1.y == u2.y && v1.x == v2.x) {
          // U is horizontal, V is vertical
          int32_t Uy = u1.y, Vx = v1.x;
          Intersection crossing;
          crossing.point.x = Vx;
          crossing.point.y = Uy;
          crossing.distance = distance_a + distance_b;

          int32_t vy1, vy2;
          if(v1.y < v2.y) {
            vy1 = v1.y;
            vy2 = v2.y;
          } else {
            vy2 = v1.y;
            vy1 = v2.y;
          }

          int32_t ux1, ux2;
          if(u1.x < u2.x) {
            ux1 = u1.x;
            ux2 = u2.x;
          } else {
            ux2 = u1.x;
            ux1 = u2.x;
          }

          if (vy1 < Uy && Uy <= vy2 && // V crosses Uy
              ux1 < Vx && Vx <= ux2) { // U crosses Vx
            crossing.distance -= abs(u2.x - Vx) + abs(v2.y - Uy);
            DrawCircle((Vx - offset.x) / scale.x, (Uy - offset.y) / scale.y, 5, olc::GREEN);
            crossings.push_back(crossing);
          }
        }
      }
    }

    Intersection *nearest = nullptr;
    uint32_t fewest_steps = INT_MAX;
    for (auto &crossing: crossings) {
      if(crossing.point.x != 0 || crossing.point.y != 0) {
        uint32_t distance = crossing.point.manhattan_distance();

        printf(
            "%d, %d: distance=%d, %d steps\n",
            crossing.point.x, crossing.point.y,
            distance, crossing.distance
        );
        if (nearest == nullptr || distance < nearest->point.manhattan_distance())
          nearest = &crossing;

        if (crossing.distance < fewest_steps)
          fewest_steps = crossing.distance;
      }
    }

    if (num_segments == max_segments) {
      day_complete = true;
    }

    if (nearest != nullptr) {
      FillCircle((nearest->point.x - offset.x) / scale.x, (nearest->point.y - offset.y) / scale.y, 3, olc::CYAN);

      string result =
        "Nearest: " + to_string(nearest->point.manhattan_distance()) + "\n"
        "Fewest steps: " + to_string(fewest_steps);
      return result;
    } else {
      return "";
    }
  }

  string day4() {
    vector<string> range = regex_split(input_lines[0], "-");
    vector<string> part1_matches, part2_matches;
    uint32_t lb = atoi(range[0].c_str()), ub = atoi(range[1].c_str());

    for(uint32_t i = lb; i <= ub; i++) {
      string s = to_string(i);
      char doubled_digit = 0;
      bool has_double = false;
      bool no_decrease = true;

      for (uint32_t v = 0; v < s.length() - 1; v++) {
        if(s[v] == s[v+1]) {
          has_double = true;
          if(
              (v == 0 || s[v-1] != s[v]) &&
              (v == s.length() - 2 || s[v+2] != s[v])
          )
            doubled_digit = s[v];
        }
        if(s[v] > s[v+1]) no_decrease = false;
      }

      if (has_double && no_decrease) {
        part1_matches.push_back(s);
      }

      if (no_decrease && doubled_digit) {
        part2_matches.push_back(s);
      }
    }

    day_complete = true;
    return (
      "Part1: " + to_string(part1_matches.size()) + "\n" +
      "Part2: " + to_string(part2_matches.size())
    );

  }

  int32_t day5_intcode_processor(vector<int32_t> program, vector<int32_t> inputs) {
    uint32_t opcode = 0;
    uint32_t pc = 0;
    int32_t val = 0;
    uint32_t input_count = 0;

    while(opcode != 99) {
      if (debug_intcode) {
        print_program(program, pc);
      }

      if (is_interactive) {
        fgetc(terminal);
      }

      opcode = program[pc] % 100;
      uint32_t mode = program[pc] / 100;
      pc++;

      bool is_immediate[3] = {
        // Positions 1..N
        (bool) (mode & 1),
        (bool) (mode / 10 & 1),
        (bool) (mode / 100 & 1)
      };

      if(opcode == 1) { // Addition
        int32_t arg1 = is_immediate[0] ? program[pc] : program[program[pc]];
        pc++;
        int32_t arg2 = is_immediate[1] ? program[pc] : program[program[pc]];
        pc++;
        uint32_t dest = program[pc++];
        program[dest] = arg1 + arg2;
      }

      else if(opcode == 2) { // Multiplication
        int32_t arg1 = is_immediate[0] ? program[pc] : program[program[pc]];
        pc++;
        int32_t arg2 = is_immediate[1] ? program[pc] : program[program[pc]];
        pc++;
        uint32_t dest = program[pc++];
        program[dest] = arg1 * arg2;
      }

      else if (opcode == 3) {
        uint32_t position = program[pc];
        pc++;
        program[position] = inputs[input_count];
        input_count++;
      }

      else if (opcode == 4) {
        uint32_t position = program[pc];
        pc++;
        val = program[position];
        printf("Value at position %d: %d\n", position, val);
      }

      else if (opcode == 5 || opcode == 6) {
        int32_t arg1 = is_immediate[0] ? program[pc] : program[program[pc]];
        pc++;
        int32_t arg2 = is_immediate[1] ? program[pc] : program[program[pc]];
        pc++;

        if((opcode == 5 && arg1 != 0) || (opcode == 6 && arg1 == 0)) {
          pc = arg2;
        }
      }

      else if (opcode == 7 || opcode == 8) {
        int32_t arg1 = is_immediate[0] ? program[pc] : program[program[pc]];
        pc++;
        int32_t arg2 = is_immediate[1] ? program[pc] : program[program[pc]];
        pc++;
        uint32_t dest = program[pc++];
        program[dest] = (opcode == 7 && arg1 < arg2) || (opcode == 8 && arg1 == arg2) ? 1 : 0;
      }

      opcode = program[pc];
    }

    return val;
  }


  string day5() {
    vector<int32_t> program_template;

    printf("Parsing lines...\n");
    for (auto &line : input_lines) {
      vector<string> line_opcodes = regex_split(line, ",");

      for (auto &opcode : line_opcodes) {
        program_template.push_back(stoi(opcode));
      }
    }
    printf("Parsed %lu instructions\n", program_template.size());

    vector<int32_t> program = program_template;

    char user_input[10];
    if (is_interactive) {
      cout << "Input? ";
      fscanf(terminal, "%10s", user_input);
      int32_t result_1 = day5_intcode_processor(program, { atoi(user_input) });

      return to_string(result_1);
    } else {
      int32_t result_1 = day5_intcode_processor(program, { 1 });
      int32_t result_2 = day5_intcode_processor(program, { 5 });

      day_complete = true;
      return to_string(result_1) + "\n" + to_string(result_2);
    }
  }

  map<string, vector<string>> orbits;

  uint32_t count_orbits(string root, uint32_t depth) {
    uint32_t count = 0;

    try {
      vector<string> children = orbits.at(root);

      for (auto &f: children) {
        cout << "Child of root " << root << ": " << f << "\n";
        count += depth + count_orbits(f, depth + 1);
      }
    }
    catch (const std::out_of_range& oor) {
    }

    return count;
  }

  uint32_t find_distance(string start, string end, uint32_t distance) {
    try {
      vector<string> children = orbits.at(start);

      for (auto &f: children) {
        if (f == end) {
          return distance;
        }

        uint32_t c_distance = find_distance(f, end, distance + 1);

        if (c_distance > 0) {
          return c_distance;
        }
      }
    }
    catch (const std::out_of_range& oor) {
    }

    return 0;
  }

  string day6() {
    printf("Parsing lines...\n");
    for (auto &line : input_lines) {
      rtrim(line);
      vector<string> r = regex_split(line, "\\)");
      cout << "Adding " << r[0] << " <- " << r[1] << "\n";
      orbits[r[0]].push_back(r[1]);
    }

    uint32_t num_orbits = count_orbits("COM", 1);

    string result = to_string(num_orbits);

    uint32_t min_distance = INT_MAX;

    map<string, vector<string>>::iterator it;

    for (it = orbits.begin(); it != orbits.end(); it++) {
      uint32_t santa_distance = find_distance(it->first, "SAN", 0);
      uint32_t you_distance = find_distance(it->first, "YOU", 0);

      if(santa_distance > 0 && you_distance > 0 && santa_distance + you_distance < min_distance) {
        min_distance = santa_distance + you_distance;
      }
    }

    day_complete = true;

    return result + "; distance between SAN and YOU: " + to_string(min_distance);
  }

  vector<vector<int8_t>> permutations;

  void get_permutations(vector<uint8_t> choices, uint8_t pos, vector<int8_t> perm) {
    if (choices.size() == 0) {
      permutations.push_back(perm);
    }
    for(uint32_t i = 0; i < choices.size(); i++) {
      uint8_t choice = choices[i];
      vector<uint8_t> next_choices = choices;
      vector<int8_t> next_permutation = perm;
      printf("Choice for %d is %d\n", i, choice);
      next_permutation[pos] = choice;
      next_choices.erase (next_choices.begin()+i);
      get_permutations(next_choices, pos+1, next_permutation);
    }
  }

  typedef int64_t memtype;

  enum State { INPUT, HALTED };
  struct IntcodeProgram {
    vector<memtype> program;
    uint32_t pc = 0;
    State state = INPUT;
    vector<memtype> result = {};
    int32_t relative_base = 0;
  };

  enum Mode { ABSOLUTE, IMMEDIATE, RELATIVE };

  uint32_t fetch_target(IntcodeProgram *p, Mode mode) {
    uint32_t target = 0;
    switch(mode) {
      case ABSOLUTE:
        target = p->program[p->pc++];
        break;
      case IMMEDIATE:
        target = p->pc++;
        break;
      case RELATIVE:
        target = p->relative_base + p->program[p->pc++];
        break;
      default:
        printf("Invalid fetch mode found in program! Bye!\n");
        exit(1);
    }

    return target;
  }

  memtype fetch_param(IntcodeProgram *p, Mode mode) {
    uint32_t target = fetch_target(p, mode);

    if (debug_intcode)
      printf(" %s [%d] = %lld ",
          mode == ABSOLUTE ? "ABS" : mode == IMMEDIATE ? "IMM" : "REL",
          target,
          p->program[target]
      );

    return p->program[target];
  }

  map <memtype, string> opcodes = {
    {1, "ADD"},
    {2, "MUL"},
    {3, "LOAD"},
    {4, "OUT"},
    {5, "JNZ"},
    {6, "JEZ"},
    {7, "SLT"},
    {8, "SEQ"},
    {9, "SRB"},
    {99, "HALT"}
  };

  void intcode_processor(IntcodeProgram *p, memtype input) {
    memtype opcode = 0;
    bool consumed_input = false;

    while(1) {
      memtype intcode = p->program[p->pc];
      p->pc++;

      if (is_interactive) {
        fgetc(terminal);
      }


      if (debug_intcode)
        printf("[%d] [%lld] [%s] ",
            p->pc,
            intcode,
            opcodes[opcode].c_str()
        );


      if (opcode == 99) {
        if (debug_intcode)
          printf("\n");
        break;
      }

      opcode = intcode % 100;
      memtype mode = intcode / 100;

      Mode modes[3] = {
        // Positions 1..N
        (Mode) (mode % 10),
        (Mode) ((mode / 10) % 10),
        (Mode) ((mode / 100) % 10)
      };

      if (opcode == 99)
        break;

      if (debug_intcode)
        printf("[%d] [%lld] [%s] ",
            p->pc,
            intcode,
            opcodes[opcode].c_str()
        );

      if(opcode == 1) { // Addition
        memtype
          arg1 = fetch_param(p, modes[0]),
          arg2 = fetch_param(p, modes[1]),
          dest = fetch_target(p, modes[2]);
        p->program[dest] = arg1 + arg2;

        if (debug_intcode)
          printf(" %lld + %lld = %lld -> program[%lld]", arg1, arg2, p->program[dest], dest);
      }

      else if(opcode == 2) { // Multiplication
        memtype
          arg1 = fetch_param(p, modes[0]),
          arg2 = fetch_param(p, modes[1]),
          dest = fetch_target(p, modes[2]);
        p->program[dest] = arg1 * arg2;

        if (debug_intcode)
          printf(" %lld * %lld = %lld -> program[%lld]", arg1, arg2, p->program[dest], dest);
      }

      else if (opcode == 3) {
        if (!consumed_input) {
          uint32_t position;
          position = p->program[p->pc++];
          if (modes[0] == RELATIVE) {
            position += p->relative_base;
          }
          p->program[position] = input;

          if (debug_intcode)
            printf(" p->program[%d] = %lld", position, input);

          consumed_input = true;
        } else {
          p->state = INPUT;
          p->pc--; // Restart here
          if (debug_intcode)
            printf("Waiting for next input...\n");
          return;
        }
      }

      else if (opcode == 4) {
        memtype output = fetch_param(p, modes[0]);
        p->result.push_back(output);

        if (debug_intcode)
          printf(" OUTPUT %lld", p->result.back());
      }

      else if (opcode == 5 || opcode == 6) {
        memtype arg1 = fetch_param(p, modes[0]),
                dest = fetch_param(p, modes[1]);

        if (debug_intcode)
          printf(" %lld %s 0 -> %lld", arg1, opcode == 5 ? "!=" : "==", dest);

        if((opcode == 5 && arg1 != 0) || (opcode == 6 && arg1 == 0)) {
          if (debug_intcode)
            printf("; Jumping to %lld",  dest);
          p->pc = dest;
        }
      }

      else if (opcode == 7 || opcode == 8) {
        memtype arg1 = fetch_param(p, modes[0]),
                arg2 = fetch_param(p, modes[1]),
                dest = fetch_target(p, modes[2]);

        p->program[dest] = (opcode == 7 && arg1 < arg2) || (opcode == 8 && arg1 == arg2) ? 1 : 0;

        if (debug_intcode)
          printf(" %lld %s %lld = %lld -> %lld\n", arg1, opcode == 7 ? "<" : "==", arg2, p->program[dest], dest);
      }

      else if (opcode == 9) {
        memtype arg1 = fetch_param(p, modes[0]);

        if (debug_intcode)

          printf(" %d", p->relative_base);
        p->relative_base += arg1;

        if (debug_intcode)
          printf(" += %lld => %d", arg1, p->relative_base);
      }

      else {
        printf("[ERROR] Invalid opcode %lld!\n", opcode);
      }

      if (debug_intcode)
        printf("\n");
      opcode = p->program[p->pc];
    }

    p->state = HALTED;
  }

  string day7() {
    parse_program();
    vector<memtype> program_template = parsed_program;

    vector<uint8_t> choices = { 0, 1, 2, 3, 4 };
    vector<int8_t> permutation = { -1, -1, -1, -1, -1 };

    uint32_t max_output = 0;
    get_permutations(choices, 0, permutation);

    // Find max output and permutation which triggers it
    for(int i = 0; i < permutations.size(); i++) {
      auto perm = permutations[i];
      int32_t input = 0;

      printf("%i: %d %d %d %d %d\n", i, perm[0], perm[1], perm[2], perm[3], perm[4]);

      for(int j = 0; j < 5; j++) {
        uint8_t phase = perm[j];
        vector<int32_t> program;

        for(int k = 0; k < program_template.size(); k++) {
          program.push_back(program_template[k]);
        }

        input = day5_intcode_processor(program, { phase, input });
      }

      printf("Output for this permutation: %d\n", input);

      if (input > max_output) {
        max_output = input;
      }
    }

    vector<uint8_t> part2_choices = { 5, 6, 7, 8, 9 };
    permutation = { -1, -1, -1, -1, -1 };
    memtype max_feedback_output = 0;
    get_permutations(part2_choices, 0, permutation);

    for(int i = 0; i < permutations.size(); i++) {
      vector<int8_t> perm = permutations[i];
      printf("%i: %d %d %d %d %d\n", i, perm[0], perm[1], perm[2], perm[3], perm[4]);

      IntcodeProgram programs[5];

      for(int j = 0; j < 5; j++) {
        programs[j].program = program_template;
        // printf("Initializing stage %d with %d\n", j, perm[j]);
        intcode_processor(&programs[j], perm[j]);
      }

      IntcodeProgram *cp = &programs[0];
      memtype input = 0;
      uint32_t stage = 0;
      uint32_t num_halted = 0;

      while(1) {
        //printf("Running stage %d with input %lld, pc=%d\n", stage, input, cp->pc);
        intcode_processor(cp, input);
        input = cp->result.back();
        cp->result.pop_back();
        //printf("Stage %d stopped with status %d and result %lld\n", stage, cp->state, input);
        stage = (stage + 1) % 5;

        if(cp->state == HALTED) {
          num_halted ++;
          //printf("Stage %d, halted with result %lld\n", stage, input);

          if(num_halted == 5)
            break;
        }

        cp = &programs[stage];
      }

      if (input > max_feedback_output) {
        max_feedback_output = input;
      }
    }

    day_complete = true;
    return "Max output: " + to_string(max_output) + "\n"
      + "Max feedback output: " + to_string(max_feedback_output);
  }

  string day8() {
    day_complete = 1;

    uint8_t width = 25;
    uint8_t height = 6;
    string input = input_lines[0];

    vector<vector<uint8_t>> layers;

    for(uint32_t i = 0; i < input.size(); i++) {
      uint32_t layer_idx = i / (width * height);

      if(layer_idx == layers.size()) {
        layers.push_back(vector<uint8_t>());
      }

      layers[layer_idx].push_back(input[i] - '0');
    }
    layers.pop_back();

    int32_t layer_with_fewest_zeros = -1;
    int32_t fewest_zero_count = INT_MAX;

    vector<vector<uint32_t>> layer_digit_counts;

    for(uint32_t layer_idx = 0; layer_idx < layers.size(); layer_idx++) {
      printf("\nLayer %d", layer_idx);
      vector<uint8_t> layer = layers[layer_idx];
      vector<uint32_t> digit_counts = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

      for(uint32_t i = 0; i < layer.size(); i++) {
        if(i % width == 0) { printf("\n"); }
        printf("%d", layer[i]);

        digit_counts[layer[i]]++;
      }

      if(digit_counts[0] < fewest_zero_count) {
        fewest_zero_count = digit_counts[0];
        layer_with_fewest_zeros = layer_idx;
      }

      layer_digit_counts.push_back(digit_counts);
      printf("\n");
    }

    vector<uint8_t> merged_result = layers[0];

    for(uint8_t l = 0; l < layers.size(); l++) {
      for(uint32_t i = 0; i < merged_result.size(); i++) {
        if (merged_result[i] == 2) {
          merged_result[i] = layers[l][i];
        }
      }
    }

    for(uint32_t i = 0; i < merged_result.size(); i++) {
      if (merged_result[i] == 1) {
        FillRect(500 + i % width, 100 + i / width, 1, 1);
      }
    }

    day_complete = true;
    return "Layer with fewest 0s: " + to_string(layer_with_fewest_zeros) + "\n" +
        "Number of 1s: " + to_string(layer_digit_counts[layer_with_fewest_zeros][1]) + "; " +
        "Number of 2s: " + to_string(layer_digit_counts[layer_with_fewest_zeros][2]) + "\n" +
        "Product: " + to_string(layer_digit_counts[layer_with_fewest_zeros][1] * layer_digit_counts[layer_with_fewest_zeros][2]) + "\n";
  }

  vector<memtype> parsed_program;

  void parse_program() {
    parsed_program.clear();

    for (auto &line : input_lines) {
      vector<string> line_opcodes = regex_split(line, ",");

      for (auto &opcode : line_opcodes) {
        parsed_program.push_back(stol(opcode));
      }
    }

    parsed_program.resize(parsed_program.size() * 100);
  }

  string day9() {
    IntcodeProgram p;
    parse_program();
    p.program = parsed_program;
    intcode_processor(&p, 1);
    memtype result_1 = p.result.back();
    p.result.pop_back();
    /*

    cout << "\033[2J";
    cout << "\033[H\033[7m         ";
    for(uint32_t i = 0; i < 8; i++) {
      printf("%02x        ", i);
    }
    cout << "\033[0m";
    for(uint32_t i = 0; i < p.program.size(); i++) {
      if (i % 8 == 0) {
        cout << "\n";
        printf("\033[7m%02x\033[0m ", i);
      }

      if (i == p.pc) {
        printf("\033[5m%8lld\033[0m  ", p.program[i]);
      }
      else {
        printf("%8lld  ", p.program[i]);
      }
    }
    printf("\nPC is at %d\n", p.pc);
    */
    day_complete = true;
    IntcodeProgram p2;
    p2.program = parsed_program;
    intcode_processor(&p2, 2);
    memtype result_2 = p2.result.back();
    p2.result.pop_back();

    return "Part1: " + to_string(result_1) + "\nPart2: " + to_string(result_2);
  }

  typedef vector<vector<bool>> asteroid_field;

  map<double, vector<Asteroid>> find_visible(asteroid_field field, int y, int x) {
    // Only one asteroid can occupy any particular angle so the number
    // of unique angles we can trace to other asteroids is the number that are
    // visible
    map<double, vector<Asteroid>> angles;
    printf("Counting at (%d, %d)... ", x, y);
    for(int i = 0; i < field.size(); i++) {
      for(int j = 0; j < field[i].size(); j++) {
        if((i == y && j == x) || !field[i][j]) continue;
        double angle;

        double dx = j - x;
        double dy = i - y;

        Asteroid a;
        a.point.x = j;
        a.point.y = i;
        a.distance = dx * dx + dy * dy;
        angle = atan2(dy, dx) * 180 / acos(-1) + 180;

        if (angles.find(angle) == angles.end()) {
          angles[angle] = { a };
        } else {
          angles[angle].push_back(a);
        }
      }
    }

    printf("%ld asteroids\n", angles.size());
    return angles;
  }

  string day10() {
    asteroid_field field;

    for (int i = 0; i < input_lines.size(); i++) {
      vector<bool> line;
      for (int c = 0; c < input_lines[i].length(); c++) {
        if (input_lines[i][c] != '\n') {
          printf("%c", input_lines[i][c]);
          line.push_back(input_lines[i][c] == '#');
        }
      }
      printf("\n");
      field.push_back(line);
    }

    float star_scale = 0.6;
    uint32_t field_height = field.size(),
             field_width = field[0].size(),
             grid_height = (ScreenHeight() - border * 2) / field_height,
             grid_width = (ScreenWidth() - border * 2) / field_width,
             star_height = grid_height * star_scale,
             star_width = grid_width * star_scale,
             star_offset_y = (grid_height - star_height),
             star_offset_x = (grid_width - star_width);

    for (int i = 0; i < field_height; i++) {
      DrawLine(border, i * grid_height + border, ScreenWidth() - border, i * grid_height + border, olc::GREY);
      for (int j = 0; j < field_width; j++) {
        DrawLine(j * grid_width + border, border, j * grid_width + border, ScreenHeight() - border, olc::GREY);
        if(field[i][j]) {
          FillCircle(
              border + grid_width * j + star_offset_x,
              border + grid_height * i + star_offset_y,
              (star_width + star_height) / 6,
              olc::YELLOW
          );
        }
      }
    }
    DrawLine(ScreenWidth() - border, border, ScreenWidth() - border, ScreenHeight() - border, olc::GREY);
    DrawLine(border, ScreenHeight() - border, ScreenWidth() - border, ScreenHeight() - border, olc::GREY);

    uint32_t max_visible = 0;
    tuple<int, int> station_coordinates;
    map<double, vector<Asteroid>> station_angles;

    for (int i = 0; i < field_height; i++) {
      for (int j = 0; j < field_width; j++) {
        if (field[i][j]) {
          map<double, vector<Asteroid>> angles = find_visible(field, i, j);
          uint32_t num_visible = angles.size();

          if (num_visible > max_visible) {
            printf("Found %d at (%d, %d)\n", num_visible, j, i);
            max_visible = num_visible;
            station_coordinates = { j, i };
            station_angles = angles;
          }
        }
      }
    }

    string part1_result = "Max visible: " + to_string(max_visible);

    // Pull a vector of all the possible angles, sort by distance, count
    // total asteroids
    vector<double> angles;
    uint32_t total_asteroids = 0;
    for (auto it = station_angles.begin(); it != station_angles.end(); ++it) {
      angles.push_back(it->first);
      sort(it->second.begin(), it->second.end());
      total_asteroids += it->second.size();
    }

    sort(angles.begin(), angles.end());

    // Start at the angle nearest to 90
    uint32_t first_angle_idx = 0, i = 0;

    for (auto it = angles.begin(); it != angles.end(); ++it) {
      if(*it >= 90) {
        first_angle_idx = i;
        break;
      }
      i++;
    }

    uint32_t destroyed_asteroids = 0, angle_idx = first_angle_idx;
    Asteroid last_destroyed;
    while(1) {
      double angle = angles[angle_idx++];

      vector<Asteroid> asteroids = station_angles[angle];

      if (asteroids.size() > 0) {
        destroyed_asteroids ++;
        last_destroyed = asteroids[0];

        asteroids.erase(asteroids.begin());
      }

      if (destroyed_asteroids == 200 || destroyed_asteroids == total_asteroids)
        break;

      if (angle_idx > angles.size()) angle_idx = 0;
    }

    string part2_result = "Destroyed " + to_string(destroyed_asteroids) + " asteroids. ";

    part2_result += "Last destroyed: (" + to_string(last_destroyed.point.x) + "," + to_string(last_destroyed.point.y) + ")";

    day_complete = true;
    return "Part1: " + part1_result + "\nPart2: " + part2_result;
  }

  struct Panel {
    bool color = 0;
    uint32_t count = 0;
  };

  string day11_execute(bool color) {
    IntcodeProgram p;
    parse_program();
    p.program = parsed_program;
    Panel hull[200][200];

    uint32_t x = 99, y = 99;
    double pi2 = acos(-1) / 2;
    double direction = pi2;
    uint32_t unique_paints = 0;

    hull[x][y].color = color;

    printf("Starting with (%d, %d) = %d\n", x, y, color);
    while(p.state != HALTED) {
      intcode_processor(&p, hull[x][y].color);

      memtype turn_direction = p.result.back();
      p.result.pop_back();
      memtype color = p.result.back();
      p.result.pop_back();

      if (turn_direction == 0)
        direction += pi2;
      else
        direction -= pi2;

      if (direction > pi2 * 4) {
        direction -= pi2 * 4;
      } else if(direction < -pi2 * 4) {
        direction += pi2 * 4;
      }

//      printf("Turn: %lld, color: %lld, direction: %f, pos=(%d, %d)\n", turn_direction, color, direction, x, y);
      hull[x][y].color = color;
      if (hull[x][y].count == 0) {
        unique_paints++;
      }
      hull[x][y].count++;

      x += cos(direction);
      y += sin(direction);
    }

    if (color == 1) {
      for(uint32_t x = 0; x < 200; x++)
        for(uint32_t y = 0; y < 200; y++)
          if (hull[x][y].color)
            FillRect(x, 200-y, 1, 1, olc::BLUE);
    }

    return to_string(unique_paints);
  }

  string day11() {
    string unique_paints = day11_execute(0);
    day11_execute(1);
    day_complete = true;
    return "Part1: painted " + unique_paints + " segments";
  }

  struct Coord {
    int64_t x = 0, y = 0, z = 0;
  };
  struct Moon {
    Coord position;
    Coord lcm;
    Coord initial;
    Coord velocity;

    uint64_t potential_energy() {
      return abs(position.x) + abs(position.y) + abs(position.z);
    }
    uint64_t kinetic_energy() {
      return abs(velocity.x) + abs(velocity.y) + abs(velocity.z);
    }
    uint64_t total_energy() {
      return potential_energy() * kinetic_energy();
    }

    uint64_t steps_to_repeat() {
      return std::lcm(std::lcm(lcm.x, lcm.z), lcm.y);
    }
  };


  string day12() {
    vector<Moon> moons;

    for (auto &line : input_lines) {
      Moon moon;

      sscanf(line.c_str(), "<x=%lld, y=%lld, z=%lld>", &moon.position.x, &moon.position.y, &moon.position.z);
      moon.initial = moon.position;
      moons.push_back(moon);
    }

    for (int i = 0; i < moons.size(); i++) {
      Moon moon = moons[i];
      printf("Moon %d: <%lld, %lld, %lld>  <%lld, %lld, %lld>\n",
          i,
          moon.position.x,
          moon.position.y,
          moon.position.z,
          moon.velocity.x,
          moon.velocity.y,
          moon.velocity.z
        );
    }

    int64_t total_energy = 0;
    uint64_t step = 0;
    uint64_t steps_to_repeat = 0;
    Coord axis_lcm;

    while(1) {
      // Apply gravity to all moons pairwise
      for(uint8_t i = 0; i < moons.size(); i++) {
        for(uint8_t j = 0; j < moons.size(); j++) {
          if(i == j) continue;

          Coord p1 = moons[i].position, p2 = moons[j].position;

          if(p1.x < p2.x) moons[i].velocity.x += 1;
          else if(p1.x > p2.x) moons[i].velocity.x -= 1;

          if(p1.y < p2.y) moons[i].velocity.y += 1;
          else if(p1.y > p2.y) moons[i].velocity.y -= 1;

          if(p1.z < p2.z) moons[i].velocity.z += 1;
          else if(p1.z > p2.z) moons[i].velocity.z -= 1;
        }

      }

      // Apply new velocity
      for(uint8_t i = 0; i < moons.size(); i++) {
        moons[i].position.x += moons[i].velocity.x;
        moons[i].position.y += moons[i].velocity.y;
        moons[i].position.z += moons[i].velocity.z;
      }

      step++;

      uint8_t matching[3] = {0, 0, 0};

      for(uint8_t i = 0; i < moons.size(); i++) {
        if(moons[i].position.x == moons[i].initial.x && moons[i].velocity.x == 0) {
          matching[0]++;
        }
        if(moons[i].position.y == moons[i].initial.y && moons[i].velocity.y == 0) {
          matching[1]++;
        }
        if(moons[i].position.z == moons[i].initial.z && moons[i].velocity.z == 0) {
          matching[2]++;
        }
      }

      if(matching[0] == moons.size() && axis_lcm.x == 0)
        axis_lcm.x = step;
      if(matching[1] == moons.size() && axis_lcm.y == 0)
        axis_lcm.y = step;
      if(matching[2] == moons.size() && axis_lcm.z == 0)
        axis_lcm.z = step;

      if(step == 10 || step == 100 || step == 1000 || (step % 1000000) == 0) {
        printf("At %lld steps...\n", step);

        total_energy = 0;
        for(uint8_t i = 0; i < moons.size(); i++) {
          printf("[%d] <%lld, %lld, %lld>   <%lld, %lld, %lld>  pot=%lld kin=%lld tot=%lld\n",
              i,
              moons[i].position.x, moons[i].position.y, moons[i].position.z,
              moons[i].velocity.x, moons[i].velocity.y, moons[i].velocity.z,
              moons[i].potential_energy(),
              moons[i].kinetic_energy(),
              moons[i].total_energy()
          );
          total_energy += moons[i].total_energy();
        }
        printf("%lld,%lld\n", step, total_energy);
      }

      steps_to_repeat = 1;

      if(axis_lcm.x > 0 && axis_lcm.y > 0 && axis_lcm.z > 0) {
        steps_to_repeat = lcm(lcm(axis_lcm.x, axis_lcm.y), axis_lcm.z);
        printf("[%lld] lcm(%lld, %lld, %lld) = %lld\n", step,
            axis_lcm.x, axis_lcm.y, axis_lcm.z, steps_to_repeat);
      }

      if(steps_to_repeat > 1)
        break;
    }

    day_complete = true;
    return
      "Part1: Total energy after 1000 steps: " + to_string(total_energy) + "\n" +
      "Part2: Steps needed to return: " + to_string(steps_to_repeat);
  }

#define FIELD_SIZE 50
#define TILE_SIZE 5
  enum Tile { BLANK, WALL, BLOCK, PADDLE, BALL };

  typedef Tile Field[FIELD_SIZE][FIELD_SIZE];

  void draw_field(Field field) {
    for(int i = 0; i < FIELD_SIZE; i++) {
      for(int j = 0; j < FIELD_SIZE; j++) {
        olc::Pixel color = olc::BLACK;
        if(field[i][j] == WALL) color = olc::BLUE;
        if(field[i][j] == BLOCK) color = olc::RED;
        if(field[i][j] == PADDLE) color = olc::GREEN;
        if(field[i][j] == BALL) color = olc::CYAN;
        FillRect(100 + i * TILE_SIZE, 100 + j * TILE_SIZE, TILE_SIZE, TILE_SIZE, color);
      }
    }
  }

  void count_tiles(Field field, uint32_t *tiles) {
    for(int i = 0; i < 5; i++) tiles[i] = 0;

    for(int i = 0; i < FIELD_SIZE; i++)
      for(int j = 0; j < FIELD_SIZE; j++)
        tiles[field[i][j]]++;
  }

  void process_intcode_results(IntcodeProgram *p, Field field, uint32_t *score) {
    for(int i = p->result.size() - 1; i > 0; i-=3) {
      Tile blocktype = (Tile) p->result[i];
      int y = p->result[i-1], x = p->result[i-2];

      if(x == -1 && y == 0) {
        *score = blocktype;
      } else {
        field[x][y] = blocktype;
      }
    }
  }

  // How many iterations per frame we'll do
  int day13_speed = 10;
  Field day13_field;

  IntcodeProgram day13_program;
  uint32_t day13_starting_blocks = 0;
  uint32_t day13_blocks = 0;
  uint32_t day13_score = 0;

  string day13() {
    parse_program();
    vector<memtype> program_template = parsed_program;

    uint32_t num_tiles[5];

    // First time around, just figure out the field
    if (day13_blocks == 0) {
      day13_program.program = program_template;
      intcode_processor(&day13_program, 0);

      for(int i = 0; i < FIELD_SIZE; i++)
        for(int j = 0; j < FIELD_SIZE; j++)
          day13_field[i][j] = BLANK;

      uint32_t score;
      process_intcode_results(&day13_program, day13_field, &score);
      draw_field(day13_field);
      count_tiles(day13_field, num_tiles);
      day13_starting_blocks = day13_blocks = num_tiles[BLOCK];

      day13_program.program = program_template;
      day13_program.program[0] = 2;
      printf("Found %d starting blocks; inserting a quarter...\n", day13_blocks);
      return to_string(day13_blocks) + " starting blocks";
    }

    uint32_t num_iterations = 0;

    while(day13_blocks > 0 && num_iterations < day13_speed) {
      int ball_x = 0, paddle_x = 0;
      for(int i = 0; i < FIELD_SIZE; i++) {
        for(int j = 0; j < FIELD_SIZE; j++) {
          if (day13_field[i][j] == BALL)
            ball_x = i;
          else if (day13_field[i][j] == PADDLE)
            paddle_x = i;
        }
      }

      // Tilt the paddle towards the ball
      int input = 0;
      if (ball_x < paddle_x) input = -1;
      if (ball_x > paddle_x) input = 1;

      day13_program.result.clear();

      intcode_processor(&day13_program, input);

      uint32_t score = 0;
      process_intcode_results(&day13_program, day13_field, &score);

      if (score > day13_score) day13_score = score;

      count_tiles(day13_field, num_tiles);
      day13_blocks = num_tiles[BLOCK];

      num_iterations++;
    }

    if (is_visual) {
      FillRect(100, 80, 200, 20, olc::BLACK);
      DrawStringDecal(olc::vf2d(100, 80), "Score: " + to_string(day13_score));
      draw_field(day13_field);
    }

    if (day13_blocks == 0) {
      day_complete = true;
    }

    return to_string(day13_starting_blocks) + " starting blocks; Final score: " + to_string(day13_score);
  }

  struct Term {
    string name;
    uint32_t quantity;

    Term() {
      name = "";
      quantity = 0;
    }

    Term(string v) {
      char _name[250];
      sscanf(v.c_str(), "%d %s", &quantity, _name);

      name = string(_name);
    }
  };
  struct Formula {
    vector<Term> inputs;
    uint8_t level = 0;
    Term output;
  };

  map<string, Formula> formulae;
  map<string, uint64_t> requirements;
  map<string, uint64_t> surplus;

  /*
  function requiredOreToMakeProduct(product, qty) {
    var recipe = recipes.filter(r => r.result.name == product)[0];
    var existing = surplus.has(product) ? surplus.get(product) : 0;
    var multiple = Math.ceil(Math.max((qty - existing),0) / recipe.result.qty);
    var extra = (recipe.result.qty * multiple) - (qty - existing);
    if (product != "ORE") { surplus.set(product,extra); }
    var ore = 0;
    for (const ingr of recipe.ingredients) {
        if (ingr.name == "ORE") {
            ore += multiple * ingr.qty;
        } else {
            ore += requiredOreToMakeProduct(ingr.name, multiple * ingr.qty);
        }
    }
    return ore;
}
*/

  uint64_t find_requirements(string name, uint64_t qty = 1) {
    Formula f = formulae[name];
    uint64_t extra = surplus[name];

//    printf("finding requirements for %llu of %s (%lld extra)\n", qty, name.c_str(), extra);
    uint64_t multiplier = (qty + f.output.quantity - extra - 1) / f.output.quantity;

    if (name != "ORE") {
      surplus[name] = f.output.quantity * multiplier - (qty - extra);
    }

    uint64_t total_ore = 0;

    for(auto &i: f.inputs) {
      if (i.name == "ORE") {
        total_ore += multiplier * i.quantity;
      } else {
        total_ore += find_requirements(i.name, multiplier * i.quantity);
      }
    }

    return total_ore;
  }

  string day14() {
    formulae.clear();
    requirements.clear();

    for (auto &line: input_lines) {
      vector<string> io = regex_split(line, " => ");
      vector<string> inputs = regex_split(io[0], ",");
      Formula f;

      for(auto &input: inputs) {
        f.inputs.push_back(Term(input));
      }

      f.output = Term(io[1]);
      formulae[f.output.name] = f;
    }

    uint64_t ore_for_one = find_requirements("FUEL");

    printf("Total ore required for 1 FUEL: %llu\n", ore_for_one);

    uint64_t starting_ore = 1000000000000;
    uint64_t max_fuel = starting_ore / ore_for_one;

    printf("Maximum fuel we could make: %lld\n", max_fuel);

    while(1) {
      surplus.clear();
      uint64_t required_ore = find_requirements("FUEL", max_fuel);
      int64_t diff = starting_ore - required_ore;

      printf("Ore required for %llu fuel: %llu (ore remaining: %lld)\n", max_fuel, required_ore, diff);

      if (diff > 0) {
        uint64_t error_scale, error_adjust;
        diff /= ore_for_one;

        if (diff >= 10) {
          error_scale = (int) log10(diff);
          error_adjust = pow(10, error_scale);
        } else {
          error_scale = 0;
          error_adjust = 1;
        }

        max_fuel += error_adjust;
        printf("Diff=%lld, Adjusting by 10^%llu = %llu: %llu\n", diff, error_scale, error_adjust, max_fuel);
      } else {
        max_fuel --;
        break;
      }
    }
    day_complete = true;

    return "Total required: " + to_string(ore_for_one) + ";  Max fuel: " + to_string(max_fuel);
  }

  enum Direction { NONE, NORTH, SOUTH, WEST, EAST };
  map<Direction, string> dir_strings = {{NORTH, "NORTH"}, {SOUTH, "SOUTH"}, {WEST, "WEST"}, {EAST, "EAST"}};

  map <Direction, Direction> right_turns = {{NORTH, EAST}, {EAST, SOUTH}, {SOUTH, WEST}, {WEST, NORTH}};
  map <Direction, Direction> left_turns = {{NORTH, WEST}, {WEST, SOUTH}, {SOUTH, EAST}, {EAST, NORTH}};

#define DROID_MAP_SIZE 50
  uint8_t day15_map[DROID_MAP_SIZE][DROID_MAP_SIZE];

  struct Droid {
    uint8_t map[DROID_MAP_SIZE][DROID_MAP_SIZE];
    uint16_t x = DROID_MAP_SIZE / 2;
    uint16_t y = DROID_MAP_SIZE / 2;
    uint16_t id = 0;

    IntcodeProgram program;

    Droid() {
      printf("Initializing droid with empty map...");
      for(int _x = 0; _x < DROID_MAP_SIZE; _x++)
        for(int _y = 0; _y < DROID_MAP_SIZE; _y++)
          map[_x][_y] = 0;

      set(0, 0, 1);
    }

    Droid(IntcodeProgram p) : Droid() {
      program = p;
    }

    uint8_t get(int16_t dx, int16_t dy) {
      return map[x + dx][y + dy];
    }

    void set(int16_t dx, int16_t dy, uint8_t val) {
      map[x + dx][y + dy] = val;
    }

    uint8_t get() {
      return map[x][y];
    }
  };

  Droid winning_droid;

  float day15_speed = 150;
  int day15_steps = 0;
  int found_steps = 0;
  vector<Droid> day15_droids;

  int draw_day15_map() {
    int unfilled_spots = 0;

    for(uint16_t _x = 0; _x < DROID_MAP_SIZE; _x++) {
      for(uint16_t _y = 0; _y < DROID_MAP_SIZE; _y++) {
        olc::Pixel color = olc::BLACK;

        if (day15_map[_x][_y] == 1) color = olc::RED;
        if (day15_map[_x][_y] == 2) {
          color = olc::GREEN;
          unfilled_spots++;
        }
        if (day15_map[_x][_y] == 3) color = olc::YELLOW;
        if (day15_map[_x][_y] == 4) color = olc::WHITE;
        if (day15_map[_x][_y] > 4) color = olc::MAGENTA;

        if(_x == DROID_MAP_SIZE / 2 && _y == DROID_MAP_SIZE / 2)
          color = olc::CYAN;

//        printf("Drawing map <%d, %d> for %d\n", _x, _y, day15_map[_x][_y]);
        FillRect(_x * map_scale, _y * map_scale, map_scale, map_scale, color);
      }
    }

    return unfilled_spots;
  }

  bool find_oxygen_system() {
    int stepped_droids = 0;

    if (day_time * day15_speed < day15_steps) {
      return false;
    }

    printf("\n[%d] Processing %lu\n", day15_steps, day15_droids.size());

    vector<Droid> kept_droids;

/*
    for(int pair = 0; pair < day15_droids.size() - 1; pair++) {
      Droid *d1 = &day15_droids[pair], *d2 = &day15_droids[pair+1];

      for(int i = 0; i < d1->program.program.size(); i++)
        if(d1->program.program[i] != d2->program.program[i])
          printf("Program differences: [%d/%d] [%d] %lld != %lld\n", pair, pair+1, i, d1->program.program[i], d2->program.program[i]);
    }
    */
    for (auto &droid: day15_droids) {
      int steps = 0;
      int forks = 0;

      // Old position is now just seen
      if(day15_map[droid.x][droid.y] != 4)
        day15_map[droid.x][droid.y] = 2;

      Droid og_droid = droid;
      IntcodeProgram p = droid.program;

      printf("[%d] is now at <%d, %d>\n", droid.id, droid.x, droid.y);
      Direction dir = NORTH;

      do {
        int step_x = dir == EAST ? 1 : dir == WEST ? -1 : 0;
        int step_y = dir == NORTH ? -1 : dir == SOUTH ? 1 : 0;

        printf("[%d/%d] %s, num_steps=%d At %d,%d: %d\n", droid.id, steps, dir_strings[dir].c_str(), day15_steps, og_droid.x+step_x, og_droid.y+step_y, og_droid.get(step_x, step_y));

        if (og_droid.get(step_x, step_y) == 0) {
          IntcodeProgram fork = p;
          intcode_processor(&fork, dir);

          memtype result = fork.result.back();

          printf("[%d/%d] trying %s (<%d, %d>) to <%d, %d>: %lld\n", og_droid.id, steps, dir_strings[dir].c_str(), step_x, step_y, og_droid.x + step_x, og_droid.y + step_y, result);

          if (result == 0) {
            // We hit a wall, this isn't the right way
            og_droid.set(step_x, step_y, 1);
            droid = og_droid;
            //droid.set(step_x, step_y, 1);
            day15_map[og_droid.x + step_x][og_droid.y + step_y] = 1;
          }
          else if (result == 1 || result == 2) {
            Droid *next_droid = &droid;
            Droid new_droid = og_droid;

            if (forks > 0) {
              // The original droid ref already stepped away but we can also
              // go in another direction; fork it off
              new_droid.id = day15_droids.size() + forks - 1;
              printf("[%d] NEW at <%d, %d>\n", new_droid.id, new_droid.x, new_droid.y);
              next_droid = &new_droid;
            }
            next_droid->program = fork;

            next_droid->x += step_x;
            next_droid->y += step_y;

            next_droid->set(0, 0, 1);

            day15_map[next_droid->x][next_droid->y] = 3;

            kept_droids.push_back(*next_droid);

            forks++;

            if (result == 2) {
              printf("FOUND! %d,%d\n", droid.x, droid.y);
              winning_droid = droid;
              found_steps = day15_steps + 1;
              day15_map[next_droid->x][next_droid->y] = 4;
            }
          } else {
            printf("Unexpected result from intcode program: %lld\n", result);
            exit(1);
          }

          steps++;
        }

        dir = right_turns[dir];
      } while (dir != NORTH);

      if (steps > 0) {
        printf("[%d] took a step\n", og_droid.id);
        stepped_droids++;
      } else {
        // printf("[%d] hit a dead-end!\n", droid.id);
      }
    }

    if(stepped_droids > 0)
      day15_steps++;

    // printf("%d droids took a step; adding %lu droids\n\n", stepped_droids, new_droids.size());

    day15_droids.clear();
    for (auto &kept_droid: kept_droids)
        day15_droids.push_back(kept_droid);

    return stepped_droids == 0;
  }

  void day15_init() {
    parse_program();
    vector<memtype> program_template = parsed_program;

    IntcodeProgram droid_program;
    droid_program.program = program_template;

    day15_steps = 0;
    day15_droids.clear();

    Droid droid0 = Droid(droid_program);
    day15_droids.push_back(droid0);

    for(int i = 0; i < DROID_MAP_SIZE; i++) {
      for(int j = 0; j < DROID_MAP_SIZE; j++) {
        day15_map[i][j] = 0;
      }
    }

    day15_map[droid0.x][droid0.y] = 3;
  }

  int day15_flood_time = 0;

  void fill_map(int x, int y, int steps = 0) {
    if (day_time * day15_speed < (day15_steps + steps)) {
      return;
    }

    Direction dir = NORTH;
    day15_map[x][y] += 5;

    int forks = 0;

    do {
      int step_x = dir == EAST ? 1 : dir == WEST ? -1 : 0;
      int step_y = dir == NORTH ? -1 : dir == SOUTH ? 1 : 0;

      if(day15_map[x+step_x][y+step_y] == 2) {
        forks++;
        fill_map(x+step_x, y+step_y, steps+1);
      }

      dir = right_turns[dir];
    } while (dir != NORTH);

    if (forks == 0 && steps > day15_flood_time)
      day15_flood_time = steps;
  }

  string day15() {
    bool oxygen_found = find_oxygen_system();

    if (oxygen_found) {
      fill_map(winning_droid.x, winning_droid.y);
    }

    int unfilled_spots = draw_day15_map();

    if (oxygen_found && unfilled_spots == 0)
      day_complete = true;

    return (
      "Minimum steps to find system: " + to_string(found_steps) + "\n" +
      "Flood time: " + to_string(day15_flood_time)
    );
  }

  // Flawed frequency transmission... naive but general
  void day16_fft(vector<uint8_t> *in_digits, vector<uint8_t> *out_digits, uint32_t phase) {
    int num_digits = in_digits->size();
    vector<int8_t> base_pattern = { 0, 1, 0, -1 };

    for(int i = 0; i < num_digits; i++) {
      int new_digit = 0;
      int index = 0;

      for (int j = 0; j < num_digits; j++) {
        if(i == 0 || (j + 1) % (i + 1) == 0) index ++;

        uint8_t midx = ((int) index) % 4;

        // Skip 0s
        if (midx % 2 == 0) continue;

        uint8_t in_digit = (*in_digits)[j];

        if (midx == 1)
          new_digit += in_digit;
        else
          new_digit -= in_digit;

      }

      //printf("out[%d] = %d\n", i, abs(new_digit % 10));
      out_digits->push_back(abs(new_digit % 10));
    }
  }

  // This only works for determining digits in the last 1/4 of
  // the input, i.e., offset > input_size * .75, as it's a simple sum
  // of digits
  void day16_faster_fft(vector<uint8_t> *in_digits, vector<uint8_t> *out_digits) {
    int num_digits = in_digits->size();
    int sum = 0;

    // Get the sum ONCE, then each digit will be that sum minus previous digits
    for(int i = 0; i < num_digits; i++) {
      sum += (*in_digits)[i];
    }

    for (int j = 0; j < num_digits; j++) {
      out_digits->push_back(sum % 10);
      sum -= (*in_digits)[j];
    }
  }

  string day16() {
    vector<uint8_t> digits;
    uint32_t offset = 0;

    for(int i = 0; i < input_lines[0].size(); i++) {
      uint8_t digit = input_lines[0][i] - '0';
      if (i < 7) {
        offset *= 10;
        offset += digit;
      }
      digits.push_back(input_lines[0][i] - '0');
    }

    printf("Offset: %d  Input: ", offset);
    for (uint8_t digit: digits) {
      printf("%d", digit);
    }

    printf("\n");

    vector<uint8_t> last = digits;
    vector<uint8_t> next;

    string part1_result = "";
    for (int i = 0; i < day16_phases; i++) {
      day16_fft(&last, &next, i);
      printf("[After #%d] ", i + 1);
      for (uint8_t digit: next) {
        printf("%d", digit);
      }
      printf("\n");
      last = next;
      next.clear();
    }

    for (int i = 0; i < 8; i++)
      part1_result += (last[i] + '0');

    day_complete = 1;

    printf("Part 2 offset: %d\n", offset);

    last.clear();
    next.clear();

    for(int i = offset; i < digits.size() * 10000; i++)
      last.push_back(digits[i % digits.size()]);

    for (int i = 0; i < day16_phases; i++) {
      string next_step;
      for (int j = 0; j < 8; j++)
        next_step += last[j] + '0';

      printf("[%d] Running faster FFT on %lu elements: %s\n", i, last.size(), next_step.c_str());
      day16_faster_fft(&last, &next);

      last = next;
      next.clear();
    }

    string part2_result;

    for (int i = 0; i < 8; i++)
      part2_result += last[i] + '0';

    printf("After %d phases, part2_result[%d-%d]: %s\n", day16_phases, offset, offset + 8, part2_result.c_str());

    return "Part1: " + part1_result + "\nPart2: " + part2_result;
  }

  /**
   * Day 17!
   */
  struct PossiblePath {
    Direction dir;
    Point pos;
    vector<Point> points = {};
    vector<string> steps = {};
    int num_steps = 0;
  };

  char **day17_map = NULL;
  int day17_total_points = 0;
  Point day17_dims, day17_start;
  vector<memtype> day17_program;
  IntcodeProgram day17_intcode;
  vector<PossiblePath> day17_paths;
  vector<PossiblePath> day17_fullpaths;
  vector<PossiblePath> day17_kept_paths;
  bool day17_robot_running = false;
  uint32_t day17_alignment_parameter_sum = 0;
  string day17_compressed_command[4];
  float day17_speed = 200;
  string day17_message = "";

  int day17_route_step() {
    int total_steps = 0;
    int expected_points = day17_paths[0].points.size() + 1;

    for(auto path: day17_paths) {
      uint8_t num_steps = 0; // How many steps this path did take

      vector<Direction> directions = { path.dir, left_turns[path.dir], right_turns[path.dir] };

      for (auto &td: directions) {
        int step_x = td == EAST ? 1 : td == WEST ? -1 : 0;
        int step_y = td == NORTH ? -1 : td == SOUTH ? 1 : 0;

        int next_x = path.pos.x + step_x;
        int next_y = path.pos.y + step_y;

        if(next_x < 0 || next_x >= day17_dims.x || next_y < 0 || next_y >= day17_dims.y)
          continue;

        if (day17_map[next_x][next_y] == '#') { // There's another point
          PossiblePath next_path = path;
          Point next_point(next_x, next_y);

          next_path.points.push_back(next_point);
          next_path.pos = next_point;

          bool visited_point = std::find(path.points.begin(), path.points.end(), next_point) != path.points.end();

          if (td == path.dir) { // In the same direction
            next_path.num_steps++;
            num_steps++;
            day17_kept_paths.push_back(next_path);
          }
          else { // We took a turn
            if (path.num_steps > 0) // The first turn might have no steps, don't save it
              next_path.steps.push_back(to_string(path.num_steps));

            next_path.dir = td;
            next_path.num_steps = 1;
            next_path.steps.push_back(td == left_turns[path.dir] ? "L" : "R");

            num_steps++;

            day17_kept_paths.push_back(next_path);
          }
          break;
        }
      }

      if (num_steps == 0) {
        // We've reached the end of the line and can step no more; this is a potential path
        if (path.points.size() >= day17_total_points) {
          path.steps.push_back(to_string(path.num_steps));
          printf("Adding path with %d steps to full paths list\n", path.steps.size());
          day17_fullpaths.push_back(path);
          return 0;
        } else {
          printf("Rejecting path with only %lu of %d necessary points\n", path.points.size(), day17_total_points);
        }
      } else {
        total_steps ++;
      }
    }

    day17_paths.clear();
    day17_paths = day17_kept_paths;
    day17_kept_paths.clear();

    return total_steps;
  }

  void day17_seek_routes() {
    PossiblePath path;
    Point start;

    for (int x = 0; x < day17_dims.x - 1; x++) {
      for (int y = 0; y < day17_dims.y - 1; y++) {
        if (day17_map[x][y] == '^' || day17_map[x][y] == 'v' || day17_map[x][y] == '<' || day17_map[x][y] == '>') {
          start.x = x;
          start.y = y;
        }
      }
    }

    path.pos = start;

    switch(day17_map[start.x][start.y]) {
      case '^':
        path.dir = NORTH;
        break;
      case '>':
        path.dir = EAST;
        break;
      case '<':
        path.dir = WEST;
        break;
      case 'v':
        path.dir = SOUTH;
        break;
    }

    path.points.push_back(start);
    day17_paths.push_back(path);
  }

  void day17_compress_steps(vector<string> steps) {
    string full = "", main = "", a = "", b = "", c = "";

    for(auto &step: steps)
      full += step + ",";

    full.resize(full.length() - 1);
    main = full;

    // Search for substrings going downwards from 20 characters
    // to the smallest substring that starts on a turn and doesn't en d on a turn
    for(int j = 20; j > 1; j--) {
      for(int i = 0; i + j < full.size(); i++) {
        string s = full.substr(i, j);

        // Drop leading numbers or commas: always start on a turn
        while (s[0] == ',' || (s[0] - '0' < 10)) s.erase(0, 1);

        // Drop trailing commas or turns: always end on a step length
        while(s.back() == ',' || s.back() == 'L' || s.back() == 'R')
         s.resize(s.length() - 1);

        if (s == "") {
          // We've exhausted our options, and are done
          day17_compressed_command[0] = main;
          day17_compressed_command[1] = a;
          day17_compressed_command[2] = b;
          day17_compressed_command[3] = c;

          return;
        }
        // Search for patterns starting with half of the length which is the longest duplicatable string;
        // Take a step down and try with one less symbol each time;
        int first = main.find(s);

        if (first != string::npos && main.find(s, first + 1) != string::npos) {
          // found a duplicate! put it in a register and replace all occurrences with that string
          // in the main
          int idx;

          while((idx = main.find(s, 0)) != string::npos) {

            if (a == "" || a == s) {
              a = s;
              main.replace(idx, a.size(), "A");
            }
            else if (b == "" || b == s) {
              b = s;
              main.replace(idx, b.size(), "B");
            }
            else if (c == "" || c == s) {
              c = s;
              main.replace(idx, c.size(), "C");
            } 
            else {
              // We ran out of replacement strings. Bad news.
              error_state = "Failed to find sufficient replacement strings!";
              return;
            }
          }
          i = 0;
        }
      }
    }

  }

  void day17_init() {
    parse_program();
    day17_program = parsed_program;

    IntcodeProgram p;
    p.program = day17_program;

    intcode_processor(&p, 0);

    int width = 0, height = 0, output_column = 0;

    for(auto &o: p.result) {
      if (output_column == 0) {
        printf("%02d. ", height);
      }
      putc(o, stdout);
      if(o != '\n' && height == 0)
        width++;

      if (o == '\n' && output_column > 0) {
        output_column = 0;
        height++;
      } else {
        output_column++;
      }
    }

    day17_map = new char*[width];

    int x = 0, y = 0;

    for (x = 0; x < width; x++)
      day17_map[x] = new char[height];

    x = 0;

    for(auto &o: p.result) {
      if(o == '\n') {
        y++;
        x = 0;
      }
      else day17_map[x++][y] = o;
    }

    day17_dims.x = width;
    day17_dims.y = height;
  }

  int day17_scale = 7;
  Point day17_offset = {50, 50};

  void day17_draw_map() {
    for (int x = 0; x < day17_dims.x; x++) {
      for (int y = 0; y < day17_dims.y; y++) {
        olc::Pixel color(64, 64, 64, 255);
        switch (day17_map[x][y]) {
          case '#':
            color = olc::Pixel(128, 128, 255, 255);
            break;
          case '^':
          case '>':
          case '<':
          case 'v':
            color = olc::RED;
            break;
        }

        FillRect(
          olc::vi2d(x * day17_scale + day17_offset.x, y * day17_scale + day17_offset.y), 
          olc::vi2d(day17_scale, day17_scale), 
          color
        ); 
      }
    }
    if(day17_message.size() > 0) {
      DrawStringDecal(olc::vi2d(offset.x, offset.y), day17_message);
    }
  }

  void day17_munge_map_results() {
    int dims = (day17_dims.x + 1) * day17_dims.y + 1;

    if (day17_intcode.result[0] == '\n')
      day17_intcode.result.erase(day17_intcode.result.begin());

    for(int r = 0; r < dims; r++) {
      int x = r % (day17_dims.x + 1);
      int y = r / (day17_dims.x + 1);

      if (day17_intcode.result[r] != '\n')
        day17_map[x][y] = day17_intcode.result[r];
    }

    day17_intcode.result.erase(day17_intcode.result.begin(), day17_intcode.result.begin() + dims);
  }

  void day17_munge_message() {
    string message = "";
    
    for (int i = 0; i < day17_intcode.result.size(); i++) {
      if (day17_intcode.result[i] == 10)
        break;

      message += (char) day17_intcode.result[i];
    }

    if (message.size() > 0) {
      cout << "Bot message: " << message << "\n";
    }

    day17_intcode.result.erase(day17_intcode.result.begin(), day17_intcode.result.begin() + message.size() + 1);

    day17_message = message;
  }

  string day17() {
    if (day17_alignment_parameter_sum == 0) {
      for (int x = 0; x < day17_dims.x - 1; x++) {
        for (int y = 0; y < day17_dims.y - 1; y++) {
          if (x > 0 && x < day17_dims.x - 1 && y > 0 && y < day17_dims.y - 1 && day17_map[x][y] == '#') {
            day17_total_points++;

            if (
                day17_map[x-1][y] == '#' && day17_map[x+1][y] == '#' &&
                day17_map[x][y-1] == '#' && day17_map[x][y+1] == '#'
            )
              day17_alignment_parameter_sum += x * y;
          } 
        }
      }

      printf("Part 1: %d\n", day17_alignment_parameter_sum);
    }
    DrawStringDecal(olc::vf2d(10, 10), "Sum of alignment parameters: " + to_string(day17_alignment_parameter_sum));

    if(day17_paths.size() == 0) {
      day17_seek_routes();
    }

    int steps_taken;
    if (day17_paths[0].points.size() < day_time * day17_speed) {
      steps_taken = day17_route_step();
    } else {
      steps_taken = 1;
    }

    for(int pidx = 0; pidx < day17_paths.size(); pidx++) {
      auto p = day17_paths[pidx];
      int alpha = 255; // max((int) (255 / day17_paths.size()), 1);
      for (auto &point: p.points) {
        FillRect(olc::vi2d(point.x * day17_scale + day17_offset.x, point.y * day17_scale + day17_offset.y), olc::vi2d(day17_scale, day17_scale), olc::Pixel(0, 128, 128, alpha));
      }
      olc::vi2d pos = olc::vi2d(p.pos.x * day17_scale + day17_offset.x, p.pos.y * day17_scale + day17_offset.y);
      FillRect(pos, olc::vi2d(day17_scale, day17_scale), olc::Pixel(0, 255, 128, alpha));
      DrawStringDecal(pos, to_string(pidx));
    }

    if (steps_taken == 0 && day17_robot_running == false) {
      for(int path_idx = 0; path_idx < day17_fullpaths.size(); path_idx++) {
        PossiblePath p = day17_fullpaths[path_idx];

        printf("Path %d: ", path_idx);

        for (auto &s: p.steps) {
          printf("%s,", s.c_str());
        }

        printf("\n");

        day17_compress_steps(p.steps);
        day17_robot_running = true;

        day17_intcode.program = day17_program;
        day17_intcode.program[0] = 2;
        day17_intcode.result.clear();

        for(int i = 0; i < 4; i++) {
          string command = day17_compressed_command[i];
          for (int j = 0; j < command.size(); j++) {
            intcode_processor(&day17_intcode, (memtype) command[j]);
          }
          if (i == 0) day17_munge_map_results();
          day17_munge_message();
          intcode_processor(&day17_intcode, (memtype) '\n');
        }
        day17_munge_message();
        intcode_processor(&day17_intcode, (memtype) is_visual ? 'y' : 'n');
        intcode_processor(&day17_intcode, '\n');
      }
    }

    memtype result = 0;

    if (day17_robot_running) {
      if (is_visual && day17_intcode.result.size() > 1) {
        // We'll have a bunch of output frames: display them one-by-one
        day17_munge_map_results();
        day17_draw_map();
      }

      if (day17_intcode.result.size() == 1) {
        result = day17_intcode.result[0];
        day_complete = true;

        printf("There's a result! %lld", result);
      }
    }

    return (
      "\nPart1: " + to_string(day17_alignment_parameter_sum) + "\n"
      + "potential paths: " + to_string(day17_paths.size()) + "\n"
      + "current length: " + to_string(day17_paths[0].points.size()) + "\n"
      + "found paths: " + to_string(day17_fullpaths.size()) + "\n"
      + "result: " + to_string(result)
    );
  }

  int8_t current_day = -1;
  float day_time = 0;
  bool day_complete = false;
  int8_t selected_day = 0;

  vector<string(Advent2019::*)()> days;

  Advent2019(int8_t _selected_day) : days {
    &Advent2019::day0,
    &Advent2019::day1,  &Advent2019::day2,  &Advent2019::day3,  &Advent2019::day4,  &Advent2019::day5,
    &Advent2019::day6,  &Advent2019::day7,  &Advent2019::day8,  &Advent2019::day9,  &Advent2019::day10,
    &Advent2019::day11, &Advent2019::day12, &Advent2019::day13, &Advent2019::day14, &Advent2019::day15,
    &Advent2019::day16, &Advent2019::day17
  } {
    selected_day = _selected_day;

    sAppName = "Bendoh's Advent 2019";

    printf("Starting up. There are %lu days attempted.\n", days.size());

    if (selected_day > -1) {
      current_day = selected_day;
      printf("Only doing day %d\n", selected_day);
    } else {
      current_day = 0;
      printf("Doing all days!\n");
    }
  }

  void init() {
    cout << "Starting up!\n";

    if (!isatty(fileno(stdin))) {
      cout << "Not connected to a TTY: Parsing lines from stdin\n";
      while (1) {
        char linebuf[1024 * 50];
        cin.getline(linebuf, 1024 * 50);

        if(cin.gcount() == 0)
          break;

        input_lines.push_back(string(linebuf));
      }
    }
  }

  void init_day() {
    if ((selected_day == -1 || input_lines.empty()) && current_day > 0) {
      string input_file = "inputs/2019/" + to_string(current_day);
      cout << "Getting input from " << input_file << "\n";
      FILE *input = fopen(input_file.c_str(), "r");

      if (input == NULL) {
        error_state = "Couldn't open puzzle input " + input_file + "\n";
        return;
      }

      // If lines are longer than 50k we have a problem...
      char input_line[1024 * 50];
      input_lines = {};
      while(fgets(input_line, 10240 * 50, input)) {
        input_lines.push_back(string(input_line));
      }
      fclose(input);
    }

    if(current_day == 15)
      day15_init();

    if(current_day == 17)
      day17_init();
    day_time = 0;
    day_complete = false;
  }

  void run_day() {
    string (Advent2019::*func)() = days[current_day];

    init_day();

    if (!error_state.empty())  {
      cout << "Error: " << error_state;
      exit(1);
    }

    std::chrono::time_point tp_start = std::chrono::system_clock::now();
    std::chrono::time_point tp_last = tp_start;

    std::chrono::duration<float> elapsed_time;

    while(!day_complete) {
      // Day is started, start computing next frame
      string result = (this->*func)();

      std::chrono::time_point tp_after = std::chrono::system_clock::now();
      elapsed_time = tp_after - tp_last;

      //cout << "Computing " << to_string(current_day) << " ...\n";
      day_time += elapsed_time.count();

      cout << "Result: " << result << "\n";

      tp_last = tp_after;
    }
  }

  void run() {
    init();

    day13_speed = 10000;

    if (selected_day == -1) {
      for (current_day = 0; current_day < days.size(); current_day++) {
        run_day();
      }
    } else {
      current_day = selected_day;
      run_day();
    }

  }

	bool OnUserCreate() override {
    init();

    if (input_lines.size() > 0 && selected_day == 0) {
      error_state = "Sorry, you can't specify stdin but not a day number";
    }
    return true;
	}

  bool starting_day = true;

	bool OnUserUpdate(float fElapsedTime) override {
    if (!error_state.empty()) {
      DrawStringDecal(olc::vf2d(5, 5), error_state);
      return true;
    }

    if (current_day > -1) {
      string (Advent2019::*func)() = days[current_day];

      Clear(olc::BLACK);

      if (starting_day) {
        cout << "Day " << to_string(current_day) << " started\n";

        init_day();

        // Day is started, start computing next frame
        starting_day = false;
      } else {
        //cout << "Computing " << to_string(current_day) << " ...\n";
        day_time += fElapsedTime;

        // Day is started, start computing next frame
        string result = (this->*func)();
        //cout << "Computed!\n";

        results[current_day] = result;

        if (day_complete) {
          cout << result << "\n";

          if (selected_day == -1) {
            if (current_day == days.size() - 1) {
              current_day = -1; // We are done.
            } else {
              current_day++; // Move on to the next puzzle
              starting_day = true;
            }
          } else {
            current_day = -1; // We are done.
          }
        }
      }
    }

    for (map<uint8_t, string>::iterator it=results.begin(); it!=results.end(); ++it) {
      uint32_t y_offset = selected_day == -1 ? 20 * it->first : 0;
      DrawStringDecal(olc::vf2d(5, y_offset + border), "Day " + to_string(it->first));
      DrawStringDecal(olc::vf2d(60, y_offset + border), it->second);
    }

		return true;
	}
};


int main(const int argc, const char **argv) {
  int8_t selected_day = argc > 1 ? stoi(argv[1]) : -1;

  parse_environment();

  Advent2019 program(selected_day);

  if (is_visual) {
    program.Construct(600, 400, 2, 2);
    program.Start();
  } else {
    program.run();
  }

  return 0;
}
