#include <iostream>
#include <algorithm>
#include "context.hpp"
#include <fstream>
#include <map>
#include <boost/algorithm/string/trim.hpp>
#include "booleanFormulaParser.hpp"

auto constexpr HTML_ERROR_START = "<p class=\"error\">";
auto constexpr HTML_ERROR_END = "</p>\n";
auto constexpr HTML_BDD_START = "<p class=\"bdd\">";
auto constexpr HTML_BDD_END = "</p>\n";
auto constexpr HTML_COPY_START = "<p class=\"copied\">";
auto constexpr HTML_COPY_END = "</p>\n";
auto constexpr HTML_FINALLINE_START = "<p class=\"terminalLine\">";
auto constexpr HTML_FINALLINE_END = "</p>\n";


void processCommands(Context &context, std::istream &inStream, std::ostream &outStream, bool html) {

    try {
        // Parse input file

        if (inStream.fail()) throw "Error: Could not read input.";
        std::string currentLine;
        unsigned int lineNumber = 0;
        unsigned int graphNum = 0;
        std::map<std::string,BF> definedBFs;
        while (std::getline(inStream,currentLine)) {
            lineNumber++;
            boost::trim(currentLine);

            if (html) {
                if (html) outStream << HTML_COPY_START;
                outStream << "&gt;&nbsp;" << currentLine;
                if (html) outStream << HTML_COPY_END;
            }

            std::cerr << "Joojo: " << currentLine.substr(0,10) << std::endl;

            if (currentLine.substr(0,2)=="//") {
                // Comment, ignore
            } else if (currentLine.substr(0,10)=="printLetX ") {

                std::string varDef = currentLine.substr(10,std::string::npos);
                boost::trim(varDef);

                if (definedBFs.count(varDef)==0) {
                    if (html) outStream << HTML_ERROR_START;
                    outStream << "Error in line " << lineNumber << ": The boolean function " << varDef << " was not defined.";
                    if (html) outStream << HTML_ERROR_END;
                    return;
                }

                context.printLetExpression(definedBFs[varDef],html,outStream);


            } else if (currentLine.substr(0,9)=="printDNF ") {

                std::string varDef = currentLine.substr(9,std::string::npos);
                boost::trim(varDef);

                if (definedBFs.count(varDef)==0) {
                    if (html) outStream << HTML_ERROR_START;
                    outStream << "Error in line " << lineNumber << ": The boolean function " << varDef << " was not defined.";
                    if (html) outStream << HTML_ERROR_END;
                    return;
                }

                context.printDNF(definedBFs[varDef],html,outStream);

            } else if (currentLine.substr(0,9)=="printBDD ") {

                std::string varDef = currentLine.substr(9,std::string::npos);
                boost::trim(varDef);

                if (definedBFs.count(varDef)==0) {
                    if (html) outStream << HTML_ERROR_START;
                    outStream << "Error in line " << lineNumber << ": The boolean function " << varDef << " was not defined.";
                    if (html) outStream << HTML_ERROR_END;
                    return;
                }

                // Print a test
                if (html) outStream << HTML_BDD_START;
                outStream << "<div id=\"graph" << graphNum << "\">Hello!</div>\n";
                outStream << "<MyBDD>graph" << graphNum << "<MyBDD>" << "digraph { A -> B; }" << "<MyBDD>";
                if (html) outStream << HTML_BDD_END;
                graphNum++;

            } else if (currentLine.substr(0,4)=="var ") {

                // Define variable
                std::string varDef = currentLine.substr(4,std::string::npos);
                boost::trim(varDef);

                // Variable name illegal?
                if ((varDef=="1") || (varDef=="0") || (varDef=="f") || (varDef=="t") || (varDef=="true") || (varDef=="false") || (varDef=="True") || (varDef=="False") || (varDef=="TRUE") || (varDef=="FALSE") || (varDef=="exists") || (varDef=="forall")) {
                    if (html) outStream << HTML_ERROR_START;
                    outStream << "Error: Illegal variable name in line " << lineNumber << std::endl;
                    if (html) outStream << HTML_ERROR_END;
                    return;
                }

                // See if variable already existing
                if (context.hasVariable(varDef)) {
                    if (html) outStream << HTML_ERROR_START;
                    outStream << "Error: The variable in line " << lineNumber << " has been defined multiple times.";
                    if (html) outStream << HTML_ERROR_END;
                    return;
                } else {

                    if (definedBFs.count(varDef)!=0) {
                        if (html) outStream << HTML_ERROR_START;
                        outStream << "Error in line " << lineNumber << ": Variables must be assigned to fresh identifiers.";
                        if (html) outStream << HTML_ERROR_END;
                        return;
                    }
                    definedBFs[varDef] = context.newVariable(varDef);

                }

            } else if (currentLine.length()>0) {
                // Ok, a new command
                // Search for equality sign
                size_t posEqual = currentLine.find("=");
                if (posEqual==std::string::npos) {
                    if (html) outStream << HTML_ERROR_START;
                    outStream << "Error: Line " << lineNumber << " does not represent a valid assignment.";
                    if (html) outStream << HTML_ERROR_END;
                    return;
                }
                std::string assignTo = currentLine.substr(0,posEqual);
                boost::trim(assignTo);

                if (context.hasVariable(assignTo)) {
                    if (html) outStream << HTML_ERROR_START;
                    outStream << "Error: Trying to assign to a name that is already in use for a variable in line " << lineNumber << ".";
                    if (html) outStream << HTML_ERROR_END;
                    return;
                }

                if ((assignTo=="1") || (assignTo=="0") || (assignTo=="f") || (assignTo=="t") || (assignTo=="true") || (assignTo=="false") || (assignTo=="True") || (assignTo=="False") || (assignTo=="TRUE") || (assignTo=="FALSE")) {
                    if (html) outStream << HTML_ERROR_START;
                    outStream << "Error: Trying to assign to a reserved name in line " << lineNumber << std::endl;
                    if (html) outStream << HTML_ERROR_END;
                    return;
                }

                std::string formula = currentLine.substr(posEqual+1,std::string::npos);
                boost::trim(formula);

                BF thisData;
                try {
                    thisData = parseBooleanFormula(formula,context,definedBFs,outStream);
                    definedBFs[assignTo] = thisData;
                } catch (std::string error) {
                    if (html) outStream << HTML_ERROR_START;
                    outStream << "Error in line " << lineNumber << ": " << error << std::endl;
                    if (html) outStream << HTML_ERROR_END;
                    return;
                } catch (ParseException error) {
                    if (html) outStream << HTML_ERROR_START;
                    outStream << "Error in line " << lineNumber << ": " << error.getMsg() << std::endl;
                    if (html) outStream << HTML_ERROR_END;
                    return;
                }


            }
        }

        if (inStream.bad()) {
            if (html) outStream << HTML_ERROR_START;
            outStream << "Error reading input file to the end.\n";
            if (html) outStream << HTML_ERROR_END;
            return;
        }

        if (html) outStream << HTML_FINALLINE_START;
        outStream << "End of input.\n";
        if (html) outStream << HTML_FINALLINE_END;

        return;

    } catch (const char *error) {
        if (html) outStream << HTML_ERROR_START;
        outStream << error << std::endl;
        if (html) outStream << HTML_ERROR_END;
    } catch (const std::string error) {
        if (html) outStream << HTML_ERROR_START;
        outStream << error << std::endl;
        if (html) outStream << HTML_ERROR_END;
    } catch (const ParseException error) {
        if (html) outStream << HTML_ERROR_START;
        outStream << error.getMsg() << std::endl;
        if (html) outStream << HTML_ERROR_END;
    }

}

#ifdef __EMSCRIPTEN__

extern "C" char *runHTMLVersion(const char *inData);

char *runHTMLVersion(const char *inData) {

	std::ostringstream os;
    std::istringstream is(inData);
    Context context;
    processCommands(context,is,os,true);

    std::string allOutput = os.str();
    char *retData = (char*)malloc(sizeof(char)*(allOutput.length()+1));
    strcpy(retData,allOutput.c_str());
	return retData;

}


#else

int main(int argc, const char **args) {
    (void)argc;
    (void)args;
	std::cout << "Boollab v.1.0" << std::endl;

    Context context;

    processCommands(context,std::cin,std::cout,false);

}

#endif
