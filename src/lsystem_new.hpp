// lsystem.hpp
#pragma once
#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"

namespace lsystem_new {

constexpr int textfield_size = 4096;

enum class SymbolIdentifier : size_t {
  A = 0,
  a = 1,
  B = 2,
  b = 3,
  RotateLeft = 4,
  RotateRight = 5,
  IncWidth = 6,
  DecWidth = 7,
  Count = 8
};
constexpr std::array<char, static_cast<std::size_t>(SymbolIdentifier::Count)>
    symbols{'A', 'a', 'B', 'b', '-', '+', '^', '&'};

enum class SymbolCategory : size_t {
  Move = 0,
  Rotate = 1,
  Width = 2,
  // Color = 3,
};

std::optional<SymbolCategory> get_symbol_category(const char ch);


// // ---- Plant ----
// struct Branch {
// 	int node1_id{};
// 	int node2_id{};
// 	double width{};
// 	// uint32_t color{};
// };
//
// struct Turtle {
// 	Vec2 position{};
// 	double angle{};
// 	double width{};
// 	Turtle() = default;
// 	Turtle(int node_id_, const float angle_) : node_id{node_id_}, angle{angle_} {}
// 	inline Vec2 move(const double length) {
// 		position.x += length * cos(angle);
// 		position.y += length * -sin(angle);
// 		return position;
// 	}
// 	void _turn(Turtle &turtle, const float angle) { turtle.angle += angle; }
// 	void reset(int node_id_, float angle_) {
// 		node_id = node_id_;
// 		angle = angle_;
// 		width = 1.0f;
// 	}
// };
//
// // builder has conditions that change how he draws a lstring
// struct Turtle2 {
// 	int id_current_node{};
// 	double angle{};
// 	double width{};
// 	uint32_t color{};
// 	Plant* plant;
// 	bool grow_branch(std::vector<Vec2>& nodes, std::vector<Branch>& branches,
// 			const double length) {
// 		if (!plant) { return false; }
// 		Vec2 position = nodes[id_current_node];
// 		position.x += length * cos(angle);
// 		position.y += length * -sin(angle);
// 		nodes.push_back(position);
// 		int id_new_node = static_cast<int>(nodes.size()) - 1;
// 		branches.push_back(Branch{id_current_node, id_new_node, width});
// 	}
// 	inline void _turn(Turtle &turtle, const float angle) { turtle.angle += angle; }
// 	void change_width();
// 	void change_hue();
// };
//
// struct Plant {
//   Vec2 base_node{static_cast<double>(app::video.width) / 2.0,
//                  static_cast<double>(app::video.height) / 2.0};
//   double base_angle{2 * gk::pi};
//   double base_width{3.0};
//   uint32_t base_color{0xFF00FFFF};
//
//   std::vector<Vec2> nodes;
//   std::vector<Branch> branches;
// 	Plant() = default;
// 	Plant(Vec2 base_node_, float base_angle_) :
// 		base_node{base_node_}, base_angle{base_angle_} {
// 			nodes.push_back(base_node);
// 	}
//
// 	Turtle turtle{0, start_angle};
// 	std::vector<Turtle> turtle_stack;
//
// 	inline int grow(const double length) {
// 		Vec2 new_node_position = turtle.move(length);
// 		nodes.push_back(new_node_position);
// 		int new_node_id = static_cast<int>(nodes.size()) - 1;
// 		branches.push_back(Branch{turtle.node_id, new_node_id, turtle.width});
// 		turtle.node_id = new_node_id;
// 		return new_node_id;
// 	}
//
// 	bool needs_regen = true;
// 	bool regenerating = false;
// 	int current_lstring_index = 0;
// 	bool needs_redraw = true;
// 	bool redrawing = false;
// 	int current_branch = 0;
//
// 	void clear() {
// 		nodes.clear();
// 		nodes.push_back(start_node);
// 		branches.clear();
// 		turtle_stack.clear();
// 		turtle.reset(0, start_angle);
// 	}
// };

// ---- lstring generation ----


struct Production {
	char symbol{};
	char condition[textfield_size]{};
	char rule[textfield_size]{};
};

struct Generator {
  bool reset_needed{false};
  bool done_generating{false};
  int current_iteration{};
  int iterations{};
  int current_index{};
	std::string lstring{};
  std::string lstring_buffer{};
  char axiom[app::gui.textfield_size]{};
  std::vector<Production> productions;
  inline void add_production() { productions.push_back(Production{}); }
  inline void remove_production() {
    if (productions.empty()) { return; }
    productions.pop_back();
  }

  std::unordered_map<SymbolCategory, double> symbol_defaults{
      {SymbolCategory::Move, 50.0},
      {SymbolCategory::Rotate, gk::pi / 2.0}, // TODO: make this pi * value
      {SymbolCategory::Width, 50.0}};
  std::unordered_map<std::string, double> global_variables{
      {"h", 0.0}, {"i", 0.0}, {"j", 0.0}, {"k", 0.0}};

	// TODO
  State update_vars();
};

struct LsystemManager {
	std::unordered_map<int, std::unique_ptr<Generator>> generators;
	std::vector<std::string*> lstrings;
	Generator* get_generator(int id) {
		if (generators.find(id) == generators.end()) {
			return nullptr;
		}
		return generators[id].get();
	}
	void add_generator(const int id) {
		generators.emplace(id, std::make_unique<Generator>());
	}
	State remove_generator(int id) {
		Generator* generator = get_generator(id);
		if (!generator) {
			print_info("no generator found specified id");
			return State::Error;
		}
		generators.erase(id);
		return State::True;
	}
};

std::optional<SymbolCategory> 
get_symbol_category(const char ch);

std::expected<double, Error>
evaluate_expression(std::unordered_map<std::string, double>& local_variables,
                    std::unordered_map<std::string, double>& global_variables,
                    const std::string& expression_string);

std::expected<std::string, Error>
evaluate_production(std::unordered_map<std::string, double>& local_variables,
                    std::unordered_map<std::string, double>& global_variables,
                    const std::string& production);

std::expected<std::vector<std::string>, Error>
split_arg_block(const std::string &arg_block);

// return substring arg block including '}', index must be set to '{'
std::expected<std::string, Error>
get_arg_block(const std::string lstring, const int index);

std::expected<std::vector<double>, Error>
parse_arg_block(const std::string arg_block);

std::expected<std::vector<double>, Error>
evaluate_arg_block(std::unordered_map<std::string, double>& local_variables,
                   std::unordered_map<std::string, double>& global_variables,
                   const std::string& arg_block);

std::optional<int>
parse_symbol(const std::string lstring, const int index, char& symbol,
             std::vector<double> &args);

std::unordered_map<std::string, double>
args_to_map(std::vector<double> args);

std::expected<std::string, Error>
maybe_apply_rule(Generator* generator, const char symbol, std::vector<double> args);

State
reset_generator(Generator* generator);

State
generate_timed(Generator* generator);

State
_expand_lstring(Generator* generator);

State
update_generator(Generator* generator);

std::expected<void, Error>
test_func(int i);

} // namespace lsystem_new
