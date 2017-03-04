#include "scipp_expr.h"
#include <stdio.h>
#include <assert.h>
#include "scipp_tryparser.h"
#include "scipp_value.h"

static const bool bCountStaticIExpression = 0;
static int        nStaticIExpressionCount = 0;
ScpIExpr::ScpIExpr( const char* name2, const ScpToken& tknn )
	: ExprName(name2), OpToken(new ScpToken(tknn))
	, EnclosingIfAny(0,0)
{
	if(bCountStaticIExpression)
		printf("ScpIExpr() %d\n", ++nStaticIExpressionCount );
}
ScpIExpr::~ScpIExpr()
{
	if(bCountStaticIExpression)
		printf("~ScpIExpr() %d '%s'\n", --nStaticIExpressionCount, ExprName.c_str() );
	if(OpToken){
		delete OpToken;
		OpToken = 0;
	}
	if( EnclosingIfAny.first ){
		assert( EnclosingIfAny.second );
		delete EnclosingIfAny.first;
		delete EnclosingIfAny.second;
		EnclosingIfAny = std::pair<ScpToken*,ScpToken*>(0,0);
	}else
		assert( !EnclosingIfAny.second );
}
/// Sets enclosing tokens, for reference purposes.
/// This operation is not needed from the parsing or evaluating standpoint.
/// Purpose is to have place to store extra info about expressions such as
/// grouping (ie. braces "()"), object literal string literal, etc.
ScpIExpr& ScpIExpr::setEnclosingTokens( const ScpToken& ltkn, const ScpToken& rtkn )
{
	assert( !EnclosingIfAny.first && !EnclosingIfAny.second );
	EnclosingIfAny = std::pair<ScpToken*,ScpToken*>(
			new ScpToken(ltkn), new ScpToken(rtkn) );
	return *this;
}
/// Returns enclosing tokens, if any.
/// \param bGetSecondInstead - if 0 returns left token, if 1 returns
///        right enclosing token.
const ScpToken* ScpIExpr::getEnclosingTokens( bool bGetSecondInstead )const
{
	if(bGetSecondInstead)
		return EnclosingIfAny.second;
	return EnclosingIfAny.first;
}
ScpIExpr& ScpIExpr::setOpnToken( const ScpToken& inp )
{
	*OpToken = inp;
	return *this;
}
std::string ScpIExpr::strPrint2( const ScpPrnt& inp )const
{
	assert(!"Expression string-print not implemented (ScpIExpr::strPrint2()).");
	return "";
}

const ScpToken& ScpIExpr::getOpnToken()const
{
	return *OpToken;
}
ScpPrnt& ScpPrnt::addTabs( int num )
{
	tbs += "\x20\x20\x20\x20";
	return *this;
}
ScpEval::ScpEval( const ScpEval& inp, ScpEvalOu& out_ )
	: out4(out_), scope2(inp.scope2)
{
}
ScpEval::ScpEval( ScpEvalOu& out_, ScpScope* scope_, const char* scope_name )
	: out4(out_), scope2(scope_)
{
}
ScpValue* ScpEval::extractValue()const
{
	ScpValue* v = out4.value2;
	out4.value2 = 0;
	return v;
}
void ScpEval::clear5()const
{
	if( out4.value2 ){
		delete out4.value2;
		out4.value2 = 0;
	}
	out4.err4 = ScpErr();
}
void ScpEval::errClear2( const ScpErr& err_ )const
{
	clear5();
	out4.err4 = err_;
}





