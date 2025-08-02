// lsystem.hpp
#pragma once
#include "core.hpp"
#include "graphics.hpp"
// #include "turtle.hpp"

#define MAX_NODES 4096

struct Node {
	Vec2 p{};
	int id = -1;
};

enum struct BranchType {
  A,
  B,
};

struct Branch {
	int n1_id = -1;
	int n2_id = -1;
	char branch_type;
	bool visible = false;
	float wd = 0.0;
};

struct Plant {
  std::vector<Node> nodes;
  int _node_id_cnt = 0;
	// int active_node = -1;
  std::vector<Branch> branches;

  bool node_exists(int id) {
    for (auto &node : nodes) {
      if (node.id == id) {
        return true;
      }
    }
    return false;
  }

  Node get_node(int id) {
    for (auto &node : nodes) {
      if (node.id == id) {
        return node;
      }
    }
    return Node{};
  }

  void add_node(Vec2 position) {
		nodes.push_back({position, _node_id_cnt++});
	}

  bool pop_node(int node_id) {
    for (auto iter = nodes.begin(); iter != nodes.end(); iter++) {
      if ((*iter).id == node_id) {
        nodes.erase(iter);
        return true;
      }
    }
    return false;
  }
};

struct Turtle {
	Vec2 p{};
	int current_node = -1;
  double angle = 0.0;
  Turtle() = default;
  Turtle(int current_node, const double angle)
      : current_node{current_node}, angle{angle} {}
};

namespace turtle {
void move(Turtle &turtle, const double length);
void turn(Turtle &turtle, const double angle);
} // namespace turtle


struct Lsystem {
	std::string alphabet = "A,a,B,b,+,-,";
	struct Axiom {
		std::string lstring = "";
		Plant plant{};
	} axiom;
	// std::string axiom = "";
	std::string rule_A = "";
	int iterations = 0;
	float standard_length = 50.0;
	float standard_angle = gk::pi/2;

};

namespace lsystem {
Plant generate_plant(Turtle &turtle, std::vector<Turtle> &turtle_stack,
                     Lsystem &lsystem, const Vec2 start,
                     const std::string lstring);
std::string generate_lstring(Lsystem &lsystem);
std::string assemble_lstring_part(Plant &plant);
} // namespace lsystem
