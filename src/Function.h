#pragma once
#include <string>
#ifndef PYTHON_INTERPRETER_FUNCTION_H
#define PYTHON_INTERPRETER_FUNCTION_H

#include "Type.h"
#include "Python3Parser.h"
#include <map>
#include <vector>

struct FlowControl {
  enum class Type{REGULAR, RETURN, BREAK, CONTINUE} typ;
  std::any ret;
  FlowControl() : typ(Type::REGULAR), ret(Value(rValue())) {
    ;
  };
  FlowControl(FlowControl::Type typ_) : typ(typ_), ret(Value(rValue())) {
    ;
  };
  FlowControl(Value val) : typ(Type::RETURN), ret(val) {
    ;
  };
};

typedef std::vector<std::pair<std::string, rValue>> FunctionParams;
typedef std::pair<std::string, rValue> FunctionArgument;
typedef std::vector<FunctionArgument> FuncArgs;

struct Function {
  FunctionParams parameters;
  Python3Parser::SuiteContext *body;
  std::map<std::string, size_t> name2place;
  Function (const FunctionParams &params_, Python3Parser::SuiteContext *body_) :
      parameters(params_), body(body_) {
    for (size_t i = 0; i < params_.size(); i++) {
      name2place[params_[i].first] = i;
    }
  };
};

typedef std::map<std::string, Function> FunctionHandler;

#endif