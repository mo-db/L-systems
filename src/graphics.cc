#include "graphics.hpp"

Vec2 operator+(const Vec2 &a, const Vec2 &b) {
	return {a.x + b.x, a.y + b.y};
}
Vec2 operator-(const Vec2 &a, const Vec2 &b) {
	return {a.x - b.x, a.y - b.y};
}
Vec2 operator*(const Vec2 &v, double d) {
	return {v.x * d, v.y * d};
}
Vec2 operator*(double d, const Vec2 &v) {
	return v * d;
}

namespace vec2 {
double dot(const Vec2 &a, const Vec2 &b) {
	return a.x * b.x + a.y * b.y;
}
double distance(const Vec2 &a, const Vec2 &b) {
  return std::sqrt(std::pow(a.x - b.x, 2.0) + std::pow(a.y - b.y, 2.0));
}
bool equal_epsilon(const Vec2 &a, const Vec2 &b) {
	if (util::equal_epsilon(a.x, b.x) && util::equal_epsilon(a.y, b.y)) {
		return true;
	} else {
		return false;
	}
}
bool equal_iepsilon(const Vec2 &a, const Vec2 &b) {
	if (util::equal_iepsilon(a.x, b.x) && util::equal_iepsilon(a.y, b.y)) {
		return true;
	} else {
		return false;
	}
}
double get_angle(Vec2 P, Vec2 Q) {
	Vec2 v = Q - P;
  double angle = -std::atan2(v.y, v.x); // because (0,0) is up-left
	if (angle < 0) { angle += 2 * gk::pi; }
	return angle;
}
} // namespace vec2

namespace line2 {
Vec2 project_point(const Line2 &line, const Vec2 &p) {
	Vec2 a = line.get_a();
	double k = ((line.p1.x * a.x + line.p1.y * a.y) -
							(p.x * a.x + p.y * a.y)) / (a.x * a.x + a.y * a.y);
	return k * a + p;
}

bool point_in_segment_bounds(const Line2 &line, const Vec2 &P) {
  double distance_to_far_endpoint = std::max(vec2::distance(line.p1, P),
																						 vec2::distance(line.p2, P));
  return distance_to_far_endpoint <= vec2::distance(line.p1, line.p2);
}

double get_distance_point_to_ray(const Line2 &line, const Vec2 &P) {
	Vec2 a = line.get_a();
	return std::abs((a.x * P.x + a.y * P.y +
									(-a.x * line.p1.x - a.y * line.p1.y)) / a.get_mag());
}
double get_distance_point_to_seg(const Line2 &line, const Vec2 &P) {
  Vec2 projected_point = project_point(line, P);
  if (point_in_segment_bounds(line, projected_point)) {
    return get_distance_point_to_ray(line, P);
  } else {
    return std::min(vec2::distance(P, line.p1), vec2::distance(P, line.p2));
  }
}
} // namespace line2
