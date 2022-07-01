#ifndef __ENCODER_CONTEXT_HPP
#define __ENCODER_CONTEXT_HPP

#include "BF.h"
#include "bddDump.h"
#include <vector>
#include <set>

class Context : public VariableInfoContainer {
protected:
    BFManager mgr;
    std::vector<std::string> variableNames;
    std::vector<BF> variables;

public:

    // Building a problem
    BF newVariable(std::string newName) {
        variableNames.push_back(newName);
        BF newVar = mgr.newVariable();
        variables.push_back(newVar);
        return newVar;
    }
    BF getVariable(unsigned int nr) const {
        return variables.at(nr);
    }

    const BFManager &getBFMgr() const { return mgr; }
    unsigned int getNofVariables() const { return variables.size(); }


    BF getVariableBF(std::string name) const {
        for (unsigned int i=0;i<variables.size();i++) {
            if (variableNames[i]==name) return variables[i];
        }
        throw "Error: Variable '"+name+"' was not declared.";
    }

    bool hasVariable(std::string name) {
        for (unsigned int i=0;i<variables.size();i++) {
            if (variableNames[i]==name) return true;
        }
        return false;
    }

    // Implementations for the VariableInfoContainer
    void getVariableTypes(std::vector<std::string> &types) const {
        types.push_back("all");
    }

    virtual void getVariableNumbersOfType(std::string type, std::vector<unsigned int> &nums) const {
        if (type!="all") throw "Illegal variable type";
        for (unsigned int i=0;i<variables.size();i++) nums.push_back(i);
    }

    BF getVariableBF(unsigned int number) const {
        return variables.at(number);
    }

    std::string getVariableName(unsigned int number) const {
        return variableNames.at(number);
    }

    void printDNF(BF data, bool html, std::ostream &os) const;
    void printLetExpression(BF data, bool html, std::ostream &os) const;

};



#endif
