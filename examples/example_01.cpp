#include <stdio.h>
#include <string>
#include "scipp_script.h"

class MyNumber : public ScpHostObject
{
public:
	MyNumber( int ResultVal_=0 ) : ResultVal(ResultVal_) {}
	virtual MyNumber operator*( const MyNumber& other ){
		return MyNumber( ResultVal * other.ResultVal );
	}
	virtual MyNumber operator+( const MyNumber& other ){
		return MyNumber( ResultVal + other.ResultVal );
	}
	virtual bool evaluateFunctionCall( const ScpHoc& inp ){
		printf("Calling user function, n:%d.\n", inp.argvc );
		return 1;
	}
	int ResultVal;
};

int main()
{
	std::string myCcode =
		"// Script example to run. \n"
		"v2(v3,v4)            // simple arithmetic.\n"
		"v4 = (v2 + v3) * v3  // use of parentheses to force '+' before '*'.\n"
		;
	ScpScript* sci = ScpCreateScript( myCcode.c_str(), -1, 0 );
	sci->setOptions( SCP_SF_ShowTokens|SCP_SF_ShowExpressions );
	if( !sci->parse2() ){
		sci->showMeTheError2();
		return 2;
	}
	MyNumber v2(2), v3(3), v4(4);
	sci->addHostOperator( new ScpMultiplicativeOperator<MyNumber>, 1 );
	sci->addHostOperator( new ScpAsssignmentOperator<MyNumber>, 1 );
	sci->addHostOperator( new ScpPlusOperator<MyNumber>, 1 );
	sci->addHostObject( "v2", &v2, 0 );
	sci->addHostObject( "v3", &v3, 0 );
	sci->addHostObject( "v4", &v4, 0 );
	if( !sci->eval2() ){
		sci->showMeTheError2();
		return 3;
	}
	printf("v2: %d\n", v2.ResultVal );
	printf("v3: %d\n", v3.ResultVal );
	printf("v4: %d\n", v4.ResultVal );
	delete sci;
	printf("Application exit...\n");
	return 0;
}
