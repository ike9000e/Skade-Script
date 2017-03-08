
#include "scipp_axb_e.h"
#include "scipp_tryparser.h"
#include "scipp_value.h"
#include <assert.h>
#include <stdio.h>

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
/**
	Constructor.
	\param argvOwn - list of expressions that the object will take owneship of,
	                 ie. they will be deleted on this object destruction.
*/
ScpArgListExpr::
ScpArgListExpr( const ScpToken& opn2, const ScpToken& close2 )
	: ScpIExpr( "ScpArgListExpr", opn2 )
{
	setEnclosingTokens( opn2, close2 );
}
void ScpArgListExpr::addArgExpressin( ScpIExpr* expr, bool bOwnAndDel )
{
	assert(bOwnAndDel);
	ArgList.push_back( expr );
}
ScpArgListExpr::~ScpArgListExpr()
{
	std::vector<ScpIExpr*>::iterator a;
	for( a = ArgList.begin(); a != ArgList.end(); ++a ){
		delete *a;
	}
	ArgList.clear();
}
ScpCallExpr::
ScpCallExpr( const ScpToken& opn2, ScpIExpr* LeftExpr_, ScpArgListExpr* ArgsExpr_ )
	: ScpIExpr( "ScpCallExpr", opn2 ), LeftExpr(LeftExpr_), ArgsExpr(ArgsExpr_)
{
}
ScpCallExpr::~ScpCallExpr()
{
	delete LeftExpr;
	delete ArgsExpr;
}
std::string ScpCallExpr::strPrint2( const ScpPrnt& inp )const
{
	ScpPrnt in2( inp );
	in2.addTabs(1);
	std::string z;
	z += *ScpStr("%a""CAL: L:[\n%a\n%a] R:[\n%a\n%a]")
			.a( inp.tbs )
			.a( LeftExpr->strPrint2( in2 ) )
			.a( inp.tbs )
			.a( ArgsExpr->strPrint2( in2 ) )
			.a( inp.tbs );
	return z;
}
std::string ScpArgListExpr::strPrint2( const ScpPrnt& inp )const
{
	ScpPrnt in2( inp );
	in2.addTabs(1);
	std::string z;
	std::vector<ScpIExpr*>::const_iterator a; int i;
	z += *ScpStr("%a""ARGs-%a: [")
			.a( inp.tbs )
			.a( (int)ArgList.size() );
	for( i=0, a = ArgList.begin(); a != ArgList.end(); ++a, i++ ){
		z += *ScpStr(" ""arg-%a: [\n%a\n%a]")
				.a( i+1 )
				.a( (**a).strPrint2(in2) )
				.a( inp.tbs );
	}
	z += "]";
	return z;
}
bool ScpArgListExpr::eval6( const ScpEval& inp )
{
	assert(!"ScpArgListExpr ment to be never evalueated.");
	return 0;
}
ScpIExpr* ScpArgListExpr::getArg( int idx )
{
	assert( idx < (int)ArgList.size() );
	return ArgList[idx];
}
bool ScpCallExpr::eval6( const ScpEval& inp )
{
	assert( LeftExpr && ArgsExpr );
	if( !LeftExpr->eval6( inp ) )
		return 0;
	std::vector<ScpNamedVal>::iterator c;
	std::vector<ScpNamedVal> argvals2;
	//std::vector<ScpAutoPtr<ScpValue> > argvals3;
	ScpValue* val2 = inp.extractValue();
	assert( val2 );
	ScpNamedVal nv = { val2->getVarname(), val2 };
	argvals2.push_back( nv );
	{
		for( int i=0; i < ArgsExpr->getArgCount(); i++ ){
			ScpIExpr* expr = ArgsExpr->getArg(i);
			ScpEvalOu ou2( inp.out4.err4 );
			ScpEval in2( inp, ou2 );
			if( !expr->eval6( in2 ) )
				return 0;
			ScpValue* val = in2.extractValue();
			assert( val );
			ScpNamedVal nv2 = { val->getVarname(), val };
			argvals2.push_back( nv2 );
		}
	}
	int iErrIs = -1, nErr = -1;
	ScpEvalCallAndArgs evca = { &argvals2, &iErrIs, &nErr, &inp.out4.value2, LeftExpr->getOpnToken(), };
	if( !inp.scope2->evalHostFunctionCall( evca ) ){
		assert( !inp.out4.value2 );
		inp.errClear2( ScpErr( LeftExpr->getOpnToken().tkn, nErr ) );
		for( c = argvals2.begin(); c != argvals2.end(); ++c )
			delete c->val3;
		return 0;
	}
	assert( inp.out4.value2 );
	for( c = argvals2.begin(); c != argvals2.end(); ++c )
		delete c->val3;
	return 1;
}





