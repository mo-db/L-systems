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
	bool live = true;
	int iterations = 1;
	float standard_length = 50.0;
	float standard_angle = gk::pi / 2.0;
	float standard_wd = 1.0;
	int standard_branch_seg_count = 1;
	static constexpr int alphabet_size = 6;
	static constexpr int text_size = 512;
	static constexpr int max_rules = 3;


	static constexpr int n_parameters = 4;
	char parameter_strings[n_parameters][text_size] = { "0.0", "0.0", "0.0", "0.0" };
	std::array<double, n_parameters> parameters;

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
		int n_args = 1;
		char condition[text_size] = "0.0";
		FIELD_STATE condition_state = FIELD_STATE::FALSE;
		char text[text_size] = "";
		Plant plant{};
	};
	std::array<Rule, max_rules> rules;
};

namespace lsystem {
// generate plant from lstring

std::optional<std::string> _get_string_between_brackets(std::string in_string, const int index);
std::optional<double> _eval_expr(std::string &expr_string, Lsystem &lsystem,
                                 const double in_x);
Vec2 _calculate_move(Turtle &turtle, const double length);
void _turn(Turtle &turtle, const double angle);
void _turtle_action(Plant &plant, Lsystem &lsystem, const char c,
                    const double *value);
Plant generate_plant(Lsystem &lsystem, const Vec2 start,
                                    const std::string lstring);

// generate the complete lstring from axiom and rules
std::string _maybe_apply_rule(Lsystem &lsystem, const char symbol,
                              const double *x_in);
std::string generate_lstring(Lsystem &lsystem);

// TODO: convert a plant into an lstring
// std::string assemble_lstring_part(Plant &plant);

bool eval_parameters(Lsystem &lsystem);

// save a rule in the $PROJ_ROOT/save folder
bool save_rule_as_file(Lsystem::Rule &rule, const std::string &save_file_name);
bool load_rule_from_file(Lsystem::Rule &rule, std::string &save_file_name);
std::optional<std::vector<std::string>> scan_saves();



std::string subst_arg(const std::string arg, const std::string rule_arg);

} // namespace lsystem
