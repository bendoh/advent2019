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

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine/olcPixelGameEngine.h"

using namespace std;

vector<std::string> input_lines;
bool is_interactive = false;
bool is_visual = false;

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





/** Day 3! **/
struct Point {
  int32_t x = 0;
  int32_t y = 0;
  int32_t z = 0;

  int32_t manhattan_distance() { return abs(x) + abs(y) + abs(z); }
};

struct Segment {
    Point start, end;
    int val;
    bool operator<(const Segment& rhs) const {
        return val < rhs.val;
    }
};

bool intersects(const Segment& vertical, const Segment& horizontal) {
    int y = horizontal.start.y;
    int x = vertical.start.x;
    int xub = std::max(horizontal.start.x, horizontal.end.x);
    int xlb = std::min(horizontal.start.x, horizontal.end.x);
    int yub = std::max(vertical.start.y, vertical.end.y);
    int ylb = std::min(vertical.start.y, vertical.end.y);
    return  x >= xlb && x <= xub &&
            y >= ylb && y <= yub;
}

void find_intersections(
        const std::vector<Segment>& vertical,
        const std::vector<Segment>& horizontal,
        std::vector<Point>& intersections
) {
    for(Segment s : vertical) {
        // get interval to iterate over.
        int lb = std::min(s.start.y, s.end.y);
        int ub = std::max(s.start.y, s.end.y);

        // find first (smallest) occurance in horizontal
        int l = 0;
        int r = horizontal.size()-1;
        int m;
        while (l < r) {
            m = (l + r) / 2;
            if (horizontal[m].val < lb) {
                l = m + 1;
            } else if (horizontal[m].val >= lb) {
                r = m;
            }
        }

        // walk through all horizontal lines within the interval and check for intersections
        for (int i = m; i < horizontal.size(); i++) {
            Segment h = horizontal[i];
            if (s.val > ub)
                break;
            if (intersects(s, h)) {
              printf("segment (%d,%d),(%d,%d),%d intersects (%d,%d),(%d,%d),%d\n",
                  s.start.x, s.start.y, s.end.x, s.end.y, s.val,
                  h.start.x, h.start.y, h.end.x, h.end.y, h.val
                );

              intersections.push_back({s.start.x, horizontal[i].start.y});
            }
        }
    }
}

template <typename T>
void ascending(T& dFirst, T& dSecond)
{
    if (dFirst > dSecond)
        std::swap(dFirst, dSecond);
}



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
  int8_t daynum = 0;
  int8_t currentDay;
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
        "total fuel needed for all mass: %d\n\
        total fuel needed for all mass + all fuel: %d",
        total_fuel_part1,
        total_fuel_part2
        );

    day_complete = true;
    return string(result);
  }

  /** Day2 **/
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

  string day2() {
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

    uint32_t result_1 = day2_intcode_processor(program);
    char part1_result[255], part2_result[255];
    snprintf(part1_result, 255, "Part 1: With noun=12 & verb=2: %i", result_1);
    cout << part1_result << "\n";

    uint32_t part2_search = 19690720;

    for (noun = 0; noun < 99; noun++) {
      for (verb = 0; verb < 99; verb++) {
        program = program_template;
        program[1] = noun;
        program[2] = verb;
        uint32_t result_2 = day2_intcode_processor(program);

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

  const uint16_t day3_speed = 1000;

  void draw_wire(vector<Point> wire, olc::Pixel p, olc::vi2d offset, olc::vf2d scale) {

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

  string day3() {
    vector<vector<Point>> wires;
    int num_wires = input_lines.size();

    vector<Segment> horizontals[num_wires];
    vector<Segment> verticals[num_wires];

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

        if (dircode == 'U' || dircode == 'D') {
          verticals[wire_num].push_back({ point, next, next.x });
        } else {
          horizontals[wire_num].push_back({ point, next, next.y });
        }
        points.push_back(next);
        point = next;
      }

      wires.push_back(points);
      sort(horizontals[wire_num].begin(), horizontals[wire_num].end());
      sort(verticals[wire_num].begin(), verticals[wire_num].end());
    }

    vector<Point> intersections;
    find_intersections(verticals[1], horizontals[0], intersections);
    find_intersections(verticals[0], horizontals[1], intersections);

    for (auto &intersection : intersections) {
      printf("Intersection at (%d, %d)\n", intersection.x, intersection.y);
    }
    int min_d = INT_MAX; // something big
    for (int i = 0; i < intersections.size(); i++) {
        int d = abs(intersections[i].x) + abs(intersections[i].y);
        if (d == 0)
           continue;
        min_d = (min_d < d) ? min_d : d;
    }
    printf("Minimum distance: %d\n", min_d);

    // Search for intersections by checking each vertex of wire1
    // for a crossing with wire2
    list<Point> crossings;

    // For drawing, figure out boundaries of all wires
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

    // Border in SCREEN space
    uint16_t border = 10;
    olc::vf2d scale(
      abs((float) distanceX / (float) (ScreenWidth() - border * 2)),
      abs((float) distanceY / (float) (ScreenHeight() - border * 2))
    );

    // Offset in WORLD space
    olc::vi2d offset(southwest.x - border * scale.y, southwest.y - border * scale.y);

    printf(
        "Wire bounds are (%d, %d), (%d, %d) with distances (%d, %d) and scales (%f, %f) screenMin=(%d, %d) screenMax=(%d/%d, %d/%d) with offset (%d, %d)\n",
        southwest.x, southwest.y,
        northeast.x, northeast.y,
        distanceX, distanceY,
        scale.x, scale.y,
        (int) ((float)(southwest.x - southwest.x) / scale.x),
        (int) ((float)(southwest.y - southwest.y) / scale.y),
        (int) ((float)(northeast.x - southwest.x) / scale.x),
        ScreenWidth(),
        (int) ((float)(northeast.y - southwest.y) / scale.y),
        ScreenHeight(),
        offset.x, offset.y
    );

    draw_wire(wires[0], olc::RED, offset, scale);
    draw_wire(wires[1], olc::BLUE, offset, scale);

    size_t max_segments = max(wires[0].size(), wires[1].size());
    size_t num_segments = max((size_t) 1, min(max_segments, (size_t) (day_time * day3_speed)));

    for(uint32_t i = 0; i < min(num_segments, wires[0].size()) - 1; i++) {
      Point u1 = wires[0][i];
      Point u2 = wires[0][i+1];

      for(uint32_t j = 0; j < min(num_segments, wires[1].size()) - 1; j++) {
        Point v1 = wires[1][j];
        Point v2 = wires[1][j+1];

        if(u1.x == u2.x && v1.y == v2.y) {
          int32_t Ux = u1.x, Vy = v1.y;
          Point crossing;
          crossing.x = Ux;
          crossing.y = Vy;

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
            DrawCircle((Ux - offset.x) / scale.x, (Vy - offset.y) / scale.y, 5, olc::GREEN);
            crossings.push_back(crossing);
          }
        }
        else if(u1.y == u2.y && v1.x == v2.x) {
          // U is horizontal, V is vertical
          int32_t Uy = u1.y, Vx = v1.x;
          Point crossing;
          crossing.x = Vx;
          crossing.y = Uy;

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
            DrawCircle((Vx - offset.x) / scale.x, (Uy - offset.y) / scale.y, 5, olc::GREEN);
            crossings.push_back(crossing);
          }
        }
      }
    }

    Point *nearest = nullptr;

    for (auto &crossing: crossings) {
      if(crossing.x != 0 || crossing.y != 0) {
        uint32_t distance = crossing.manhattan_distance();

        printf("distance (%d, %d): %d\n", crossing.x, crossing.y, distance);
        if (nearest == nullptr || distance < nearest->manhattan_distance())
          nearest = &crossing;
      }
    }

    if (num_segments == max_segments) {
      day_complete = true;
    }

    if (nearest != nullptr) {
      DrawCircle((nearest->x - offset.x) / scale.x, (nearest->y - offset.y) / scale.y, 3, olc::CYAN);

      string result = "Nearest: " + to_string(nearest->manhattan_distance());
      return result;
    } else {
      return "";
    }
  }

  vector<string(Advent2019::*)()> days;

  Advent2019(int8_t _daynum) : days {
    &Advent2019::day0, &Advent2019::day1, &Advent2019::day2, &Advent2019::day3
  } {
    daynum = _daynum;

    if (daynum > -1)
      currentDay = daynum;
    sAppName = "Bendoh's Advent 2019";

    printf("Starting up. There are %lu days attempted.\n", days.size());

    if (daynum > -1) {
      printf("Only doing day %d\n", daynum);
    } else {
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

	bool OnUserUpdate(float fElapsedTime) override {
    if (!error_state.empty()) {
      DrawStringDecal(olc::vf2d(5, 5), error_state);
      return true;
    }

    if (currentDay > -1) {
      if (starting_day) {
        if ((daynum == 0 || input_lines.empty()) && currentDay > 0) {
          string input_file = "inputs/2019/" + to_string(currentDay);
          cout << "Getting input from " << input_file << "\n";
          FILE *input = fopen(input_file.c_str(), "r");

          if (input == NULL) {
            error_state = "Couldn't open puzzle input " + input_file + "\n";
            return true;
          }

          // If lines are longer than 10k we have a problem...
          char input_line[10240];
          input_lines = {};
          while(fgets(input_line, 10240, input))
            input_lines.push_back(string(input_line));
          fclose(input);
        }
        day_time = 0;
      }

      string (Advent2019::*func)() = days[currentDay];

      Clear(olc::BLACK);

      if (starting_day) {

        cout << "Day " << to_string(currentDay) << " started\n";
        // Day is started, start computing next frame
        starting_day = false;
        day_complete = false;
      } else {
        cout << "Computing " << to_string(currentDay) << " ...\n";
        day_time += fElapsedTime;

        // Day is started, start computing next frame
        string result = (this->*func)();

        if (day_complete) {
          results[currentDay] = result;
          cout << result << "\n";

          if (daynum == 0) {
            if (currentDay >= days.size()) {
              currentDay = -1; // We are done.
            } else {
              currentDay++; // Move on to the next puzzle
              starting_day = true;
            }
          } else {
            currentDay = -1; // We are done.
          }
        }
      }
    }

    for (map<uint8_t, string>::iterator it=results.begin(); it!=results.end(); ++it) {
      DrawStringDecal(olc::vf2d(5, 45), "Day " + to_string(it->first) + ": " + it->second);
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
