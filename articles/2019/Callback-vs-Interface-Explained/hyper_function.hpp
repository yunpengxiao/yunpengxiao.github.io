//
//  hyper_function.hpp
//  libkinda_objc
//
//  Created by Ling Guo on 2019/1/31.
//

#ifndef hyper_function_h
#define hyper_function_h

#include <type_traits>
#include <utility>

// hyper_function: a std::function-like util, with the feature of accepting inheritance function
// Example: hyper_function<void(Base*)> callback = hyper_function<void(Child*)>();

template<typename...> struct __convertTuple;

template<> struct __convertTuple<> {
	static const bool value = true;
};

template<typename T, typename... Args> struct __convertTuple<T, Args...> {
	static const bool value = T::value && __convertTuple<Args...>::value;
};

template<typename R, typename... Args>
struct hyper_function_state_base {
	virtual R call(Args...) const = 0;
	virtual hyper_function_state_base *clone() const = 0;
	virtual ~hyper_function_state_base() = default;
};

template<typename T, typename R, typename... Args>
struct hyper_function_state : hyper_function_state_base<R, Args...> {
	using Self = hyper_function_state<T, R, Args...>;
	T t_;
	explicit hyper_function_state(T t) : t_(std::move(t)) {
	}
	R call(Args... args) const override {
		return const_cast<T&>(t_)(static_cast<Args&&>(args)...);
	}
	hyper_function_state_base<R, Args...> *clone() const override {
		return new Self(*this);
	}
};

template<typename Sig> class hyper_function;

template<typename R, typename... Args>
class hyper_function<R(Args...)>
{
	typedef hyper_function_state_base<R, Args...> BaseType;
public:
	BaseType *ptr_ = nullptr;
	
	hyper_function() = default;
	
	template<typename Callable, typename = decltype(R(std::declval<typename std::decay<Callable>::type>()(std::declval<Args>()...)))>
	hyper_function(Callable&& t) :
	ptr_(new hyper_function_state<typename std::decay<Callable>::type, R, Args...>(static_cast<Callable&&>(t)))
	{}
	
	// accepting inheritance function
	template<typename... ThatArgs, typename = typename std::enable_if_t<__convertTuple<std::is_convertible<ThatArgs, Args>...>::value>>
	hyper_function(const hyper_function<R(ThatArgs...)>& rhs) noexcept {
		ptr_ = reinterpret_cast<BaseType*>(rhs.ptr_ ? rhs.ptr_->clone() : nullptr);
	}
	
	~hyper_function() {
		if (ptr_) {
			delete ptr_;
			ptr_ = nullptr;
		}
	}
	
	hyper_function(hyper_function& rhs) : ptr_(rhs.ptr_ ? rhs.ptr_->clone() : nullptr) {}
	hyper_function(const hyper_function& rhs) : ptr_(rhs.ptr_ ? rhs.ptr_->clone() : nullptr) {}
	hyper_function(hyper_function&& rhs) noexcept : ptr_(rhs.ptr_) { rhs.ptr_ = nullptr; }
	hyper_function(const hyper_function&& rhs) = delete;
	
	void operator=(const hyper_function& rhs) noexcept {
		if (ptr_) {
			delete ptr_;
			ptr_ = nullptr;
		}
		ptr_ = reinterpret_cast<BaseType*>(rhs.ptr_ ? rhs.ptr_->clone() : nullptr);
		//std::swap(ptr_, rhs.ptr_);
	}
	
	// accepting inheritance function
	template<typename... ThatArgs, typename = typename std::enable_if_t<__convertTuple<std::is_convertible<ThatArgs, Args>...>::value>>
	void operator=(const hyper_function<R(ThatArgs...)>& rhs) noexcept {
		if (ptr_) {
			delete ptr_;
			ptr_ = nullptr;
		}
		ptr_ = reinterpret_cast<BaseType*>(rhs.ptr_ ? rhs.ptr_->clone() : nullptr);
		//std::swap(ptr_, rhs.ptr_);
	}
	
	R operator()(Args... args) const {
		return ptr_->call(static_cast<Args&&>(args)...);
	}
	
	explicit operator bool() const noexcept {
		return ptr_ != nullptr;
	}
};

#endif /* hyper_function_h */
