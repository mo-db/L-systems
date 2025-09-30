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
	B = 2,
	b = 3,
  RotateLeft = 4,
  RotateRight = 5,
  IncWidth = 6,
  DecWidth = 7,
  Count = 8
};
constexpr std::array<char, static_cast<std::size_t>(Symbol::Count)> symbols {
    'A', 'a', 'B', 'b', '-', '+', '^', '&'};
inline char get_symbol(Symbol symbol) {
  return symbols[static_cast<std::size_t>(symbol)];
}
enum class SymbolCategory : size_t { Move = 0, Rotate = 1, Width = 2, COUNT = 3 };


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

struct GenerationManager {
	int current_iteration{};
	int iterations{};

	bool reset_needed{false};
	bool done_generating{false};
	int current_index{};
	std::string lstring_buffer{};
	char axiom[app::gui.textfield_size]{};
	std::vector<Rule> rules;
	inline void add_rule() { rules.push_back(Rule{}); }
};

// ---- object for the whole lsystem data ----
inline bool plants_need_redraw{true};
inline bool plants_redrawing{false};
inline int plants_drawn{0};
enum class GlobalVar : size_t { h = 0, i = 1, j = 2, k = 3, COUNT = 4 };
struct Module {
  Plant plant;
  GenerationManager generation_manager{};
	std::string lstring{};
  std::vector<Var> default_vars{{"default branch-length", 50.0f},
                                {"default rotation-angle", gk::pi / 2.0f},
                                {"default width-change", 1.0f}};
  std::vector<Var> global_vars{{"h"}, {"i"}, {"j"}, {"k"}};

	// rework
	std::unordered_map<SymbolCategory, double> symbol_defaults 
			{ {SymbolCategory::Move, 50.0}, {SymbolCategory::Rotate, gk::pi / 2.0}, 
				{SymbolCategory::Width, 50.0}};
  std::unordered_map<std::string, double> global_variables{
      {"h", 0.0}, {"i", 0.0}, {"k", 0.0}, {"k", 0.0}};

  Module(Plant plant_) : plant{plant_} {}
  Module(const Vec2 &position, const float starting_angle)
      : plant{position, starting_angle} {}

  inline Var &get_default_var(SymbolCategory symbol_category) {
    return default_vars[static_cast<std::size_t>(symbol_category)];
  }
  inline Var &get_global_var(GlobalVar global_var) {
    return global_vars[static_cast<std::size_t>(global_var)];
  }
  State update_vars();

  std::optional<float> evaluate_expression(std::string &expr_string, float *x,
                                           float *y, float *z, bool quantize);
};

class LsystemManager {
	int module_count{};
public:
	std::unordered_map<int, std::unique_ptr<Module>> modules;
	Module* get_module(int id) {
		if (modules.find(id) == modules.end()) {
			return nullptr;
		}
		return modules.at(id).get();
	}

	int add_module(const Vec2 &position, const float starting_angle) {
		modules.emplace(module_count++, std::make_unique<Module>(position, starting_angle));
		return module_count - 1;
	}

	State remove_module(int id) {
		Module* module = get_module(id);
		if (!module) {
			print_info("no module found specified id");
			return State::Error;
		}
		modules.erase(id);
		return State::True;
	}

	State draw_plants_timed(draw::FrameBuf &fb);
};

// serializing
// bool save_rule_as_file(System::Rule &rule, const std::string &save_file_name);
// bool load_rule_from_file(System::Rule &rule, std::string &save_file_name);
// std::optional<std::vector<std::string>> scan_saves();


// make member of lstring
State expand_lstring(Module* module, bool iterate);
void clear_lstring(Module* module);
State regenerate_lstring(Module* module);
State expand(Module* module, const std::string& string_to_expand);

// make member of module
std::string _maybe_apply_rule(Module &module, const char symbol, const std::string args);
std::optional<float> _eval_expr(Module module, std::string &expr_string, float *x,
																 float *y, float *z);
void update_vars(Module &module);
// TODO use the implementation inside module, check for error and return empty
std::optional<float> get_default(Module &module, const char symbol);


// member of turtle



// free in namespace
bool op_is_valid(const char op);
std::string arg_rulearg_substitute(const std::string arg, const std::string rule_arg);
int parse_args(const std::string &args_str, ArgsString &args);
bool try_block_match(const char op1, const char op2,
														const std::string expr1, const std::string expr2);
bool parse_block(const std::string &block, char &op, std::string &expr, int *n);
bool parse_arg(const std::string arg, std::string &base, std::string &pattern,
							 std::vector<std::string> &repeats, std::string & scale);




Vec2 _calculate_move(Plant &plant, const float length);
void _turn(Turtle &turtle, const float angle);
void _turtle_action(Module &module, const char c, const float *value);
Plant generate_plant(const Vec2 start, const std::string lstring);
bool generate_plant_timed(Module &module);
bool draw_plants_timed(draw::FrameBuf &fb);


// new lstring generation



// calculate x,y,z args for a symbol, if x = 0.0 -> error, y,z default to 0.0
ArgsValue symbol_eval_args(Module &module, const char symbol, const std::string &args);

std::optional<float> evaluate_expression_vector(
		std::string &expression_string, std::vector<Var> vars, bool quantize);

std::optional<float> evaluate_expression_map(
		std::string &expression_string, std::unordered_map<std::string, float> vars, bool quantize);



// rework
State update_module(Module* module);
std::optional<SymbolCategory> get_symbol_category(const char ch);


} // namespace lsystem_new
