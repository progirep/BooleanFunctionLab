//========================================================================
// This file contains the parser for Boolean formulas
//
// It is a modification of code given by ??? on
// http://stackoverflow.com/questions/12598029/how-to-calculate-boolean-expression-in-spirit
//
// and licensed separately from the rest of the program under
// CC-BY-SA
// terms (including the modifications performed).

#include "booleanFormulaParser.hpp"
#include <set>

// Tokenizer
void getTokens(std::string const &formula, std::vector<std::string> &tokens, std::ostream &os) {
    int fptr = 0;
    std::string currentID = "";
    while (fptr<formula.length()) {
        //os << fptr << "-";
        char c = formula[fptr];
        bool notend = fptr<formula.length()-1;

        // part 1: Long token parsing
        if ((c==' ') || (c=='\n') || (c=='\t') || (c==0)) {
            // do nothing
            if (currentID!="") {
                tokens.push_back(currentID);
                //os << "LB" << currentID.size();
                currentID = "";
            }
            fptr++;
            goto nextone;
        } else if (((c>='A') && (c<='Z')) || ((c>='a') && (c<='z')))  {
            currentID += c;
            fptr++;
            goto nextone;
        } else if ((c=='_') && (currentID!="")) {
            currentID += c;
            fptr++;
            goto nextone;
        } else if ((c>='0') && (c<='9') && (currentID!="")) {
            currentID += c;
            fptr++;
            goto nextone;
        } else if (currentID!="") {
            //os << "LA" << currentID.size();
            tokens.push_back(currentID);
            currentID = "";
        }

        // Part 2: Process rest
        if ((c=='-') && notend && (formula[fptr+1]=='>')) {
            tokens.push_back("->");
            fptr+=2;
            goto nextone;
        } else if ((c=='|') && notend && (formula[fptr+1]=='|')) {
            fptr++; // redundant
        } else if ((c=='&') && notend && (formula[fptr+1]=='&')) {
            fptr++; // redundant
        }

        // Single characters
        //os << "LX" << (int)c;
        tokens.push_back(std::string("")+c);
        fptr++;
nextone:
        (void)fptr; // NOP -- There has to be a command after a label.
    }
    if (currentID!="") throw "Error: currentID must be empty at the end.";
}



// Order of operations:


// exists, forall
// swap
// OR
// AND
// NOT
// BRACES
// VAR & CONSTANTS
BF parseExists(Context const &context, const std::map<std::string,BF> &definedBFs,std::vector<std::string> const &tokens, int &pos);

BF parseAtomic(Context const &context, const std::map<std::string,BF> &definedBFs,std::vector<std::string> const &tokens, int &pos) {
    if (pos>=tokens.size()) throw ParseException("Unexpected end of the expression");
    std::string thisOne = tokens[pos++];
    if (thisOne=="1") return context.getBFMgr().constantTrue();
    if (thisOne=="0") return context.getBFMgr().constantFalse();
    if (definedBFs.count(thisOne)>0) return definedBFs.at(thisOne);

    std::ostringstream osm;
    osm << "Identifyer '" << thisOne << "' not recognized.";
    throw ParseException(osm.str());
}



BF parseBraces(Context const &context, const std::map<std::string,BF> &definedBFs,std::vector<std::string> const &tokens, int &pos) {
    if (pos>=tokens.size()) throw ParseException("Unexpected end of the expression");
    if (tokens[pos]=="(") {
        pos++;
        BF fresh = parseExists(context,definedBFs,tokens,pos);
        if (pos>=tokens.size()) throw ParseException("Unexpected end of the brace expression");
        if (tokens[pos]!=")") {
            throw ParseException("Braces not properly closed. "+tokens[pos]);
        }
        pos++;
        return fresh;
    };
    return parseAtomic(context,definedBFs,tokens,pos);
}

BF parseNot(Context const &context, const std::map<std::string,BF> &definedBFs,std::vector<std::string> const &tokens, int &pos) {
    if (pos>=tokens.size()) throw ParseException("Unexpected end of the expression");
    if (tokens[pos]=="!") {
        pos++;
        return !(parseNot(context,definedBFs,tokens,pos));
    }
    return parseBraces(context,definedBFs,tokens,pos);
}

BF parseSwap(Context const &context, const std::map<std::string,BF> &definedBFs,std::vector<std::string> const &tokens, int &pos) {
    if (pos>=tokens.size()) throw ParseException("Unexpected end of the expression");

    // First parse something. If that works, then see if a squared brace is following
    BF subPart = parseNot(context,definedBFs,tokens,pos);

    while ((pos<tokens.size()) && (tokens[pos]=="[")) {
        // Substitution...
        //std::ostringstream m;
        //m << "SUBS A" << tokens[pos].size();
        //throw ParseException(m.str());
        pos++;
        std::vector<BF> varsA;
        std::vector<BF> varsB;
        while (true) {
            // Parse next var
            if (pos>=tokens.size()) throw ParseException("Unexpected end of swapping expression (A)");
            BF nextVar = context.getVariableBF(tokens[pos++]);
            varsA.push_back(nextVar);
            if (pos>=tokens.size()) throw ParseException("Unexpected end of swapping expression (B)");
            if (tokens[pos++]!="/") {
                throw ParseException("Expected '/' in swapping expression");
            }
            if (pos>=tokens.size()) throw ParseException("Unexpected end of swapping expression (C)");
            nextVar = context.getVariableBF(tokens[pos++]);
            varsB.push_back(nextVar);
            if (pos>=tokens.size()) throw ParseException("Unexpected end of swapping expression (D)");
            if (tokens[pos]==",") {
                pos++;
            } else {
                break; // Exit loop
            }
        }
        if (tokens[pos++]!="]") {
            throw ParseException("Expected an ']' at the end of a swapping expression.");
        }

        // Sanity check - No overlap!
        std::set<DdNode*> parts;
        for (auto it : varsA) parts.insert(it.getCuddNode());
        for (auto it : varsB) parts.insert(it.getCuddNode());
        if (parts.size()!=varsA.size()+varsB.size()) throw ParseException("When swapping, variables must not occur twice!");

        BFVarVector v1 = context.getBFMgr().computeVarVector(varsA);
        BFVarVector v2 = context.getBFMgr().computeVarVector(varsB);
        subPart = subPart.SwapVariables(v1,v2);

    }
    return subPart;
}



BF parseAnd(Context const &context, const std::map<std::string,BF> &definedBFs,std::vector<std::string> const &tokens, int &pos) {
    if (pos>=tokens.size()) throw ParseException("Unexpected end of the expression");

    BF subPart = parseSwap(context,definedBFs,tokens,pos);
    if (pos>=tokens.size()) return subPart;
    if (tokens[pos]=="&") {
        pos++;
        BF subPart2 = parseAnd(context,definedBFs,tokens,pos);
        return subPart & subPart2;
    } else {
        return subPart;
    }
}

BF parseOr(Context const &context, const std::map<std::string,BF> &definedBFs,std::vector<std::string> const &tokens, int &pos) {
    if (pos>=tokens.size()) throw ParseException("Unexpected end of the expression");

    BF subPart = parseAnd(context,definedBFs,tokens,pos);
    if (pos>=tokens.size()) return subPart;
    if (tokens[pos]=="|") {
        pos++;
        BF subPart2 = parseOr(context,definedBFs,tokens,pos);
        return subPart | subPart2;
    } else if (tokens[pos]=="->") {
        pos++;
        BF subPart2 = parseOr(context,definedBFs,tokens,pos);
        return !subPart | subPart2;
    } else {
        return subPart;
    }
}


BF parseExists(Context const &context, const std::map<std::string,BF> &definedBFs,std::vector<std::string> const &tokens, int &pos) {
    if (pos>=tokens.size()) throw ParseException("Unexpected end of the expression");

    if ((tokens[pos]=="exists") || (tokens[pos]=="forall")) {
        bool isExists = tokens[pos]=="exists";
        pos++;
        std::vector<BF> varsAbstract;
        while (true) {
            // Parse next var
            if (pos>=tokens.size()) throw ParseException("Unexpected end of abstraction expression");
            BF nextVar = context.getVariableBF(tokens[pos]);
            varsAbstract.push_back(nextVar);
            pos++;
            if (pos>=tokens.size()) throw ParseException("Unexpected end of abstraction expression");
            if (tokens[pos]==",") {
                pos++;
            } else {
                break; // Exit loop
            }
        }

        BFVarCube cube = context.getBFMgr().computeCube(varsAbstract);

        // consume dot if present
        if (pos>=tokens.size()) throw ParseException("Unexpected end of abstraction expression");
        if (tokens[pos]==".") pos++;

        BF toAbstract = parseExists(context,definedBFs,tokens,pos);
        if (isExists)
            return toAbstract.ExistAbstract(cube);
        else
            return toAbstract.UnivAbstract(cube);

    } else {
        return parseOr(context,definedBFs,tokens,pos);
    }
}



BF parseBooleanFormula(std::string const &formula, Context const &context, const std::map<std::string,BF> &definedBFs, std::ostream &os) {

    std::vector<std::string> tokens;
    getTokens(formula+" ",tokens,os);

    // Debug
    /*os << tokens.size() << "\n" << "\n";
    os << "(" << tokens[3].size();
    for (auto a : tokens) {
        os << "," << a;
    }
    os << ")\n";*/

    int posTokens = 0;
    BF result = parseExists(context,definedBFs,tokens,posTokens);

    if (posTokens!=tokens.size()) {
        // Is it the start of a comment?
        if (posTokens<=tokens.size()-2) {
            if ((tokens[posTokens]=="/") && (tokens[posTokens+1]=="/")) return result;
        }
        // Otherwise it's superfluous tokens...
        std::ostringstream osm;
        osm << "Superfluous tokens in expression, starting with '" << tokens[posTokens] << "'";
        throw ParseException(osm.str());
    }

    return result;

}
