#include <stdio.h>
#include <string>
#include "scipp/scipp_script.h"

// Example class that can be exported to the scripting engine.
// Once visible in the scripts, values can be added or multiplied.
// Additionally, variable can be used as a function and call to it can
// be made using random number of arguments.
class MyNumber : public ScpHostValue
{
public:
	// Constructor can be anything.
	// Here its function is only to assign default value.
	MyNumber( int nrVal=0 ) : NrValue(nrVal) {}

	// Implementation of multiplicative operator, the asterisk character ('*').
	// Once this member function is reiplemented and does not
	// return 'false', in the scripts, multiplication of this class by another
	// value is possible.
	virtual bool evalOperatorMul( const ScpSpeOper& inp )const
	{
		*inp.result2 = new MyNumber( NrValue * (int)inp.otherr->stdToDouble() );
		return 1;
	}

	// Similairly to multiplicative operator, this is implementation of
	// the additive operator ('+').
	virtual bool evalOperatorAdd( const ScpSpeOper& inp )const
	{
		*inp.result2 = new MyNumber( NrValue + (int)inp.otherr->stdToDouble() );
		return 1;
	}

	// Function call implementation.
	// To allow the variable to be called as a function from the scripts.
	// Implementation here only prints actual call arguments to the STDOUT.
	// All input variables are assumed to be integers, thus, if other type of
	// variable is passed, eg. a string, it is internally converted to the
	// 'double' type first. Example code below downcasts it to 'int'.
	virtual bool evalFunctionCall( const ScpSpeCall& inp )
	{
		std::string str;
		char bfr[64];
		for( int i=0; i < inp.argc2; i++ ){
			sprintf(bfr,"%d,", (int)inp.argv2[i]->stdToDouble() );
			str += bfr;
		}
		str = ( str.empty() ? "<empty>" : str );
		printf("MyNumber::evalFunctionCall(), arguments: %s\n", str.c_str() );
		return 1;
	}

	// Implementation of this member function is required to allow 
	// number-like manipulation.
	// In this simple example it is enough to type cast return value from 'int'.
	virtual double stdToDouble()const {return (double)NrValue;}

	// Called when there is an assignment from some other variable in the script.
	// Code here is assuming that the other value is of some numeric
	// type and is retrieving it's decimal value. N.b. via scripts, main use of 
	// it is to access another variable of this very class.
	// All internal types, eg. string, have this member function implemented,
	// so there will be an attempt to convert string to a numeric representation
	// in case of such expression is evaluated in the script.
	virtual bool evalOperatorAssign( const ScpSpeOper& inp )
	{
		NrValue = (int) inp.otherr->stdToDouble();
		return 1;
	}

	// Actual integer value that this class represents.
	// Using public access to easilly show it in the STDOUT at the end of the example 
	// in main() function.
	int NrValue;
};

int main()
{
	// Actual source code of this example, script that is to be parsed and evaluated.
	// Presented as c-string (or std::string) here, in real applications instead, 
	// scripts could be loaded from external sources such as text files.
	std::string myCcode =
		"// Script example to run. \n"
		"var a = '10' \n"
		"v2(v3,v4)            // simple arithmetic. \n"
		"v3 = a \n"
		"v4 = (v2 + v3) * v3  // use of parentheses to force '+' before '*'. \n";

	// Create instance of the script parsing object.
	ScpScript* sci = ScpCreateScript( myCcode.c_str(), -1, 0 );

	// Uncomment the line below to have tokens and expressions
	// shown for debug purposes, an internal representation of the script parsing process.
	//sci->setOptions( SCP_SF_ShowTokens|SCP_SF_ShowExpressions );

	// Parse the script and if error occurs, print info to STDOUT and exit.
	if( !sci->parse2() ){
		// If error occurs, call to member function below will output 
		// detailed message into console STDOUT.
		sci->showMeTheError2(0);
		delete sci;
		return 2;
	}
	// Instantiate host variables with default values.
	// Later on successfull parse, theese are possibly modified according to
	// the script used, that is, in this example entire script is the 'myCcode' variable.
	MyNumber v2(2), v3(3), v4(4);

	// Add the variables to the script engine, so that they're visible by the text script.
	// Here, they're being exported past the parsing step, just before eavl step.
	// This can be done earlier as well.
	sci->addHostVariable("v2", &v2, 0 );
	sci->addHostVariable("v3", &v3, 0 );
	sci->addHostVariable("v4", &v4, 0 );

	// Evaluate the script.
	if( !sci->eval2() ){
		sci->showMeTheError2(0);
		delete sci;
		return 3;
	}
	printf("Showing state of the host variables.\n");
	printf("v2: %d\n", v2.NrValue );
	printf("v3: %d\n", v3.NrValue );
	printf("v4: %d\n", v4.NrValue );

	delete sci;
	printf("Application exit...\n");
	return 0;
}

