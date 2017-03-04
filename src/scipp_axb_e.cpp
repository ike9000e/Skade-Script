
#include "scipp_axb_e.h"
#include "scipp_tryparser.h"
#include "scipp_value.h"
#include <assert.h>
//#include <stdio.h>

ScpIdentifierExpr::ScpIdentifierExpr( const ScpToken& tkn )
	: ScpIExpr("ScpIdentifierExpr",tkn)
{
}
std::string ScpIdentifierExpr::strPrint2( const ScpPrnt& inp )const
{
	std::string z;
	z += *ScpStr("%a""'%a' (identifier)")
			.a( inp.tbs )
			.a( getOpnToken().tkn2.c_str() );
	return z;
}
ScpAxBExpr::
ScpAxBExpr( ScpIExpr* leftexpr, const ScpToken& operToken, ScpIExpr* rightexpr )
	: ScpIExpr( "ScpAxBExpr", leftexpr->getOpnToken() )
	, LeftExpr(leftexpr), RightExpr(rightexpr)
	, OpTkn(new ScpToken(operToken))
{
	assert( LeftExpr && RightExpr );
}
ScpAxBExpr::~ScpAxBExpr()
{
	assert( LeftExpr && RightExpr && OpTkn );
	delete LeftExpr;
	delete RightExpr;
	delete OpTkn;
}
std::string ScpAxBExpr::strPrint2( const ScpPrnt& inp )const
{
	ScpPrnt in2( inp );
	in2.addTabs(1);
	std::string z;
	z += *ScpStr("%a""AxB: '%a' L:[\n%a\n%a] '%a' R:[\n%a\n%a]")
			.a( inp.tbs )
			.a( OpTkn->tkn2.c_str() )
			.a( LeftExpr->strPrint2( in2 ) )
			.a( inp.tbs )
			.a( OpTkn->tkn2.c_str() )
			.a( RightExpr->strPrint2( in2 ) )
			.a( inp.tbs );
	return z;
}
ScpGroupingExpr::
ScpGroupingExpr( ScpIExpr* innerExpr, const ScpToken& opn2, const ScpToken& close2 )
	: ScpIExpr( "ScpGroupingExpr", opn2 ), InnerExpr(innerExpr)
{
	setEnclosingTokens( opn2, close2 );
}
ScpGroupingExpr::~ScpGroupingExpr()
{
	delete InnerExpr;
}
std::string ScpGroupingExpr::strPrint2( const ScpPrnt& inp )const
{
	ScpPrnt in2( inp );
	in2.addTabs(1);
	std::string z;
	z += *ScpStr("%a""GRP: '%a%a' [\n%a\n%a]")
			.a( inp.tbs )
			.a( getEnclosingTokens(0)->tkn2.c_str() )
			.a( getEnclosingTokens(1)->tkn2.c_str() )
			.a( InnerExpr->strPrint2( in2 ) )
			.a( inp.tbs );
	return z;
}
bool ScpIdentifierExpr::eval6( const ScpEval& inp )
{
	const ScpToken& tke = getOpnToken();
	assert( !tke.tkn2.empty() );
	inp.out4.value2 = new ScpSubtleRefVal( tke.tkn2.c_str(), tke );
	return 1;
}
bool ScpGroupingExpr::eval6( const ScpEval& inp )
{
	return InnerExpr->eval6(inp);
}
bool ScpAxBExpr::eval6( const ScpEval& inp )
{
	assert( LeftExpr && RightExpr );
	if( !RightExpr->eval6( inp ) )
		return 0;
	ScpAutoPtr<ScpValue> rval( inp.extractValue() );
	assert( rval() );
	if( !LeftExpr->eval6( inp ) )
		return 0;
	ScpAutoPtr<ScpValue> lval( inp.extractValue() );
	assert( lval() );
	/// \todo Evaluating of built-in expressions,
	///       now doing only host-objects.
	//printf("[%s]\n", OpTkn->tkn2.c_str() );
	int iErrIs = -1, nErr = -1;
	ScpEvalPair evp = { lval()->getVarname(), rval()->getVarname(), *OpTkn, &iErrIs, &nErr, &inp.out4.value2, lval(), rval(), };
	if( !inp.scope2->evalHostObjectPair( evp ) ){
		assert( !inp.out4.value2 );
		inp.errClear2( ScpErr(
					( iErrIs == -1 ? OpTkn->tkn : (
						iErrIs == 0 ? lval()->getTokenIfAny()->tkn :
							rval()->getTokenIfAny()->tkn )), nErr ) );
		return 0;
	}
	assert( inp.out4.value2 );
	return 1;
}
ScpAssignmentExpr::
ScpAssignmentExpr( ScpIExpr* leftexpr, const ScpToken& operToken, ScpIExpr* rightexpr )
	: ScpAxBExpr(leftexpr,operToken,rightexpr)
{
}
bool ScpAssignmentExpr::eval6( const ScpEval& inp )
{
	return ScpAxBExpr::eval6(inp);
}
