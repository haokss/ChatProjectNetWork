#pragma once
#include <memory>
template<typename T>
class Singleton {

public:
	static T& GetInstance() {
		static T instance;
		return instance;
	}
protected:
	Singleton() = default;
	virtual ~Singleton(){}
	Singleton(const Singleton<T>&) = delete;
	Singleton<T>& operator=(const Singleton<T>&) = delete;
};