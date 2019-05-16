#pragma once
template<typename T>
class SimpleList {
public:
	T* arr;
	unsigned int capacity;
	unsigned int count;
	unsigned int growAmount;

	SimpleList() = delete;
	SimpleList(const SimpleList<T>&) = delete;
	SimpleList(unsigned int capacity, unsigned int growAmount, bool lazyInit = false) 
		: capacity(capacity), arr((lazyInit) ? nullptr : new T[capacity]), count(0), growAmount(growAmount) {

	}
	~SimpleList() {
		delete[] arr;
	}

	unsigned int add(T elem) {
		if (arr == nullptr) { // Lazy initialize
			arr = new T[capacity];
		}
		if (count == capacity) {
			growArray();
		}
		int index = count++;
		arr[index] = elem;
		return index;
	}

	void remove(unsigned int index) {
		for (unsigned int i = index + 1; i < count; i++) {
			arr[i - 1] = arr[i];
		}
		count--;
	}

	void clear() {
		count = 0;
	}

	inline T& operator [](unsigned int index) {
		return arr[index];
	}

private:

	void growArray() {
		capacity += growAmount;
		T* nArr = new T[capacity];
		if (nArr == nullptr) {
			return;
		}
		unsigned int index = 0;
		for (; index < count; index++) {
			nArr[index] = arr[index];
		}
		delete[] arr;
		arr = nArr;
	}
};