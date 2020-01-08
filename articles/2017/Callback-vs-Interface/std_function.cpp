// Shahar Mike, 2016
// https://shaharmike.com/cpp/naive-std-function/
//
// Adapted by BOT Man, 2017

#include <memory>
#include <utility>

template <typename T>
class naive_function;

template <typename Ret, typename... Args>
class naive_function<Ret(Args...)> {
 public:
  template <typename T>
  naive_function& operator=(T&& t) {
    callable_ = std::make_unique<CallableT<T>>(std::forward<T>(t));
    return *this;
  }

  Ret operator()(Args... args) const {
    if (callable_)
      return callable_->Invoke(args...);
  }

 private:
  class ICallable {
   public:
    virtual ~ICallable() = default;
    virtual Ret Invoke(Args...) = 0;
  };

  template <typename T>
  class CallableT : public ICallable {
   public:
    CallableT(const T& t) : t_(t) {}

    Ret Invoke(Args... args) override { return t_(args...); }

   private:
    T t_;
  };

  std::unique_ptr<ICallable> callable_;
};

#include <functional>
#include <iostream>

void func(int& i) {
  std::cout << ++i << " func" << std::endl;
}

struct container {
  void mem_func(int& i) { std::cout << ++i << " mem_func" << std::endl; }
};

struct functor {
  void operator()(int& i) { std::cout << ++i << " functor" << std::endl; }
};

int main() {
  naive_function<void(int&)> f;
  int i = 0;

  f = func;
  f(i);
  f = std::bind(&container::mem_func, new container, std::placeholders::_1);
  f(i);
  f = functor{};
  f(i);
  f = [&i](int) { std::cout << ++i << " lambda " << std::endl; };
  f(i);

  return 0;
}
