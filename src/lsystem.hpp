// lsystem.hpp
#pragma once
#include "core.hpp"
#include "graphics.hpp"

struct Branch {
	Vec2 *n1 = nullptr;
	Vec2 *n2 = nullptr;
	char branch_type;
	bool visible = false;
	float wd = 0.0;
};

struct Turtle {
	Vec2 *node = nullptr;
	double angle = gk::pi / 2.0;
  Turtle() = default;
  Turtle(const double angle)
      : angle{angle} {}
};

struct Plant {
	static constexpr int MAX_NODES = 4096;
  std::array<Vec2, MAX_NODES> nodes; // unsorted
	int node_counter = 0;
  std::vector<Branch> branches; // sorts the nodes
	Vec2 *add_node(Vec2 p) {
		nodes[node_counter++] = p;
		return &nodes[node_counter - 1];
	}
	Turtle turtle{};
	std::vector<Turtle> turtle_stack;
};


struct Lsystem {
	int iterations = 0;
	float standard_length = 50.0;
	float standard_angle = gk::pi / 6.0;
	static constexpr int alphabet_size = 6;
	static constexpr int text_size = 512;
	static constexpr int max_rules = 10;
	struct Vars {
		double l{}, m{}, n{}, o{}, p{}, q{}, r{}, s{}, t{}, u{}, v{}, w{};
	} vars;

	// A-K
  const char *alphabet[alphabet_size] = { "A", "a", "B", "b", "+", "-" };
	struct Axiom {
		char text[text_size] = "";
		Plant plant;
	} axiom;
	struct Rule {
		int symbol_index = 0;
		char condition[text_size] = "1.0";
		char text[text_size] = "";
		Plant plant{};
	};
	std::array<Rule, max_rules> rules;
};

namespace lsystem {
// generate plant from lstring
Vec2 _calculate_move(Turtle &turtle, const double length);
void _turn(Turtle &turtle, const double angle);
void _turtle_action(Plant &plant, Lsystem &lsystem, const char c,
                    const double *value);
Plant generate_plant(Lsystem &lsystem, const Vec2 start,
                     const std::string lstring);

// generate the complete lstring from axiom and rules
double _eval_expr(std::string &expr_string, Lsystem &lsystem, const double in_x,
                  const double in_y, const double in_z);
std::string _maybe_apply_rule(Lsystem &lsystem, const char symbol,
                              const double *x_in);
std::string generate_lstring(Lsystem &lsystem);

// TODO: convert a plant into an lstring
std::string assemble_lstring_part(Plant &plant);
} // namespace lsystem
