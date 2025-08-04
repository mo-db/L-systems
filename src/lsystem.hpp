// lsystem.hpp
#pragma once
#include "core.hpp"
#include "graphics.hpp"

constexpr int MAX_NODES = 4096;

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

namespace turtle {
Vec2 calculate_move(Turtle &turtle, const double length);
void turn(Turtle &turtle, const double angle);
} // namespace turtle



struct Lsystem {
	static constexpr int alphabet_size = 6;
	static constexpr int text_size = 512;
	static constexpr int max_rules = 10;

	// A-K
  const char *alphabet[alphabet_size] = { "A", "a", "B", "b", "+", "-" };
	char axiom[text_size] = "";
	Plant axiomm{};
	struct Rule {
		int letter_index = 0;
		char condition[text_size] = "1.0";
		char text[text_size] = "";
	};
	struct Vars {
		double l{}, m{}, n{}, o{}, p{}, q{}, r{}, s{}, t{}, u{}, v{}, w{};
	} vars;
	std::array<Rule, max_rules> rules;

	// struct Axiom {
	// 	std::string lstring = "";
	// 	Plant plant{};
	// } axiom;
	// std::string axiom = "";
	std::string rule_A = "";
	int iterations = 0;
	float standard_length = 50.0;
	float standard_angle = gk::pi / 6.0;

};

namespace lsystem {
Plant generate_plant(Lsystem &lsystem, const Vec2 start,
                     const std::string lstring);
std::string generate_lstring(Lsystem &lsystem);
std::string assemble_lstring_part(Plant &plant);
} // namespace lsystem
