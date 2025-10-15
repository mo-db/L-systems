// lsystem_new.hpp
#pragma once
#include "core.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "rasterize.hpp"

namespace lsystem_new {
constexpr int textfield_size = 4096;

enum class SymbolIdentifier : size_t {
  A = 0,
  a = 1,
  B = 2,
  b = 3,
  RotateLeft = 4,
  RotateRight = 5,
  IncWidth = 6,
  DecWidth = 7,
	IncPalUpper = 8,
	IncPalLower = 9,
  Count = 10,
};
constexpr std::array<char, static_cast<std::size_t>(SymbolIdentifier::Count)>
    symbols{ 'A', 'a', 'B', 'b', '-', '+', '^', '&', '$', '%' };

// constexpr std::array<char[4], static_cast<std::size_t>(SymbolIdentifier::Count)>
//     symbols{"A", "a", "B", "b", "-", "+", "^", "&"};

enum class SymbolCategory : size_t {
  Move = 0,
  Rotate = 1,
  Width = 2,
  Color = 3,
	Stack = 4,
};

std::optional<SymbolCategory> get_symbol_category(const char ch);
inline std::string to_string(SymbolCategory category) {
  switch (category) {
  case SymbolCategory::Move:
    return "Move";
  case SymbolCategory::Rotate:
    return "Rotate";
  case SymbolCategory::Width:
    return "Width";
  case SymbolCategory::Color:
    return "Color";
  case SymbolCategory::Stack:
    return "Stack";
  default:
    return "Unknown";
  }
}

// ---- lstring generation ----

struct Plant;

struct Production {
	char symbol{};
	int n_vars{1}; // range 1 - 3
	char condition[textfield_size]{};
	char rule[textfield_size]{};
};

struct Generator {
	std::forward_list<Plant*> plants;
	void add_plant(Plant* plant) {
		plants.push_front(plant);
	}
	
	void remove_plant(Plant* plant) {
		plants.remove(plant);
	}

	void reset_plants();

  bool reset_needed{false};
  bool done_generating{true};
  int current_iteration{};
  int iterations{1};
  int current_index{};
	std::string lstring{};
  std::string lstring_buffer{};
  char axiom[app::gui.textfield_size]{};
  std::vector<Production> productions;
  inline void add_production() { productions.push_back(Production{}); }
  inline void remove_production() {
    if (productions.empty()) { return; }
    productions.pop_back();
	}
	inline void clear() {
		// iterations = 1;
		current_iteration = 0;
		current_index = 0;
		lstring.clear();
		lstring_buffer.clear();
	}

  std::unordered_map<SymbolCategory, double> symbol_defaults {
      {SymbolCategory::Move, 50.0},
      {SymbolCategory::Rotate, gk::pi / 2.0}, // TODO: make this pi * value
      {SymbolCategory::Width, 50.0},
      {SymbolCategory::Color, 1.0}};
  std::unordered_map<std::string, double> global_variables{
      {"h", 0.0}, {"i", 0.0}, {"j", 0.0}, {"k", 0.0}};

	// TODO
  State update_vars();
};

// ---- building ----
// struct Construct {};

struct Branch {
	Vec2 start{};
	Vec2 end{};
	double width{};
	uint32_t color_upper{0xFF00FFFF};
	uint32_t color_lower{0xFF00FFFF};
	uint32_t color{0xFF00FFFF}; // remove
};

struct Plant {
	// could use generator id
	Generator* generator = nullptr;

	// starting data
	Vec2 start_position{static_cast<double>(app::video.width)/2.0, 
		static_cast<double>(app::video.height)/2.0};
	double start_angle{gk::pi / 2.0};
	double start_width{5.0};
	uint32_t start_color{0xFF00FFFF};

	// building data
	int current_index{};
	std::string lstring{}; // use string view?

	struct Data {
		Vec2 position{};
		double angle{};
		double width{};
		int palette_pos_upper{};
		int palette_pos_lower{};
		uint32_t color{0xFF00FFFF};
	} data;
	std::vector<Data> data_stack{};

	// finished Plant
	int current_branch{};
	std::vector<Branch> branches{};


  bool reset_needed{false};
	bool done_building = false;
	bool done_drawing = false;

	void reset() {
		current_index = 0;
		data.position = start_position;
		data.angle = start_angle;
		data.width = start_width;
		data.palette_pos_upper = 0;
		data.palette_pos_lower = 0;
		data.color = start_color;
		data_stack.clear();
		branches.clear();
		current_branch = 0;
		done_building = false;
		done_drawing = false;
	}
};

inline void Generator::reset_plants() {
	for (auto& plant : plants) {
		plant->reset_needed = true;
	}
}

// plant manager
struct PlantBuilder {
	// environment conditions
  bool done_building{false};
	// std::vector<Plant> plants{};
	std::unordered_map<int, std::unique_ptr<Plant>> plants{};
	
	// temporary
	// Plant plant{};
};


std::expected<bool, Error> build_timed(Plant& plant);
std::expected<bool, Error>
draw_plant_timed(Plant& plant, draw::FrameBuf &fb);

struct LsystemManager {
	std::unordered_map<int, std::unique_ptr<Generator>> generators;
	std::unordered_map<int, std::unique_ptr<PlantBuilder>> plant_builders;
	std::vector<std::string*> lstrings;
	Generator* get_generator(int id) {
		if (generators.find(id) == generators.end()) {
			return nullptr;
		}
		return generators[id].get();
	}
	void add_generator(const int id) {
		generators.emplace(id, std::make_unique<Generator>());
	}
	State remove_generator(int id) {
		Generator* generator = get_generator(id);
		if (!generator) {
			print_info("no generator found specified id");
			return State::Error;
		}
		generators.erase(id);
		return State::True;
	}

	PlantBuilder* get_plant_builder(int id) {
		if (plant_builders.find(id) == plant_builders.end()) {
			return nullptr;
		}
		return plant_builders[id].get();
	}
	void add_plant_builder(const int id) {
		plant_builders.emplace(id, std::make_unique<PlantBuilder>());
	}
	State remove_plant_builder(int id) {
		PlantBuilder* plant_builder = get_plant_builder(id);
		if (!plant_builder) {
			print_info("no plant_builder found specified id");
			return State::Error;
		}
		plant_builders.erase(id);
		return State::True;
	}

	Plant* get_plant(PlantBuilder* builder, int id) {
		if (builder->plants.find(id) == builder->plants.end()) {
			return nullptr;
		}
		return builder->plants[id].get();
	}

	void add_plant(PlantBuilder* builder, const int id) {
		builder->plants.emplace(id, std::make_unique<Plant>());
		Plant* plant = get_plant(builder, id);
		if (!plant) { throw std::runtime_error("Plant does not exist"); }
		// default generator registration
		if (!generators.empty()) {
			plant->generator = generators.begin()->second.get();
			generators.begin()->second->add_plant(plant);
		}
	}
	void remove_plant(PlantBuilder* builder, int id) {
		Plant* plant = get_plant(builder, id);
		if (!plant) { throw std::runtime_error("Plant does not exist"); }
		builder->plants.erase(id);
		// generator unregistration
		if (plant->generator) {
			plant->generator->remove_plant(plant);
		}
	}

};

std::optional<SymbolCategory> 
get_symbol_category(const char ch);

std::expected<double, Error>
evaluate_expression(std::unordered_map<std::string, double>& local_variables,
                    std::unordered_map<std::string, double>& global_variables,
                    const std::string& expression_string);

std::expected<std::string, Error>
evaluate_production(std::unordered_map<SymbolCategory, double>& symbol_defaults,
		 								std::unordered_map<std::string, double>& local_variables,
                    std::unordered_map<std::string, double>& global_variables,
                    const std::string& production);

std::expected<std::vector<std::string>, Error>
split_arg_block(const std::string &arg_block);

// return substring arg block including '}', index must be set to '{'
std::expected<std::string, Error>
get_arg_block(const std::string lstring, const int index);

std::expected<std::vector<double>, Error>
parse_arg_block(const std::string arg_block);

std::expected<std::vector<double>, Error>
evaluate_arg_block(std::unordered_map<std::string, double>& local_variables,
                   std::unordered_map<std::string, double>& global_variables,
                   const std::string& arg_block);

std::optional<int>
parse_symbol(const std::string lstring, const int index, char& symbol,
             std::vector<double> &args);

std::unordered_map<std::string, double>
args_to_map(std::vector<double> args);

std::expected<std::string, Error>
maybe_apply_rule(Generator* generator, const char symbol, std::vector<double> args);

std::expected<void, Error>
reset_generator(Generator* generator);

std::expected<bool, Error>
generate_timed(Generator* generator);

std::expected<bool, Error>
_expand_lstring(Generator* generator);

std::expected<void, Error>
update_generator(Generator* generator);

std::expected<void, Error>
update_builder(PlantBuilder* builder, draw::FrameBuf& fb);

std::expected<void, Error>
update_plant(Plant* plant, draw::FrameBuf& fb);

std::expected<void, Error>
test_func(int i);

// ---- builder ----

} // namespace lsystem_new
