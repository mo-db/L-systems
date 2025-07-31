// graphics.hpp
#pragma once
#include "core.hpp"

struct Vec2 {
  double x{0.0}, y{0.0};
  Vec2() = default;
  Vec2(const double x, const double y) : x{x}, y{y} {}
  void norm() { 
		double m = get_mag(); x /= m; y /= m;
	}
	Vec2 get_norm() const {
			Vec2 copy = *this;
			copy.norm();
			return copy;
	}
  double get_mag() const { 
		return std::sqrt(x * x + y * y);
	}
	double get_angle() {
		double angle = -std::atan2(y, x); // because (0,0) is up-left
		if (angle < 0) { angle += 2 * gk::pi; }
		return angle;
	}
};

Vec2 operator+(const Vec2 &a, const Vec2 &b);
Vec2 operator-(const Vec2 &a, const Vec2 &b);
Vec2 operator*(const Vec2 &v, double d);
Vec2 operator*(double d, const Vec2 &v);

namespace vec2 {
double dot(const Vec2 &a, const Vec2 &b);
double dist(const Vec2 &a, const Vec2 &b);
bool equal_iepsilon(const Vec2 &a, const Vec2 &b);
bool equal_epsilon(const Vec2 &a, const Vec2 &b);
} // namespace vec2

struct Line2 {
	Vec2 p1{}, p2{};
	Line2() = default;
	Line2(const Vec2 p1, const Vec2 p2) : p1{p1}, p2{p2} {}
	Vec2 get_v() const { return p2 - p1; }
	Vec2 get_a() const { return Vec2 {p2.y - p1.y, -(p2.x - p1.x)}; }
	double length() const { return vec2::dist(p1, p2); }
};

namespace line2 {
Vec2 project_point(const Line2 &line, const Vec2 &P);
bool point_in_segment_bounds(const Line2 &line, const Vec2 &P);
double get_distance_point_to_ray(const Line2 &line, const Vec2 &P);
double get_distance_point_to_seg(const Line2 &line, const Vec2 &P);
} // namespace line2
