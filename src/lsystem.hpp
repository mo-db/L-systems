// lsystem.hpp
#pragma once
#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"

// lindenmayer-system
namespace lm {

struct Args {
	std::string x, y, z;
    constexpr std::string& operator[](std::size_t i) noexcept {
        return (i==0 ? x : (i==1 ? y : z));
    }
};

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

struct System {
	bool live = true;
	int iterations = 1;
	int current_iteration = 0;
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
	// std::string lstring = "";

	// A-K
  const char *alphabet[alphabet_size] = { "A", "a", "B", "b", "+", "-" };

	char axiom[app::gui.textfield_size] = "";
	struct Rule {
		int symbol_index = 0;
		int n_args = 1;
		char condition[text_size] = "0.0";
		util::STATE condition_state = util::STATE::FALSE;
		char text[text_size] = "";
		Plant plant{};
	};
	std::array<Rule, max_rules> rules;
};
inline System system;
inline std::string lstring = "";



// generate plant from lstring

// std::optional<double> _eval_expr(std::string &expr_string, const double in_x);
std::optional<double> _eval_expr(std::string &expr_string, double *x,
																 double *y, double *z);
Vec2 _calculate_move(Turtle &turtle, const double length);
void _turn(Turtle &turtle, const double angle);
void _turtle_action(Plant &plant, const char c, const double *value);
Plant generate_plant(const Vec2 start, const std::string lstring);

// generate the complete lstring from axiom and rules



// serializing
bool save_rule_as_file(System::Rule &rule, const std::string &save_file_name);
bool load_rule_from_file(System::Rule &rule, std::string &save_file_name);
std::optional<std::vector<std::string>> scan_saves();

// new lstring generation
bool op_is_valid(const char op);
bool try_block_match(const char op1, const char op2,
														const std::string expr1, const std::string expr2);
int parse_args(const std::string &args, std::string &x, std::string &y,
                     std::string &z);
int parse_args2(const std::string &args_str, Args &args);
bool parse_block(const std::string &block, char &op, std::string &expr, int *n);
bool parse_arg(const std::string arg, std::string &base, std::string &pattern,
							 std::vector<std::string> &repeats, std::string & scale);
std::string arg_rulearg_substitute(const std::string arg, const std::string rule_arg);

std::string _maybe_apply_rule(const char symbol, const std::string args);
std::string generate_lstring();
std::string expand(std::string lstring);

// calculate x,y,z args for a symbol, if x = 0.0 -> error, y,z default to 0.0
std::array<double, 3> symbol_eval_args(const char symbol, const std::string &args);

std::optional<double> get_default(const char symbol);

// into util
std::string get_substr(const std::string &str, const int index, const char c);
std::string trim(const std::string &s);
} // namespace lsystem
