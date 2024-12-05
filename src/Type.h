#pragma once
#include <cstdlib>
#ifndef PYTHON_INTERPRETER_TYPE_H
#define PYTHON_INTERPRETER_TYPE_H

#include <any>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cmath>
#include <vector>
#include <map>
#include "int2048.h"

#undef DEBUG_

#ifdef DEBUG_
using int2048 = long long;
#else
using int2048 = sjtu::int2048;
#endif

struct rValue {
  enum class Type {None, Int, Float, Str, Bool} typ;
  std::any val;
  explicit rValue() : typ(Type::None) {
    ;
  };
  explicit rValue(int2048 x) : typ(Type::Int), val(x) {
    ;
  };
  explicit rValue(double x) : typ(Type::Float), val(x) {
    ;
  };
  explicit rValue(std::string x) : typ(Type::Str), val(x) {
    ;
  };
  explicit rValue(bool x) : typ(Type::Bool), val(x) {
    ;
  };
  explicit operator bool() const {
    switch (typ) {
      case Type::None : {
        return false;
        break;
      }
      case Type::Int : {
        return std::any_cast<int2048>(val) != 0;
        break;
      }
      case Type::Float : {
        return std::any_cast<double>(val) != 0;
        break;
      }
      case Type::Str : {
        return std::any_cast<std::string>(val) != "";
        break;
      }
      case Type::Bool : {
        return std::any_cast<bool>(val);
        break;
      }
    }
    throw;
  };
  explicit operator std::string() const { // WARNING: CHANGE output() ALSO.
    std::stringstream ss;
    switch (typ) {
      case Type::None : {
        return "None";
        break;
      }
      case Type::Int : {
        ss << std::any_cast<int2048>(val);
        break;
      }
      case Type::Float : {
        ss << std::fixed << std::setprecision(6) << std::any_cast<double>(val);
        break;
      }
      case Type::Str : {
        return std::any_cast<std::string>(val);
        break;
      }
      case Type::Bool : {
        return std::any_cast<bool>(val) ? "True" : "False";
        break;
      }
    }
    return ss.str();
  };
  explicit operator int2048() const {
    switch (typ) {
      case Type::None : { //UNDEF
        return 0;
        break;
      }
      case Type::Int : {
        return std::any_cast<int2048>(val);
        break;
      }
      case Type::Float : {
        std::stringstream ss;
        int2048 ret;
        ss << std::fixed << std::setprecision(0) << std::trunc(std::any_cast<double>(val));
        ss >> ret;
        return ret;
        break;
      }
      case Type::Str : {
        std::stringstream ss(std::any_cast<std::string>(val));
        int2048 ret;
        ss >> ret;
        return ret;
        break;
      }
      case Type::Bool : {
        return std::any_cast<bool>(val) ? 1 : 0;
        break;
      }
    }
    throw;
  };
  explicit operator double() const {
    switch (typ) {
      case Type::None : { // UNDEF
        return 0;
        break;
      }
      case Type::Int : {
        std::stringstream ss;
        double d;
        ss << std::any_cast<int2048>(val);
        ss >> d;
        return d;
        break;
      }
      case Type::Float : {
        return std::any_cast<double>(val);
        break;
      }
      case Type::Str : {
        std::stringstream ss(std::any_cast<std::string>(val));
        double d;
        ss >> d;
        return d;
        break;
      }
      case Type::Bool : {
        return std::any_cast<bool>(val) ? 1.0 : 0.0;
        break;
      }
    }
    throw;
  };
  void output() const { // WARNING: CHANGE operator std::string() ALSO.
    switch (typ) {
      case Type::None : {
        std::cout << "None";
        break;
      }
      case Type::Int : {
        std::cout << std::any_cast<int2048>(val);
        break;
      }
      case Type::Float : {
        std::cout << std::fixed << std::setprecision(6) << std::any_cast<double>(val);
        break;
      }
      case Type::Str : {
        std::cout << std::any_cast<std::string>(val);
        break;
      }
      case Type::Bool : {
        std::cout << (std::any_cast<bool>(val) ? "True" : "False");
        break;
      }
    }
  };
  void up(void) {
    if (typ == Type::Bool) {
      typ = Type::Int;
      val = int2048(static_cast<long long>(std::any_cast<bool>(val)));
    }
  };
  rValue positate() {
    up();
    return *this;
  };
  rValue negate() {
    return rValue(int2048(0)).minus(*this);
  };
  rValue add(rValue x) {
    up();
    x.up();
    if (typ == Type::Int && x.typ == Type::Int) { // end int
      return rValue(std::any_cast<int2048>(val) +
          std::any_cast<int2048>(x.val));
    } else if (typ == Type::Float && x.typ == Type::Float) {
      return rValue(std::any_cast<double>(val) +
          std::any_cast<double>(x.val));
    } else if (typ == Type::Float && x.typ == Type::Int) {
      return rValue(std::any_cast<double>(val) + 
          static_cast<double>(std::any_cast<int2048>(x.val)));
    } else if (typ == Type::Int && x.typ == Type::Float) { // end float
      return rValue(static_cast<double>(std::any_cast<int2048>(val)) +
          std::any_cast<double>(x.val));
    } else if (typ == Type::Str && x.typ == Type::Str) { // end str
      return rValue(std::any_cast<std::string>(val) +
          std::any_cast<std::string>(x.val));
    } else {
      throw;
    }
  };
  rValue minus(rValue x) {
    up();
    x.up();
    if (typ == Type::Int && x.typ == Type::Int) { // end int
      return rValue(std::any_cast<int2048>(val) -
          std::any_cast<int2048>(x.val));
    } else if (typ == Type::Float && x.typ == Type::Float) {
      return rValue(std::any_cast<double>(val) -
          std::any_cast<double>(x.val));
    } else if (typ == Type::Float && x.typ == Type::Int) {
      return rValue(std::any_cast<double>(val) - 
          static_cast<double>(std::any_cast<int2048>(x.val)));
    } else if (typ == Type::Int && x.typ == Type::Float) { // end float
      return rValue(static_cast<double>(std::any_cast<int2048>(val)) - 
          std::any_cast<double>(x.val));
    } else {
      throw;
    }
  };
  rValue times(rValue x) {
    up();
    x.up();
    if (typ == Type::Int && x.typ == Type::Int) { // end int
      return rValue(std::any_cast<int2048>(val) *
          std::any_cast<int2048>(x.val));
    } else if (typ == Type::Float && x.typ == Type::Float) {
      return rValue(std::any_cast<double>(val) *
          std::any_cast<double>(x.val));
    } else if (typ == Type::Float && x.typ == Type::Int) {
      return rValue(std::any_cast<double>(val) * 
          static_cast<double>(std::any_cast<int2048>(x.val)));
    } else if (typ == Type::Int && x.typ == Type::Float) { // end float
      return rValue(static_cast<double>(std::any_cast<int2048>(val)) *
          std::any_cast<double>(x.val));
    } else if (typ == Type::Str && x.typ == Type::Int) { // end str
      std::string ret(""), app(std::any_cast<std::string>(val));
      int2048 times(std::any_cast<int2048>(x.val));
      for (int i = 1; i <= times; i++) {
        ret.append(app);
      }
      return rValue(ret);
    } else {
      throw;
    }
  };
  rValue div(rValue x) {
    up();
    x.up();
    if (typ == Type::Int && x.typ == Type::Int) { // end int
      return rValue(static_cast<double>(std::any_cast<int2048>(val)) /
          static_cast<double>(std::any_cast<int2048>(x.val)));
    } else if (typ == Type::Float && x.typ == Type::Float) {
      return rValue(std::any_cast<double>(val) /
          std::any_cast<double>(x.val));
    } else if (typ == Type::Float && x.typ == Type::Int) {
      return rValue(std::any_cast<double>(val) / 
          static_cast<double>(std::any_cast<int2048>(x.val)));
    } else if (typ == Type::Int && x.typ == Type::Float) { // end float
      return rValue(static_cast<double>(std::any_cast<int2048>(val)) /
          std::any_cast<double>(x.val));
    } else {
      throw;
    }
  };
  rValue idiv(rValue x) {
    up();
    x.up();
    if (typ == Type::Int && x.typ == Type::Int) { // end int
      return rValue(std::any_cast<int2048>(val) /
          std::any_cast<int2048>(x.val));
    } else if (typ == Type::Float && x.typ == Type::Float) {
      return rValue(std::floor(std::any_cast<double>(val) /
          std::any_cast<double>(x.val)));
    } else if (typ == Type::Float && x.typ == Type::Int) {
      return rValue(std::floor(std::any_cast<double>(val) / 
          static_cast<double>(std::any_cast<int2048>(x.val))));
    } else if (typ == Type::Int && x.typ == Type::Float) { // end float
      return rValue(std::floor(static_cast<double>(std::any_cast<int2048>(val)) / 
          std::any_cast<double>(x.val)));
    } else {
      throw;
    }
  };
  rValue mod(rValue x) {
    return this->minus(this->idiv(x).times(x));
  };
  rValue eq(rValue x) {
    up();
    x.up();
    if (typ == Type::Int && x.typ == Type::Int) { // end int
      return rValue(std::any_cast<int2048>(val) ==
          std::any_cast<int2048>(x.val));
    } else if (typ == Type::Float && x.typ == Type::Float) {
      return rValue(std::any_cast<double>(val) ==
          std::any_cast<double>(x.val));
    } else if (typ == Type::Float && x.typ == Type::Int) {
      return rValue(std::any_cast<double>(val) == 
          static_cast<double>(std::any_cast<int2048>(x.val)));
    } else if (typ == Type::Int && x.typ == Type::Float) { // end float
      return rValue(static_cast<double>(std::any_cast<int2048>(val)) == 
          std::any_cast<double>(x.val));
    } else if (typ == Type::Str && x.typ == Type::Str) { // end str
      return rValue(std::any_cast<std::string>(val) ==
          std::any_cast<std::string>(x.val));
    } else if (typ == Type::None && x.typ == Type::None) {
      return rValue(true);
    } else {
      return rValue(false);
    }
  };
  rValue neq(rValue x) {
    return rValue(!std::any_cast<bool>(this->eq(x).val));
  };
  rValue less(rValue x) {
    up();
    x.up();
    if (typ == Type::Int && x.typ == Type::Int) { // end int
      return rValue(std::any_cast<int2048>(val) <
          std::any_cast<int2048>(x.val));
    } else if (typ == Type::Float && x.typ == Type::Float) {
      return rValue(std::any_cast<double>(val) <
          std::any_cast<double>(x.val));
    } else if (typ == Type::Float && x.typ == Type::Int) {
      return rValue(std::any_cast<double>(val) <
          static_cast<double>(std::any_cast<int2048>(x.val)));
    } else if (typ == Type::Int && x.typ == Type::Float) { // end float
      return rValue(static_cast<double>(std::any_cast<int2048>(val)) <
          std::any_cast<double>(x.val));
    } else if (typ == Type::Str && x.typ == Type::Str) { // end str
      return rValue(std::any_cast<std::string>(val) <
          std::any_cast<std::string>(x.val));
    } else {
      return rValue(false);
    }
  };
  rValue greater(rValue x) {
    return rValue(std::any_cast<bool>(x.less(*this).val));
  };
  rValue leq(rValue x) {
    return rValue(!std::any_cast<bool>(x.less(*this).val));
  };
  rValue geq(rValue x) {
    return rValue(!std::any_cast<bool>(this->less(x).val));
  };
  rValue not_(void) {
    return rValue(!bool(*this));
  };
  rValue and_(rValue x) {
    return rValue(bool(*this) && bool(x));
  };
  rValue or_(rValue x) {
    return rValue(bool(*this) || bool(x));
  };
  typedef rValue (rValue::*MemberFunc)(rValue);
};

struct lValue {
  std::map<std::string, rValue>::iterator it;
  lValue(std::map<std::string, rValue>::iterator it_) : it(it_) {
    ;
  };
  explicit operator rValue() {
    return it->second;
  };
  typedef void (lValue::*MemberFunc)(rValue) const;
  void assign(rValue x) const {
    it->second = x;
  };
  void add_assign(rValue x) const {
    it->second = it->second.add(x);
  };
  void minus_assign(rValue x) const {
    it->second = it->second.minus(x);
  };
  void times_assign(rValue x) const {
    it->second = it->second.times(x);
  };
  void div_assign(rValue x) const {
    it->second = it->second.div(x);
  };
  void idiv_assign(rValue x) const {
    it->second = it->second.idiv(x);
  };
  void mod_assign(rValue x) const {
    it->second = it->second.mod(x);
  }
};

struct Value {
  std::any val;
  Value() : val(rValue()) {
    ;
  };
  Value(rValue x) : val(x) {
    ;
  };
  Value(lValue x) : val(x) {
    ;
  };
  operator rValue() {
    if (auto ptr = std::any_cast<rValue>(&val)) {
      return *ptr;
    } else {
      return rValue(std::any_cast<lValue>(val));
    }
  };
  operator lValue() {
    return std::any_cast<lValue>(val);
  };
  void swap(Value t) {
    val.swap(t.val);
  };
};

typedef std::vector<Value> Tuple;

#endif