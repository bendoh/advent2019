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

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine/olcPixelGameEngine.h"

using namespace std;

vector<std::string> input_lines;
bool is_interactive = false;
bool is_visual = false;

// Border in SCREEN space
uint16_t border = 10;

olc::PixelGameEngine *pge;

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
};


struct Intersection {
  Point point = { 0, 0 };
  uint32_t distance = 0;
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
      if (is_visual || is_interactive) {
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
      if (is_visual || is_interactive) {
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

  enum State { INPUT, OUTPUT, HALTED };
  struct IntcodeProgram {
    vector<memtype> program;
    uint32_t pc = 0;
    State state = INPUT;
    memtype result = -1;
  };

  void day7_intcode_processor(IntcodeProgram *p, memtype input) {
    memtype opcode = 0;
    bool consumed_input = false;

    while(opcode != 99) {
      if (is_interactive) {
        fgetc(terminal);
      }

      opcode = p->program[p->pc] % 100;
      memtype mode = p->program[p->pc] / 100;

      p->pc++;

      bool is_immediate[3] = {
        // Positions 1..N
        (bool) (mode & 1),
        (bool) (mode / 10 & 1),
        (bool) (mode / 100 & 1)
      };

      if(opcode == 1) { // Addition
        uint32_t target1 = is_immediate[0] ? p->pc : p->program[p->pc];
        p->pc++;
        memtype arg1 = p->program[target1];
        uint32_t target2 = is_immediate[1] ? p->pc : p->program[p->pc];
        p->pc++;
        memtype arg2 = p->program[target2];
        uint32_t dest = p->program[p->pc];
        p->program[dest] = arg1 + arg2;
        p->pc++;
        //printf("%lld + %lld = %lld -> program[%d]\n", arg1, arg2, p->program[dest], dest);
      }

      else if(opcode == 2) { // Multiplication
        uint32_t target1 = is_immediate[0] ? p->pc : p->program[p->pc];
        p->pc++;
        memtype arg1 = p->program[target1];
        uint32_t target2 = is_immediate[1] ? p->pc : p->program[p->pc];
        p->pc++;
        memtype arg2 = p->program[target2];
        uint32_t dest = p->program[p->pc];
        p->pc++;
        p->program[dest] = arg1 * arg2;
        //printf("%lld * %lld = %lld -> program[%d]\n", arg1, arg2, p->program[dest], dest);
      }

      else if (opcode == 3) {
        if (!consumed_input) {
          uint32_t position = p->program[p->pc];
          p->pc++;
          p->program[position] = input;
          //printf("p->program[%d] = %d\n", position, input);
          //printf("Now p->program[%d] = %lld\n", position, p->program[position]);
          consumed_input = true;
        } else {
          p->state = INPUT;
          p->pc--; // Restart here
          //printf("Waiting for next input...\n");
          return;
        }
      }

      else if (opcode == 4) {
        uint32_t position = p->program[p->pc];
        p->pc++;
        p->result = p->program[position];
        p->state = OUTPUT;
        //printf("Value at position %d: %d\n", position, p->program[position]);
      }

      else if (opcode == 5 || opcode == 6) {
        uint32_t target1 = is_immediate[0] ? p->pc : p->program[p->pc];
        p->pc++;
        memtype arg1 = p->program[target1];
        uint32_t target2 = is_immediate[1] ? p->pc : p->program[p->pc];
        p->pc++;
        uint32_t dest = p->program[target2];

        //printf("program[%d] = %d %s 0 -> %d\n", target1, arg1, opcode == 5 ? "!=" : "==", dest);
        if((opcode == 5 && arg1 != 0) || (opcode == 6 && arg1 == 0)) {
          printf("Jumping to %d\n",  dest);
          p->pc = dest;
        }
      }

      else if (opcode == 7 || opcode == 8) {
        uint32_t target1 = is_immediate[0] ? p->pc : p->program[p->pc];
        memtype arg1 = p->program[target1];
        p->pc++;
        uint32_t target2 = is_immediate[1] ? p->pc : p->program[p->pc];
        memtype arg2 = p->program[target2];
        p->pc++;
        uint32_t dest = p->program[p->pc];
        p->pc++;
        p->program[dest] = (opcode == 7 && arg1 < arg2) || (opcode == 8 && arg1 == arg2) ? 1 : 0;
        // printf("%lld %s %lld = %lld\n", arg1, opcode == 7 ? "<" : "==", arg2, p->program[dest]);
      } else {
        printf("[ERROR] Invalid opcode %lld!\n", opcode);
      }

      opcode = p->program[p->pc];
    }

    p->state = HALTED;
  }

  string day7() {
    vector<memtype> program_template;

    printf("Parsing lines...\n");
    for (auto &line : input_lines) {
      vector<string> line_opcodes = regex_split(line, ",");

      for (auto &opcode : line_opcodes) {
        program_template.push_back(stoi(opcode));
      }
    }

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
        day7_intcode_processor(&programs[j], perm[j]);
      }

      IntcodeProgram *cp = &programs[0];
      memtype input = 0;
      uint32_t stage = 0;
      uint32_t num_halted = 0;

      while(1) {
        //printf("Running stage %d with input %lld, pc=%d\n", stage, input, cp->pc);
        day7_intcode_processor(cp, input);
        input = cp->result;
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
        FillRect(100 + i % width, 100 + i / width, 1, 1);
      }
    }

    day_complete = true;
    return "Layer with fewest 0s: " + to_string(layer_with_fewest_zeros) + "\n" +
        "Number of 1s: " + to_string(layer_digit_counts[layer_with_fewest_zeros][1]) + "; " +
        "Number of 2s: " + to_string(layer_digit_counts[layer_with_fewest_zeros][2]) + "\n" +
        "Product: " + to_string(layer_digit_counts[layer_with_fewest_zeros][1] * layer_digit_counts[layer_with_fewest_zeros][2]) + "\n";
  }


  vector<string(Advent2019::*)()> days;
  int8_t current_day = -1;

  Advent2019(int8_t _daynum) : days {
    &Advent2019::day0,
    &Advent2019::day1, &Advent2019::day2, &Advent2019::day3, &Advent2019::day4, &Advent2019::day5,
    &Advent2019::day6, &Advent2019::day7, &Advent2019::day8

  } {
    daynum = _daynum;

    sAppName = "Bendoh's Advent 2019";

    printf("Starting up. There are %lu days attempted.\n", days.size());

    if (daynum > -1) {
      current_day = daynum;
      printf("Only doing day %d\n", daynum);
    } else {
      current_day = 0;
      printf("Doing all days!\n");
    }
  }

	bool OnUserCreate() override {
    string line;

    parse_environment();

    cout << "Starting up!\n";

    if (!isatty(fileno(stdin))) {
      cout << "Not connected to a TTY: Parsing lines from stdin\n";
      while (std::cin >> line)
        input_lines.push_back(line);
    }

    if (input_lines.size() > 0 && daynum == 0) {
      error_state = "Sorry, you can't specify stdin but not a day number";
    }

    return true;
	}

  bool starting_day = true;
  float day_time = 0;
  bool day_complete = false;
  int8_t daynum = 0;

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
        if ((daynum == -1 || input_lines.empty()) && current_day > 0) {
          string input_file = "inputs/2019/" + to_string(current_day);
          cout << "Getting input from " << input_file << "\n";
          FILE *input = fopen(input_file.c_str(), "r");

          if (input == NULL) {
            error_state = "Couldn't open puzzle input " + input_file + "\n";
            return true;
          }

          // If lines are longer than 50k we have a problem...
          char input_line[1024 * 50];
          input_lines = {};
          while(fgets(input_line, 10240 * 50, input))
            input_lines.push_back(string(input_line));
          fclose(input);
        }
        day_time = 0;
        // Day is started, start computing next frame
        starting_day = false;
        day_complete = false;
      } else {
        cout << "Computing " << to_string(current_day) << " ...\n";
        day_time += fElapsedTime;

        // Day is started, start computing next frame
        string result = (this->*func)();
        cout << "Computed!\n";

        if (day_complete) {
          results[current_day] = result;
          cout << result << "\n";

          if (daynum == -1) {
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
      uint32_t y_offset = daynum == -1 ? 20 * it->first : 0;
      DrawStringDecal(olc::vf2d(5, y_offset + border), "Day " + to_string(it->first));
      DrawStringDecal(olc::vf2d(60, y_offset + border), it->second);
    }

		return true;
	}
};


int main(const int argc, const char **argv) {
  int8_t daynum = argc > 1 ? stoi(argv[1]) : -1;

  Advent2019 program(daynum);

  program.Construct(600, 400, 2, 2);
  program.Start();

  pge = &program;

  return 0;
}
