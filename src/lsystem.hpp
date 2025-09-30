// lsystem.hpp
#pragma once
#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"

namespace lsystem {
static constexpr int textfield_size = 512;
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
  COUNT = 3
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
	std::unordered_map<SymbolCategory, double> symbol_defaults 
			{ {SymbolCategory::Move, 50.0}, {SymbolCategory::Rotate, gk::pi / 2.0}, 
				{SymbolCategory::Width, 50.0}};
  std::unordered_map<std::string, double> global_variables{
      {"h", 0.0}, {"i", 0.0}, {"j", 0.0}, {"k", 0.0}};

  Module(Plant plant_) : plant{plant_} {}
  Module(const Vec2 &position, const float starting_angle)
      : plant{position, starting_angle} {}

  State update_vars();
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
};


State update_module(Module* module, draw::FrameBuf& fb);
std::vector<double> get_args_values(const std::string args_string);
std::vector<std::string> split_args(const std::string &args_string);


// ---- lstring ----
State reset_lstring(Module* module);
State generate_lstring_timed(Module* module);
State _expand_lstring(Module* module);
std::string maybe_apply_rule(Module* module, const char symbol, std::vector<double> args);

std::optional<double>
evaluate_expression(std::unordered_map<std::string, double>& local_variables,
										std::unordered_map<std::string, double>& global_variables,
                    const std::string& expression_string);

std::optional<std::string>
evaluate_production(std::unordered_map<std::string, double>& local_variables,
										std::unordered_map<std::string, double>& global_variables,
                    const std::string& production);

// ---- symbol ----
char get_symbol(SymbolIdentifier symbol_identifier);
std::optional<SymbolCategory> get_symbol_category(const char ch);

// ---- turtle ----
Vec2 _calculate_move(Plant &plant, const float length);
void _turn(Turtle &turtle, const float angle);
State _turtle_action(Plant& plant, const char symbol, double x);

// ---- plant ----
State generate_plant_timed(Module* module);
State draw_plant_timed(Module* module, draw::FrameBuf &fb);
} //namespace lsystem
