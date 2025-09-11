// lsystem.hpp
#pragma once
#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"

// lindenmayer-system
namespace lm {
struct Args {
	std::string x, y, z;
	constexpr std::string& operator[](std::size_t i) noexcept {
			return (i==0 ? x : (i==1 ? y : z));
	}
};

struct Var {
	char label = '\0';
	char expr[app::gui.textfield_size];
	bool use_slider = false;
	float slider_start = 0.0;
	float slider_end = 1.0;
	float value = 0.0;
	Var(char label) : label{label} {}
};

struct GlobVars2 {
	static constexpr int quant = 5;
	Var l{'l'};
	Var m{'m'};
	Var n{'n'};
	Var o{'o'};
	Var p{'p'};
	Var* var(const int i) {
		switch (i) {
			case 0: return &l;
			case 1: return &m;
			case 2: return &n;
			case 3: return &o;
			case 4: return &p;
			default: return nullptr;
		}
	}
};
inline GlobVars2 glob_vars2;

struct GlobVars {
	static constexpr int amount = 5;
	static constexpr int textfield_size = app::gui.textfield_size;
	bool l_use_slider = false;
	float l_slider_start = 0.0;
	float l_slider_end = 1.0;
	char l[app::gui.textfield_size];
	float l_value = 0.0;




	char m[app::gui.textfield_size];
	char n[app::gui.textfield_size];
	char o[app::gui.textfield_size];
	char p[app::gui.textfield_size];
	float m_value = 0.0;
	float n_value = 0.0;
	float o_value = 0.0;
	float p_value = 0.0;

	constexpr char* operator[](const int i) noexcept {
		switch (i) {
			case 0: return l;
			case 1: return m;
			case 2: return n;
			case 3: return o;
			case 4: return p;
			default: return nullptr;
		}
	}
	float* value(const int i) {
		switch (i) {
			case 0: return &l_value;
			case 1: return &m_value;
			case 2: return &n_value;
			case 3: return &o_value;
			case 4: return &p_value;
			default: return nullptr;
		}
	}

	char get_label(const int i) {
		switch (i) {
			case 0: return 'l';
			case 1: return 'm';
			case 2: return 'n';
			case 3: return 'o';
			case 4: return 'p';
			default: return '\0';
		}
	}
	// for all default values and global vars do:
	// [var text field] [var slider, expo || log || normal]
	// > Defaults []
	// > GlobVars []
};
inline GlobVars glob_vars;

struct Branch {
	Vec2 *n1 = nullptr;
	Vec2 *n2 = nullptr;
	char branch_type;
	bool visible = false;
	float wd = 0.0;
};

struct Turtle {
	Vec2 *node = nullptr;
	double angle{};
	Turtle() = default;
	Turtle(Vec2 *node, const double angle) : node{node}, angle{angle} {}
};

struct Plant {
	Vec2 start_node{};
	double start_angle{};

	static const std::size_t max_nodes = 1000000; // mil
	// std::unique_ptr<Vec2[]> nodes; // heap array
	std::array<Vec2, max_nodes> nodes;

	int node_count = 0;
  std::vector<Branch> branches;

	Turtle turtle{};
	std::vector<Turtle> turtle_stack;

	Plant() = default;
	Plant(const Vec2 start_node, const double start_angle) :
		start_node{start_node}, start_angle{start_angle}
		// nodes{std::make_unique<Vec2[]>(max_nodes)} 
	{
		turtle.node = add_node(start_node);
		turtle.angle = start_angle;
	}

	void init(const Vec2 start_node_, const double start_angle_) {
		start_node = start_node_;
		start_angle = start_angle_;
		turtle.node = add_node(start_node_);
		turtle.angle = start_angle_;
	}

	Vec2 *add_node(Vec2 node) {
		if (node_count >= max_nodes) { return nullptr; }
		nodes[node_count++] = node;
		return &nodes[node_count - 1];
	}

	bool needs_regen = true;
	bool regenerating = false;
	int current_lstring_index = 0;
	bool needs_redraw = true;
	bool redrawing = false;
	int current_branch = 0;

	void clear() {
		branches.clear();
		node_count = 0;
		turtle.node = add_node(start_node);
		turtle.angle = start_angle;
		turtle_stack.clear();
	}
};

struct System {
	bool live = true;

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

	// A-K
  const char *alphabet[alphabet_size] = { "A", "a", "B", "b", "+", "-" };

	// only this
	int iterations = 1;
	int current_iteration = 0;
	std::string lstring = "";
	char axiom[app::gui.textfield_size] = "";
	struct Rule {
		int symbol_index = 0;
		int n_args = 1;
		char condition[text_size] = "0.0";
		util::STATE condition_state = util::STATE::FALSE;
		char text[text_size] = "";
	};
	std::array<Rule, max_rules> rules;
	bool expand();
	bool generate() {
		current_iteration = 0;
		for (int i = 0; i < iterations; i++) {
			if(!expand()) { return false; }
		}
		return true;
	}
};

inline System system;
inline Plant plant;

// generate plant from lstring

// std::optional<double> _eval_expr(std::string &expr_string, const double in_x);
std::optional<double> _eval_expr(std::string &expr_string, double *x,
																 double *y, double *z);
Vec2 _calculate_move(Turtle &turtle, const double length);
void _turn(Turtle &turtle, const double angle);
void _turtle_action(Plant &plant, const char c, const double *value);
Plant generate_plant(const Vec2 start, const std::string lstring);
bool generate_plant_timed(const std::string &lstring, Plant &plant, 
		int &current_index);
bool draw_plant_timed(const lm::Plant &plant, int &current_branch,
											uint32_t color, draw::FrameBuf &fb);

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

// calculate x,y,z args for a symbol, if x = 0.0 -> error, y,z default to 0.0
std::array<double, 3> symbol_eval_args(const char symbol, const std::string &args);

std::optional<double> get_default(const char symbol);

void update_vars();

// into util
std::string get_substr(const std::string &str, const int index, const char c);
std::string trim(const std::string &s);
} // namespace lsystem
