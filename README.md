# Skade-Script
----------------
Welcome to the C++ scripting mini engine that aims to implement a handfull set of features from the <a href="https://www.ecma-international.org/ecma-262/5.1/#sec-5.1.3
">ECMA Script</a> specification. A way to freely manipulate objects exported from the C++ using arithmetic, parentheses or assignment operators. Enables compilation independent scripting from external sources an application can take input from, such as: config files, command-line or UI controls.

<a href='./misc_example/example_01.cpp'>View Example</a>

# For full documenataion please see attached archive files, either in root directory or attached to the releases.
Provided are Doxygen generated API docs in two formats, zipped html and CHM.



	//
	// This example requires C++11 capable compiler.
	//
	#include <stdio.h>
	#include <string>
	#include <algorithm>
	#include "scipp/scipp_script.h"


	int main()
	{
		std::string myCcode =
			"// Script example to run.     \n"
			"var v2 = 10                   \n"
			"var v3 = host_func( v2, 'abc')\n"
			"host_func( v3, 'xyz')         \n"
			;
		ScpScript* sci = ScpCreateScript( myCcode.c_str(), -1, 0 );
		//sci->setOptions( SCP_SF_ShowTokens|SCP_SF_ShowExpressions );
		//sci->setOptions( SCP_SF_ShowValues );

		if( !sci->parse2() ){
			sci->showMeTheError2(0);
			delete sci;
			return 2;
		}
		int cnt = 0;
		sci->addHostFunction2( "host_func",
			[&]( double arg1, const char* arg2, void*, int flags2, ScpOutputStr& outp ){
				printf("Calling: host_func( %f, '%s' )...\n", (float)arg1, arg2 );
				outp.assign("11.11");
				cnt++;
				return 0.0;
		}, 0 );

		if( !sci->eval2() ){
			sci->showMeTheError2(0);
			delete sci;
			return 3;
		}
		delete sci;
		printf("Application exit...\n");
		return 0;
	}


