#include "context.hpp"
#include <map>
#include <algorithm>
#include <fstream>
#include <cuddInt.h>

void Context::printLetExpression(BF data, bool html, std::ostream &os) const {

    DdNode *fn = data.getCuddNode();
    DdManager *mgr = data.manager()->getMgr();
    std::map<unsigned int, std::string> varNames;
    for (unsigned int i=0;i<variables.size();i++) {
        BF var = variables[i];
        varNames[var.readNodeIndex()] = variableNames[i];
    }

    std::list<DdNode*> todo;
    std::map<DdNode*,std::string> mapping;
    std::list<std::string> letDeclarations;
    todo.push_back(fn);
    mapping[fn] = "fn";
    os << "fn\n";
    mapping[Cudd_ReadOne(mgr)] = "true";
    mapping[Cudd_Not(Cudd_ReadOne(mgr))] = "false";
    while (todo.size()!=0) {
        DdNode *next = *(todo.begin());
        todo.erase(todo.begin());
        DdNode *trueSucc;
        DdNode *falseSucc;
        if (Cudd_IsComplement(next)) {
            trueSucc = Cudd_Not(Cudd_Not(next)->type.kids.T);
            falseSucc = Cudd_Not(Cudd_Not(next)->type.kids.E);
        } else {
            trueSucc = next->type.kids.T;
            falseSucc = next->type.kids.E;
        }
        for (auto reg : {trueSucc,falseSucc}) {
            if (mapping.count(reg)==0) {
                std::ostringstream nodeName;
                nodeName << "n" << mapping.size();
                mapping[reg] = nodeName.str();
                todo.push_back(reg);
            }
        }

        os << "for " << mapping[next] << " = (!" << varNames[Cudd_Regular(next)->index] << " & " << mapping[falseSucc] << ") | (" << varNames[Cudd_Regular(next)->index] << " & " << mapping[trueSucc] << ")\n";
    }

}



void Context::printDNF(BF data, bool html, std::ostream &os) const {

    BF remaining = data;

    if (data.isTrue()) {
        if (html) {
            os << "<p class=\"dnf\">The BF is equivalent to TRUE.</p>";
        } else {
            os << "The BF is equivalent to TRUE.\n";
        }
        return;
    } else if (data.isFalse()) {
        if (html) {
            os << "<p class=\"dnf\">The BF is equivalent to FALSE.</p>";
        } else {
            os << "The BF is equivalent to FALSE.\n";
        }
        return;
    }

    if (html) {
        os << "<p class=\"dnf\">";
        os << "DNF:<BR/>";
    } else {
        os << "DNF:\n";
    }

    bool firstMinterm = true;
    unsigned int nofClausesSoFar = 0;
    while (!(remaining.isFalse())) {

        // Find a non-sat assignment
        BF assignment = remaining;
        std::vector<bool> startingAssignment;
        for (unsigned int i=0;i<variables.size();i++) {
            BF thisTry = (assignment & variables[i]);
            if (thisTry.isFalse()) {
                assignment = (assignment & !variables[i]);
                startingAssignment.push_back(false);
            } else {
                assignment = thisTry;
                startingAssignment.push_back(true);
            }
        }

        // Now take the assignment and shorten it (greedily)
        std::vector<int> thisClause;
        BF clauseSoFar = mgr.constantTrue();
        for (unsigned int i=0;i<variables.size();i++) {
            BF newClause = clauseSoFar;
            for (unsigned int j=i+1;j<variables.size();j++) {
                if (startingAssignment[j]) {
                    newClause &= variables[j];
                } else {
                    newClause &= !variables[j];
                }
            }
            if ((newClause & !data).isFalse()) {
                // No Literal to be added...
            } else {
                if (startingAssignment[i]) {
                    thisClause.push_back((i+1));
                    clauseSoFar &= variables[i];
                } else {
                    thisClause.push_back(-1*(i+1));
                    clauseSoFar &= !(variables[i]);
                }
            }
        }

        remaining &= !clauseSoFar;

        if (html) {
            if (firstMinterm) {
                firstMinterm = false;
            } else {
                os << "<BR/>";
            }
        }
        os << "- ";
        for (unsigned int i=0;i<thisClause.size();i++) {
            if (i>0) os << " & ";
            if (thisClause[i]<0) {
                os << "!" << variableNames[-1*thisClause[i]-1];
            } else {
                os << variableNames[thisClause[i]-1];
            }
        }
        os << std::endl;

        if (nofClausesSoFar++>50) {
            if (html) os << "<BR/>";
            os << "...(and some more)";
            if (html) {
                os << "</p>";
            } else {
                os << std::endl;
            }
            return;
        }

    }

    if (html) os << "</p>";
}
