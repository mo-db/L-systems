// lsystem_new.hpp
#pragma once
#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"

namespace lsystem_new {
static constexpr int textfield_size = 512;

// definition of all possible symbols, for all systems
enum class Symbol : size_t {
  A = 0,
  a = 1,
  rotate_left = 2,
  rotate_right = 3,
  inc_width = 4,
  dec_width = 5,
  COUNT = 6
};
constexpr std::array<char, static_cast<std::size_t>(Symbol::COUNT)> symbols{
    'A', 'a', '-', '+', '^', '&'};
inline char get_symbol(Symbol symbol) {
  return symbols[static_cast<std::size_t>(symbol)];
}

// declaration of the Var type
struct Var {
  std::string label;
  float value{0.0};
  bool use_slider{true};
  float slider_start{0.0};
  float slider_end{1.0};
  char expr[textfield_size]{0};
  Var(std::string label_) : label{label_} {}
  Var(std::string label_, float value_) : label{label_}, value{value_} {}
};





// ---- Plant ----
struct Branch {
	int node1_id{};
	int node2_id{};
	float wd{0.0f};
	// bool visible{false};
	// char branch_type{0};
};

// the turtle holds all relevant information for drawing a branch
struct Turtle {
	int node_id{};
	float angle{};
	float width{};
	// Turtle() = default;
	Turtle(int node_id_, const float angle_) : node_id{node_id_}, angle{angle_} {}
	void reset(int node_id_, float angle_) {
		node_id = node_id_;
		angle = angle_;
		width = 1.0f;
	}
};

struct Plant {
	Vec2 start_node{};
	float start_angle{};
	std::vector<Vec2> nodes;
  std::vector<Branch> branches;
	Plant(Vec2 start_node_, float start_angle_) :
		start_node{start_node_}, start_angle{start_angle_} {
			nodes.push_back(start_node);
	}

	Turtle turtle{0, start_angle};
	std::vector<Turtle> turtle_stack;

	inline int add_node(Vec2 node) {
		nodes.push_back(node);
		return static_cast<int>(nodes.size()) - 1;
	}

	inline void grow(Vec2 node) {
		int new_node_id = add_node(node);
		branches.push_back(Branch{turtle.node_id, new_node_id, turtle.width});
		// could return node_id
	}

	bool needs_regen = true;
	bool regenerating = false;
	int current_lstring_index = 0;
	bool needs_redraw = true;
	bool redrawing = false;
	int current_branch = 0;

	void clear() {
		nodes.clear();
		nodes.push_back(start_node);
		branches.clear();
		turtle_stack.clear();
		turtle.reset(0, start_angle);
	}
};

// ---- lstring generation ----
struct Rule {
	std::string symbol;
	char textfield_condition[textfield_size]{};
	char textfield_rule[textfield_size]{};
	util::STATE condition_state = util::STATE::FALSE; // TODO
};

struct LstringSpec {
	int current_iteration{};
	char axiom[app::gui.textfield_size]{};
	std::vector<Rule> rules;
	inline void add_rule() { rules.push_back(Rule{}); }
	std::string expand(std::string &lstring);
};

// ---- object for the whole lsystem data ----
enum class DefaultVar : size_t { move = 0, rotate = 1, width = 2, COUNT = 3 };
enum class GlobalVar : size_t { h = 0, i = 1, j = 2, k = 3, COUNT = 4 };
struct Complex {
  Plant plant;
  LstringSpec lstring_spec;
  std::vector<Var> default_vars{{"default branch-length", 50.0f},
                                {"default rotation-angle", gk::pi / 2.0f},
                                {"default width-change", 1.0f}};
  std::vector<Var> global_vars{{"h"}, {"i"}, {"j"}, {"k"}};

	Complex(Plant plant_, LstringSpec lstring_spec_) : 
		plant{plant_}, lstring_spec{lstring_spec_} {}

  inline Var &get_default_var(DefaultVar default_var) {
    return default_vars[static_cast<std::size_t>(default_var)];
  }
  inline Var &get_global_var(GlobalVar global_var) {
    return global_vars[static_cast<std::size_t>(global_var)];
  }
};
inline std::vector<Complex> complexes;

} // namespace lsystem_new
