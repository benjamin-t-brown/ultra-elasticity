#pragma once

#include "lib/sdl2w/Logger.h"
#include "utils/Timer.hpp"
#include <memory>
#ifdef __GNUG__
#include <cxxabi.h>
#if !defined(_WIN32)
#include <cstdlib>
#endif
#endif

namespace program {

struct State;

namespace actions {
class AbstractAction {
protected:
  long id = 0;
  State* state = nullptr;

  virtual void act() {
    sdl2w::Logger().get(sdl2w::WARN) << "AbstractAction::act() called noop";
  };

public:
  static long idCounter;
  AbstractAction() { id = idCounter++; }
  long getId() const { return id; }
  virtual std::string getName() const {
#if defined(__GNUG__)
    int status;
    char* realname = abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
    std::string name = (status == 0) ? realname : typeid(*this).name();
    std::free(realname);
    return name;
#else
    return typeid(*this).name();
#endif
  }

  // helper shorthand for insertAction 
  void insAct(AbstractAction* action, int ms);
  // helper shorthand for addParallelAction
  void pllAct(AbstractAction* action, int ms);

  void setState(State* state) { this->state = state; }

  void execute(State* state) {
    this->state = state;
    // LOG(INFO) << "Executing action: " << getName() << LOG_ENDL;
    act();
  }

  virtual ~AbstractAction() = default;
};

struct AsyncAction {
  std::unique_ptr<actions::AbstractAction> action;
  Timer timer;
  long insertAfterId = 0;
};

} // namespace actions

} // namespace program