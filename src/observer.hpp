// observer.hpp
#pragma once
#include "core.hpp"
#include "app.hpp"

namespace observer {
class Observer {
	bool& state;
	std::string& lstring;

public:
	Observer(bool& state_, std::string& lstring_) : state(state_), lstring(lstring_){}
	void on_notify(std::string lstring_) {
		state = !state;
		lstring = lstring_;
	}
};

class Subject {
	// maybe weak_ptr here? maybe no vector?
	std::forward_list<Observer*> observers;
public:
	void add_observer(Observer* observer) {
		observers.push_front(observer);
	}
	void remove_observer(Observer* observer) {
		observers.remove(observer);
	}
	void notify(std::string lstring) {
		for (auto& observer : observers) {
			observer->on_notify(lstring);
		}
	}
};
}
