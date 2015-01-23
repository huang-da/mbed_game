#pragma once

#include "std.h"

template <class T, nat sSize>
class SQueue {
public:
	SQueue() : sz(0), head(0), tail(0) {}

	void push_front(T i) {
		items[head++] = i;
		if (head == sSize)
			head = 0;
		sz++;
	}

	void clear() {
		head = tail = sz = 0;
	}

	T pop_back() {
		sz--;
		nat old = tail;
		if (++tail == sSize)
			tail = 0;
		return items[old];
	}

	T back() {
		return items[tail];
	}

	bool empty() const {
		return sz == 0;
	}

	nat size() const {
		return sz;
	}

	T at(nat i) const {
		return items[(tail + i) % sSize];
	}

private:
	nat sz;
	nat head;
	nat tail;
	T items[sSize];
};
