#include "scipp_program.h"
#include <assert.h>
#include "scipp_axb_p.h"
#include "scipp_expr.h"
#include "scipp_value.h"

ScpTryProgramParse::ScpTryProgramParse()
	: ScpIExpr("ScpTryProgramParse", ScpToken() )
	, Parsers3(0), OrigScrIfAny(0,0)
	, TryGroupingExpr(0)
{
	reinit2();
}
ScpTryProgramParse::~ScpTryProgramParse()
{
	clear4();
}
void ScpTryProgramParse::clear4()
{
	if(TryGroupingExpr){
		delete TryGroupingExpr;
		TryGroupingExpr = 0;
	}
	if(Parsers3){
		delete Parsers3;
		Parsers3 = 0;
	}
	std::vector<ScpIExpr*>::iterator a;
	for( a = Expressions2.begin(); a != Expressions2.end(); ++a ){
		assert(*a);
		delete *a;
	}
	Expressions2.clear();
}
void ScpTryProgramParse::reinit2()
{
	clear4();
	TryGroupingExpr = new ScpTryGroupingExpr(this);
	Parsers3 = new ScpTry1stOkExpr;
	Parsers3->addTryParser( TryGroupingExpr, 0 );
}
bool ScpTryProgramParse::tryy( const ScpParse& inp )
{
	assert( !inp.out3.expr2 );
	assert( inp.out3.err.tpe3 == SCP_EE_Unknown );
	ScpTCITR a;
	for( a = inp.tokenbgn; a != inp.tokenend;   ){
		{
			//ScpParse_ in2 = { a, inp.tokenend, inp.out3, };
			ScpParse in2( inp, a, inp.tokenend, inp.out3 );
			if( !Parsers3->tryy( in2 ) )
				return 0;
			assert( inp.out3.err.tpe3 == SCP_EE_Unknown );
			if( !inp.out3.expr2 ){ // if no expression was recognized at token 'a'.
				//printf("%d\n", SCP_EE_UnparsableExpr );
				inp.out3.err.tpe3   = SCP_EE_UnparsableExpr;
				inp.out3.err.errpos = a->tkn;
				return 0;
			}
			assert( a != inp.out3.endded3 );
			assert( inp.out3.expr2 );
			inp.out3.expr2->setOpnToken( *a );
		}
		a = inp.out3.endded3;
		Expressions2.push_back( inp.out3.expr2 );
		inp.out3.expr2 = 0;
	}
	return 1;
}
void ScpTryProgramParse::setOriginalScript( const char* inp, int len )
{
	OrigScrIfAny.first = inp;
	OrigScrIfAny.second = len;
}
std::string ScpTryProgramParse::getLineAndCol( const char* szPos )const
{
	if( OrigScrIfAny.first ){
		int iCol = -1, lnn;
		int poss = szPos - OrigScrIfAny.first;
		lnn = ScpSp::convPosToLineAndCol(
					OrigScrIfAny.first,
					OrigScrIfAny.second,
					poss, 0, &iCol, 0 );
		return *ScpStr("%a,%a").a(lnn+1).a(iCol+1);
	}
	return "";
}
std::string ScpTryProgramParse::strPrint2( const ScpPrnt& inp )const
{
	ScpPrnt in2( inp );
	in2.addTabs();
	std::string z;
	z += *ScpStr("%a""Program: n:%a {\n%a")
			.a( inp.tbs )
			.a( (int)Expressions2.size() )
			.a( inp.tbs );

	std::vector<ScpIExpr*>::const_iterator a;
	for( a = Expressions2.begin(); a != Expressions2.end(); ++a ){
		z += (**a).strPrint2( in2 );
		if( OrigScrIfAny.first )
			z += " (" + getLineAndCol( (**a).getOpnToken().tkn.ptr ) + ")";
		z += "\n";
	}
	z += *ScpStr("%a}\n").a(inp.tbs);
	return z;
}
bool ScpTryProgramParse::eval6( const ScpEval& ev2 )
{
	std::vector<ScpIExpr*>::const_iterator a;
	for( a = Expressions2.begin(); a != Expressions2.end(); ++a ){
		ScpEvalOu evOu( ev2.out4.err4 );
		ScpEval ev3( ev2, evOu );
		if( !(**a).eval6( ev3 ) ){
			assert( ev2.out4.err4.errpos.ptr );
			assert( !ev2.out4.value2 );
			return 0;
		}
		assert( !ev2.out4.err4.errpos.ptr );
		/// \todo here, handling when expression is returning the 'return' value.
		///       for this to work, other things must be implemented as well.
		ev3.clear5();
	}
	// TODO: return the 'void' value here.
	return 1;
}




