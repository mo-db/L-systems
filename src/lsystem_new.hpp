// lsystem_new.hpp
#pragma once
#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"

namespace lsystem_new {
static constexpr int textfield_size = 512;
struct Args {
	std::string x, y, z;
	constexpr std::string& operator[](std::size_t i) noexcept {
			return (i==0 ? x : (i==1 ? y : z));
	}
};

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

	uint32_t color{0xFF00FFFF};
	float width{3.f};

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
		turtle.node_id = new_node_id;
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
	char symbol{0};
	char textfield_condition[textfield_size]{};
	char textfield_rule[textfield_size]{};
	util::STATE condition_state = util::STATE::FALSE; // TODO
};

struct LstringSpec {
	int current_iteration{};
	int iterations{1};
	bool generation_started{false};
	char axiom[app::gui.textfield_size]{};
	std::vector<Rule> rules;
	inline void add_rule() { rules.push_back(Rule{}); }
	inline std::string generate(std::string &lstring) { return lstring; }
};

// ---- object for the whole lsystem data ----
inline bool plants_need_redraw{true};
inline bool plants_redrawing{false};
inline int plants_drawn{0};
enum class DefaultVar : size_t { move = 0, rotate = 1, width = 2, COUNT = 3 };
enum class GlobalVar : size_t { h = 0, i = 1, j = 2, k = 3, COUNT = 4 };
struct Module {
  Plant plant;
  LstringSpec lstring_spec;
	std::string lstring{};
	bool generation_started{false};
  std::vector<Var> default_vars{{"default branch-length", 50.0f},
                                {"default rotation-angle", gk::pi / 2.0f},
                                {"default width-change", 1.0f}};
  std::vector<Var> global_vars{{"h"}, {"i"}, {"j"}, {"k"}};

	Module(Plant plant_, LstringSpec lstring_spec_) : 
		plant{plant_}, lstring_spec{lstring_spec_} {}

  inline Var &get_default_var(DefaultVar default_var) {
    return default_vars[static_cast<std::size_t>(default_var)];
  }
  inline Var &get_global_var(GlobalVar global_var) {
    return global_vars[static_cast<std::size_t>(global_var)];
  }
};
// inline std::vector<Module> modules;
inline std::unordered_map<int, Module> modules;
inline int module_counter{};

// add module and return id of this module
inline int add_module(Module module) {
  modules.emplace(module_counter++, std::move(module)); // no default ctor required
	return module_counter - 1;
}

// remove module by id, true if successfull
inline bool remove_module(int id) {
	if (lsystem_new::modules.find(id) == lsystem_new::modules.end()) {
		return false;
	}
	lsystem_new::modules.erase(id);
	return true;
}

// serializing
// bool save_rule_as_file(System::Rule &rule, const std::string &save_file_name);
// bool load_rule_from_file(System::Rule &rule, std::string &save_file_name);
// std::optional<std::vector<std::string>> scan_saves();


State expand_lstring(Module &module);
State generate_lstring(Module &module);

std::optional<float> _eval_expr(Module module, std::string &expr_string, float *x,
																 float *y, float *z);
Vec2 _calculate_move(Plant &plant, const float length);
void _turn(Turtle &turtle, const float angle);
void _turtle_action(Module &module, const char c, const float *value);
Plant generate_plant(const Vec2 start, const std::string lstring);
bool generate_plant_timed(Module &module);
bool draw_plants_timed(draw::FrameBuf &fb);


// new lstring generation
bool op_is_valid(const char op);
bool try_block_match(const char op1, const char op2,
														const std::string expr1, const std::string expr2);
// int parse_args(const std::string &args, std::string &x, std::string &y,
//                      std::string &z);
int parse_args(const std::string &args_str, Args &args);
bool parse_block(const std::string &block, char &op, std::string &expr, int *n);
bool parse_arg(const std::string arg, std::string &base, std::string &pattern,
							 std::vector<std::string> &repeats, std::string & scale);
std::string arg_rulearg_substitute(const std::string arg, const std::string rule_arg);

std::string _maybe_apply_rule(Module &module, const char symbol, const std::string args);

// calculate x,y,z args for a symbol, if x = 0.0 -> error, y,z default to 0.0
std::array<float, 3> symbol_eval_args(Module &module, const char symbol, const std::string &args);

std::optional<float> get_default(Module &module, const char symbol);

void update_vars(Module &module);

} // namespace lsystem_new
