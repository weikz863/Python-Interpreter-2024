#pragma once

#include "Type.h"
#include <any>
#include <cstddef>
#include <string>
#include <utility>
#ifndef PYTHON_INTERPRETER_SCOPE_H
#define PYTHON_INTERPRETER_SCOPE_H

#include "Function.h"
#include "Python3ParserBaseVisitor.h"
#include <list>
#include <map>

struct StackHandler {
  std::list<std::map<std::string, rValue>> stk;
  FunctionHandler funs;

  StackHandler() : stk(1) {
    ;
  };
  lValue operator[](const std::string &name) {
    auto it = stk.begin();
    if (auto ret = it->find(name); ret != it->end()) {
      return ret;
    }
    it = --stk.end();
    if (auto ret = it->find(name); ret != it->end()) {
      return ret;
    }
    return stk.front().emplace(name, rValue()).first;
  };
  void deffunc(const std::string &name, const Function &f) {
    funs.insert(std::make_pair(name, f));
  };
  std::any call(const std::string &name, const FuncArgs &args, Python3ParserBaseVisitor *v) {
    if (name == "print") {
      if (args.empty()) {
        std::cout << '\n';
        return Value(rValue());
      }
      args[0].second.output();
      for (size_t i = 1; i < args.size(); i++) {
        std::cout << ' ';
        args[i].second.output();
      }
      std::cout << '\n';
      return Value(rValue());
    } else if (name == "int"){
      return Value(rValue(int2048(args[0].second)));
    } else if (name == "float") {
      return Value(rValue(double(args[0].second)));
    } else if (name == "str") {
      return Value(rValue(std::string(args[0].second)));
    } else if (name == "bool") {
      return Value(rValue(bool(args[0].second)));
    } else {
      Function f = funs.find(name)->second;
      for (size_t i = 0; i < args.size(); i++) {
        if (args[i].first == "") {
          f.parameters[i].second = args[i].second;
        } else {
          f.parameters[f.name2place[args[i].first]].second = args[i].second;
        }
      }
      stk.emplace_front();
      for (auto p : f.parameters) {
        stk.front().insert(p);
      }
      auto r = std::any_cast<FlowControl>(v->visitSuite(f.body));
      if (r.typ == FlowControl::Type::RETURN) {
        if (auto v = std::any_cast<Value>(&r.ret)) {
          auto t = Value(rValue(*v));
          stk.pop_front();
          return t;
        } else { // Tuple
          Tuple t;
          auto lst = std::any_cast<Tuple>(r.ret);
          for (auto v : lst) {
            t.emplace_back(rValue(v));
          }
          stk.pop_front();
          return t;
        }
      } else {
        stk.pop_front();
        return Value(rValue());
      }
    }
  };
};

#endif