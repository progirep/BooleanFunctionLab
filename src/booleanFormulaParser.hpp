#ifndef __BOOLEAN_FORMULA_PARSER_HPP__
#define __BOOLEAN_FORMULA_PARSER_HPP__

#include "context.hpp"
#include <map>

BF parseBooleanFormula(std::string const &formula, Context const &context, const std::map<std::string,BF> &definedBFs);

#endif
