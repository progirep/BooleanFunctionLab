#ifndef __BOOLEAN_FORMULA_PARSER_HPP__
#define __BOOLEAN_FORMULA_PARSER_HPP__

#include "context.hpp"
#include <map>
#include <iostream>
#include <string>

class ParseException {
    std::string msg;
public:
    ParseException(std::string _msg) : msg(_msg) {};
    std::string getMsg() const { return msg; }
};

BF parseBooleanFormula(std::string const &formula, Context const &context, const std::map<std::string,BF> &definedBFs, std::ostream &os);

#endif
