#pragma once

#include <deque>

#include "error/error.h"

namespace util {

/// Reader with configurable lookahead.
///
/// The CallBack provides the source of the values,
/// and the LookaheadStack will call it whenever it needs a new value.
/// The type of the callback must be ErrorOr<Value, Err>().
template <unsigned int lookahead, typename Value, typename Err>
class LookaheadStack {
 public:
  using CallBack = std::function<ErrorOr<Value, Err>()>;

  /// Construct the stack with a callback.
  ///
  /// It is typical to construct it with a functor, to keep the state. To
  /// construct it from a member function, use:
  /// std::bind(&MyClass::my_method, &my_class_instance).
  explicit LookaheadStack(CallBack callback) : callback_(std::move(callback)) {}

  /// The stack is only empty when it hasn't seen any token yet, in which case
  /// get_next() must be called once.
  bool empty() const { return token_stack_.empty(); }

  /// Update the internal state to point to the next value.
  ///
  /// If required, it will call CallBack once to get the next value,
  /// potentially propagating the error.
  MaybeError<Err> get_next() {
    if (token_stack_.size() > lookahead) {
      token_stack_.pop_front();
    }
    if (backlog_ == 0) {
      RETURN_OR_MOVE(Value t, callback_());
      token_stack_.emplace_back(std::move(t));
    } else {
      --backlog_;
    }
    return {};
  }

  /// Update the internal state to point to the previous value.
  ///
  /// Only lookahead number of values are kept, so unget will fail with an
  /// assert if asked to point to a value no longer stored.
  void unget() {
    ++backlog_;
    assert(backlog_ <= lookahead &&
           "Too much token backlog; increase k_lookahead?");
  }

  /// Return value currently pointed.
  ///
  /// The value may be invalid if the stack is empty.
  const Value& current() const {
    assert(token_stack_.size() > backlog_ &&
           "Current value is invalid: stack is empty or unget was called more "
           "than get_next");
    return token_stack_[token_stack_.size() - backlog_ - 1];
  }

 private:
  // Function to read a new token.
  CallBack callback_;
  // How many unget_token() levels we are at.
  unsigned int backlog_ = 0;
  std::deque<Value> token_stack_;
};

}  // namespace util
