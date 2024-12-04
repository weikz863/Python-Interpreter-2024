#pragma once
#include "Function.h"
#include "Python3ParserVisitor.h"
#include "Type.h"
#include <any>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H

#include "Python3ParserBaseVisitor.h"
#include "Scope.h"

extern StackHandler stk;

class EvalVisitor : public Python3ParserBaseVisitor {
	// TODO: override all methods of Python3ParserBaseVisitor
  std::any visitFile_input(Python3Parser::File_inputContext *ctx) override {
    auto stmts = ctx->stmt();
    for (auto ptr : stmts) {
      visitStmt(ptr);
    }
    return FlowControl();
  }

  std::any visitFuncdef(Python3Parser::FuncdefContext *ctx) override {
    stk.deffunc(ctx->NAME()->getText(), Function(
      std::any_cast<FunctionParams>(visitParameters(ctx->parameters())), ctx->suite()));
    return FlowControl();
  }

  std::any visitParameters(Python3Parser::ParametersContext *ctx) override {
    if (auto t = ctx->typedargslist()) {
      return visitTypedargslist(t);
    } else {
      return FunctionParams();
    }
  }

  std::any visitTypedargslist(Python3Parser::TypedargslistContext *ctx) override {
    FunctionParams ret;
    auto names = ctx->tfpdef();
    auto inits = ctx->test();
    std::vector<rValue> values(inits.size());
    for (size_t i = 0; i < inits.size(); i++) {
      values[i] = std::any_cast<Value>(visitTest(inits[i]));
    }
    for (size_t i = 0; i < names.size() - inits.size(); i++) {
      ret.emplace_back(std::any_cast<std::string>(visitTfpdef(names[i])), rValue());
    }
    for (size_t i = names.size() - inits.size(); i < names.size(); i++) {
      ret.emplace_back(std::any_cast<std::string>(visitTfpdef(names[i])), 
          values[i - (names.size() - inits.size())]);
    }
    return ret;
  }

  std::any visitTfpdef(Python3Parser::TfpdefContext *ctx) override {
    return ctx->NAME()->getText();
  }

  std::any visitStmt(Python3Parser::StmtContext *ctx) override {
    if (auto t = ctx->simple_stmt()) {
      return visitSimple_stmt(t);
    } else {
      return visitCompound_stmt(ctx->compound_stmt());
    }
  }

  std::any visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) override {
    return visitSmall_stmt(ctx->small_stmt());
  }

  std::any visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) override {
    if (auto t = ctx->expr_stmt()) {
      return visitExpr_stmt(t);
    } else {
      return visitFlow_stmt(ctx->flow_stmt());
    }
  }

  std::any visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) override {
    if (auto t =ctx->augassign()) {
      auto rhs = std::any_cast<Value>(visitTestlist(ctx->testlist(1)));
      (lValue(std::any_cast<Value>(visitTestlist(ctx->testlist(0)))).*
        std::any_cast<lValue::MemberFunc>(visitAugassign(t)))(rhs);
        // call the member function returned by visitAugassign with parameter rhs for lhs.
    } else {
      auto testlists = ctx->testlist();
      auto ret = visitTestlist(testlists.back());
      if (auto ptr = std::any_cast<Value>(&ret)) {
        Value rhs = *ptr;
        Value lhs;
        for (size_t i = testlists.size() - 2; i != 0xfffffffffffffffflu; i--, lhs.swap(rhs)) {
          lhs = std::any_cast<Value>(visitTestlist(testlists[i]));
          lValue(lhs).assign(rhs);
        }
      } else {
        Tuple rhs = std::any_cast<Tuple>(ret);
        Tuple lhs;
        for (size_t i = testlists.size() - 2; i != 0xfffffffffffffffflu; i--, lhs.swap(rhs)) {
          lhs = std::any_cast<Tuple>(visitTestlist(testlists[i]));
          for (size_t j = 0; j < lhs.size(); j++) {
            lValue(lhs[j]).assign(rhs[j]);
          }
        }
      }
    }
    return FlowControl();
  }

  std::any visitAugassign(Python3Parser::AugassignContext *ctx) override {
    if (ctx->ADD_ASSIGN()) {
      return &lValue::add_assign;
    } else if (ctx->SUB_ASSIGN()) {
      return &lValue::minus_assign;
    } else if (ctx->MULT_ASSIGN()) {
      return &lValue::times_assign;
    } else if (ctx->DIV_ASSIGN()) {
      return &lValue::div_assign;
    } else if (ctx->IDIV_ASSIGN()) {
      return &lValue::idiv_assign;
    } else if (ctx->MOD_ASSIGN()) {
      return &lValue::mod_assign;
    } else { // ERROR
      throw;
    }
  }

  std::any visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) override {
    if (auto t = ctx->return_stmt()) {
      return visitReturn_stmt(t);
    } else if (auto t = ctx->break_stmt()) {
      return visitBreak_stmt(t);
    } else if (auto t = ctx->continue_stmt()) {
      return visitContinue_stmt(t);
    } else { // ERROR
      throw;
    }
  }

  std::any visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) override {
    return FlowControl(FlowControl::Type::BREAK);
  }

  std::any visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) override {
    return FlowControl(FlowControl::Type::CONTINUE);
  }

  std::any visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) override {
    if (auto t = ctx->testlist()) {
      auto ret = visitTestlist(t);
      if (auto ptr = std::any_cast<Value>(&ret)) {
        return FlowControl(*ptr);
      } else {
        return FlowControl(std::any_cast<Tuple>(ret));
      }
    } else {
      return FlowControl(FlowControl::Type::RETURN);
    }
  }

  std::any visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) override {
    if (auto t = ctx->if_stmt()) {
      return visitIf_stmt(t);
    } else if (auto t = ctx->while_stmt()) {
      return visitWhile_stmt(t);
    } else if (auto t = ctx->funcdef()) {
      return visitFuncdef(t);
    } else { // ERROR
      throw;
    }
  }

  std::any visitIf_stmt(Python3Parser::If_stmtContext *ctx) override {
    auto tests = ctx->test();
    auto suites = ctx->suite();
    for (size_t i = 0; i < tests.size(); i++) {
      if (rValue(std::any_cast<Value>(visitTest(tests[i])))) {
        return visitSuite(suites[i]);
      }
    }
    if (ctx->ELSE()) {
      return visitSuite(suites.back());
    } else {
      return FlowControl();
    }
  }

  std::any visitWhile_stmt(Python3Parser::While_stmtContext *ctx) override {
    auto test = ctx->test();
    auto suite = ctx->suite();
    while (rValue(std::any_cast<Value>(visitTest(test)))) {
      auto ret = std::any_cast<FlowControl>(visitSuite(suite));
      switch (ret.typ) {
        case FlowControl::Type::BREAK : {
          return FlowControl();
        }
        case FlowControl::Type::RETURN : {
          return ret;
        }
        default : {
          ;
        }
      }
    }
    return FlowControl();
  }

  std::any visitSuite(Python3Parser::SuiteContext *ctx) override {
    if (auto t = ctx->simple_stmt()) { // UNSUPPORTED BY PYTHON 
      return visitSimple_stmt(t);
    } else {
      auto stmts = ctx->stmt();
      FlowControl ret;
      for (auto stmt : stmts) {
        ret = std::any_cast<FlowControl>(visitStmt(stmt));
        if (ret.typ != FlowControl::Type::REGULAR) return ret;
      }
      return FlowControl();
    }
  }

  std::any visitTest(Python3Parser::TestContext *ctx) override {
    return visitOr_test(ctx->or_test());
  }

  std::any visitOr_test(Python3Parser::Or_testContext *ctx) override {
    auto ands = ctx->and_test();
    if (ands.size() == 1) {
      return visitAnd_test(ands[0]);
    } else {
      for (auto a : ands) {
        if (rValue(std::any_cast<Value>(visitAnd_test(a)))) {
          return Value(rValue(true));
        }
      }
      return Value(rValue(false));
    }
  }

  std::any visitAnd_test(Python3Parser::And_testContext *ctx) override {
    auto nots = ctx->not_test();
    if (nots.size() == 1) {
      return visitNot_test(nots[0]);
    } else {
      for (auto n : nots) {
        if (!rValue(std::any_cast<Value>(visitNot_test(n)))) {
          return Value(rValue(false));
        }
      }
      return Value(rValue(true));
    }
  }

  std::any visitNot_test(Python3Parser::Not_testContext *ctx) override {
    if (auto t = ctx->not_test()) {
      return Value(rValue(!rValue(std::any_cast<Value>(visitNot_test(t)))));
    } else {
      return visitComparison(ctx->comparison());
    }
  }

  std::any visitComparison(Python3Parser::ComparisonContext *ctx) override {
    auto op = ctx->comp_op();
    auto ar = ctx->arith_expr();
    if (op.empty()) {
      return visitArith_expr(ar[0]);
    } else {
      rValue val = std::any_cast<Value>(visitArith_expr(ar[0]));
      rValue t;
      for (size_t i = 1; i < ar.size(); i++) {
        if (t = std::any_cast<Value>(visitArith_expr(ar[i])); 
            (rValue(val).*std::any_cast<rValue::MemberFunc>(visitComp_op(op[i - 1])))(t)) {
          val = t;
        } else {
          return Value(rValue(false));
        }
      }
      return Value(rValue(true));
    }
  }

  std::any visitComp_op(Python3Parser::Comp_opContext *ctx) override {
    if (ctx->EQUALS()) {
      return &rValue::eq;
    } else if (ctx->NOT_EQ_2()) {
      return &rValue::neq;
    } else if (ctx->LESS_THAN()) {
      return &rValue::less;
    } else if (ctx->LT_EQ()) {
      return &rValue::leq;
    } else if (ctx->GREATER_THAN()) {
      return &rValue::greater;
    } else if (ctx->GT_EQ()) {
      return &rValue::geq;
    } else { // ERROR
      throw;
    }
  }

  std::any visitArith_expr(Python3Parser::Arith_exprContext *ctx) override {
    auto terms = ctx->term();
    if (terms.size() == 1) {
      return visitTerm(terms[0]);
    }
    auto val = std::any_cast<Value>(visitTerm(terms[0]));
    auto ops = ctx->addorsub_op();
    for (size_t i = 1; i < terms.size(); i++) {
      val = (rValue(val).*std::any_cast<rValue::MemberFunc>(visitAddorsub_op(ops[i - 1])))
          (std::any_cast<Value>(visitTerm(terms[i])));
    }
    return val;
  }

  std::any visitAddorsub_op(Python3Parser::Addorsub_opContext *ctx) override {
    if (ctx->ADD()) {
      return &rValue::add;
    } else {
      return &rValue::minus;
    }
  }

  std::any visitTerm(Python3Parser::TermContext *ctx) override {
    auto factors = ctx->factor();
    if (factors.size() == 1) {
      return visitFactor(factors[0]);
    }
    auto val = std::any_cast<Value>(visitFactor(factors[0]));
    auto ops = ctx->muldivmod_op();
    for (size_t i = 1; i < factors.size(); i++) {
      val = (rValue(val).*std::any_cast<rValue::MemberFunc>(visitMuldivmod_op(ops[i - 1])))
        (std::any_cast<Value>(visitFactor(factors[i])));
    }
    return val;
  }

  std::any visitMuldivmod_op(Python3Parser::Muldivmod_opContext *ctx) override {
    if (ctx->STAR()) {
      return &rValue::times;
    } else if (ctx->DIV()) {
      return &rValue::div;
    } else if (ctx->IDIV()) {
      return &rValue::idiv;
    } else if (ctx->MOD()) {
      return &rValue::mod;
    } else { // ERROR
      throw;
    }
  }

  std::any visitFactor(Python3Parser::FactorContext *ctx) override {
    if (auto t = ctx->atom_expr()) {
      return visitAtom_expr(t);
    } else {
      if (ctx->ADD()) {
        return Value(rValue(std::any_cast<Value>(visitFactor(ctx->factor()))).positate());
      } else {
        return Value(rValue(std::any_cast<Value>(visitFactor(ctx->factor()))).negate());
      }
    }
  }

  std::any visitAtom_expr(Python3Parser::Atom_exprContext *ctx) override {
    if (auto t = ctx->trailer()) {
      return stk.call(std::any_cast<std::string>(visitAtom(ctx->atom())), 
        std::any_cast<FunctionParams>(visitTrailer(t)), this);
    } else {
      auto atm = visitAtom(ctx->atom());
      if (auto t = std::any_cast<std::string>(&atm)) {
        return Value(stk[*t]);
      } else {
        return atm;
      }
    }
  }

  std::any visitTrailer(Python3Parser::TrailerContext *ctx) override {
    if (auto t = ctx->arglist()) {
      return visitArglist(t);
    } else {
      return FunctionParams();
    }
  }

  std::any visitAtom(Python3Parser::AtomContext *ctx) override {
    if (ctx->NAME()) {
      return ctx->NAME()->getText();
    } else if (ctx->NUMBER()) {
      std::stringstream ss(ctx->NUMBER()->getText());
      if (ss.str().find('.') == std::string::npos) {
        int2048 a;
        ss >> a;
        return Value(rValue(a));
      } else {
        double a;
        ss >> a;
        return Value(rValue(a));
      }
    } else if (auto t = ctx->STRING(); !t.empty()) {
      std::string ret, tmp;
      for (auto s : t) {
        tmp = s->getText();
        for (size_t i = 1; i + 1 < tmp.size(); i++){
          if (tmp[i] != '\\') {
            ret.push_back(tmp[i]);
          } else {
            i++;
            switch (tmp[i]) {
              case 't' : {
                ret.push_back('\t');
                break;
              }
              case 'n' : {
                ret.push_back('\n');
                break;
              }
              case '\\' : case '\'' : case '\"' : default: {
                ret.push_back(tmp[i]);
                break;
              }
            }
          }
        }
      }
      return Value(rValue(ret));
    } else if (ctx->NONE()) {
      return Value(rValue());
    } else if (ctx->TRUE()) {
      return Value(rValue(true));
    } else if (ctx->FALSE()) {
      return Value(rValue(false));
    } else if (ctx->test()) {
      return visitTest(ctx->test());
    } else if (ctx->format_string()) {
      return visitFormat_string(ctx->format_string());
    } else { // ERROR
      throw;
    }
  }

  std::any visitFormat_string(Python3Parser::Format_stringContext *ctx) override {
    auto literals = ctx->FORMAT_STRING_LITERAL();
    auto testlists = ctx->testlist();
    size_t i = 0, j = 0;
    std::string ret;
    while (i < literals.size() && j < testlists.size()) {
      if (literals[i]->getSourceInterval().startsBeforeDisjoint(testlists[j]->getSourceInterval())) {
        auto str = literals[i]->getText();
        for (size_t k = 0; k < str.size(); k++) {
          if (str[k] == '{' || str[k] == '}' || str[k] == '\\') {
            k++;
            switch (str[k]) {
              case 't' : {
                ret.push_back('\t');
                break;
              }
              case 'n' : {
                ret.push_back('\n');
                break;
              }
              case '\\' : case '\'' : case '\"' : case '{' : case '}': default: {
                ret.push_back(str[k]);
                break;
              }
            }
          } else {
            ret.push_back(str[k]);
          }
        }
        i++;
      } else {
        auto t = visitTestlist(testlists[j]);
        ret.append(std::string(rValue(std::any_cast<Value>(t))));
        j++;
      }
    }
    while (i < literals.size()) {
        auto str = literals[i]->getText();
        for (size_t k = 0; k < str.size(); k++) {
          if (str[k] == '{' || str[k] == '}' || str[k] == '\\') {
            k++;
            switch (str[k]) {
              case 't' : {
                ret.push_back('\t');
                break;
              }
              case 'n' : {
                ret.push_back('\n');
                break;
              }
              case '\\' : case '\'' : case '\"' : case '{' : case '}': default: {
                ret.push_back(str[k]);
                break;
              }
            }
          } else {
              ret.push_back(str[k]);
          }
        }
        i++;
    }
    while (j < testlists.size()) {
      ret.append(std::string(rValue(std::any_cast<Value>(visitTestlist(testlists[j])))));
      j++;
    }
    return Value(rValue(ret));
  }

  std::any visitTestlist(Python3Parser::TestlistContext *ctx) override {
    auto tests = ctx->test();
    if (tests.size() == 1) {
      return visitTest(tests[0]);
    } else {
      Tuple ret;
      for (auto test : tests) {
        ret.emplace_back(std::any_cast<Value>(visitTest(test)));
      }
      return ret;
    }
  }

  std::any visitArglist(Python3Parser::ArglistContext *ctx) override {
    auto args = ctx->argument();
    FunctionParams ret;
    for (auto arg : args) {
      ret.push_back(std::any_cast<Argument>(visitArgument(arg)));
    }
    return ret;
  }

  std::any visitArgument(Python3Parser::ArgumentContext *ctx) override {
    auto tests = ctx->test();
    if (tests.size() == 1) {
      return Argument("", std::any_cast<Value>(visitTest(tests[0])));
    } else {
      return Argument(tests[0]->getText(), std::any_cast<Value>(visitTest(tests[1])));
    }
  }
};

#endif // PYTHON_INTERPRETER_EVALVISITOR_H
