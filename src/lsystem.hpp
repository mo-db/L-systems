// lsystem.hpp
#pragma once
#include "core.hpp"
#include "app.hpp"
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
	static constexpr int MAX_NODES = 10000;
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
	float standard_angle = gk::pi / 2.05;
	static constexpr int alphabet_size = 6;
	static constexpr int text_size = 512;
	static constexpr int max_rules = 3;

	// let parameters be text fields that can be evaluated
	// default value of text field must be 0.0
	// textfield parameters refer to each other hirarchical to evaluate

	// maybe use std::pair<> for this? instead of a char* array??
	
	// indexes: 0 = l, 1 = m, 2 = n ...
	
	// std::pair<char, char[text_size]> 


	static constexpr int n_parameters = 4;
	char parameter_strings[n_parameters][text_size] = { "0.0", "0.0", "0.0", "0.0" };

	// store parameters in an array and just as user experiance have letters

	std::array<double, n_parameters> parameters;

	// struct Parameters {
	// 	double l{}, m{}, n{}, o{}, p{}, q{}, r{}, s{}, t{}, u{}, v{}, w{};
	// } parameters;

	Plant plant{};
	std::string lstring = "";

	// A-K
  const char *alphabet[alphabet_size] = { "A", "a", "B", "b", "+", "-" };
	enum class FIELD_STATE {
		TRUE,
		FALSE,
		ERROR,
	};
	struct Axiom {
		char text[text_size] = "";
		Plant plant;
	} axiom;
	struct Rule {
		int symbol_index = 0;
		char condition[text_size] = "0.0";
		FIELD_STATE condition_state = FIELD_STATE::FALSE;
		char text[text_size] = "";
		Plant plant{};
	};
	std::array<Rule, max_rules> rules;
};

namespace lsystem {
// generate plant from lstring

int _get_expr_string(const std::string &in_string, std::string &expr_string, int iter);
Vec2 _calculate_move(Turtle &turtle, const double length);
void _turn(Turtle &turtle, const double angle);
void _turtle_action(Plant &plant, Lsystem &lsystem, const char c,
                    const double *value);
Plant generate_plant(Lsystem &lsystem, const Vec2 start,
                     const std::string lstring);

// generate the complete lstring from axiom and rules
std::optional<double> _eval_expr(std::string &expr_string, Lsystem &lsystem, const double in_x);
std::string _maybe_apply_rule(Lsystem &lsystem, const char symbol,
                              const double *x_in);
std::string generate_lstring(Lsystem &lsystem);

// TODO: convert a plant into an lstring
// std::string assemble_lstring_part(Plant &plant);



ExitState eval_parameters(Lsystem &lsystem);



// save a rule in the $PROJ_ROOT/save folder
ExitState save_rule_as_file(Lsystem::Rule &rule, const std::string &save_file_name);

ExitState load_rule_from_file(Lsystem::Rule &rule, std::string &save_file_name);
std::optional<std::vector<std::string>> scan_saves();
} // namespace lsystem
