
#include "scipp_axb_p.h"
#include <algorithm>
#include <assert.h>
#include "scipp_axb_e.h"
#include "scipp_program.h"

ScpTryIdentifierExpr scpTryIdentifierExpr;
ScpTryAxBExpr        scpTryMulExpr("*","/","%");
ScpTryAdditiveExpr   scpTryAdditiveExpr;
ScpTryAsgmntExpr     scpTryAsgmntExpr;
ScpTryPrimaryExpr    scpTryPrimaryExpr;

bool ScpTryIdentifierExpr::tryy( const ScpParse& inp )
{
	if( inp.tokenbgn->tpe == SCP_ET_Wrd ){
		ScpIdentifierExpr* expr = new ScpIdentifierExpr( *inp.tokenbgn );
		expr->setOpnToken( *inp.tokenbgn );
		inp.out3.expr2   = expr;
		inp.out3.endded3 = inp.tokenbgn;// + 1;
		std::advance( inp.out3.endded3, 1 );
		return 1;
	}
	return 1;
}
bool ScpTryPrimaryExpr::tryy( const ScpParse& inp )
{
	assert( !inp.out3.expr2 );
	if( !scpTryIdentifierExpr.tryy(inp) ){ // ScpTryIdentifierExpr*
		return 0;
	}else if( inp.out3.expr2 ){
		return 1;
	}
	ScpTryGroupingExpr* xxx = inp.prgrm->getTryGroupingExpr();
	assert(xxx);
	if( !xxx->tryy(inp) ){ // ScpTryGroupingExpr*
		return 0;
	}else if( inp.out3.expr2 ){
		return 1;
	}//*/
	return 1;
}

ScpTryAxBExpr::
ScpTryAxBExpr( const char* op1,const char* op2, const char* op3, const char* op4 )
{
	Ops.push_back(op1);
	if( op2 && *op2 )
		Ops.push_back( op2 );
	if( op3 && *op3 )
		Ops.push_back( op3 );
	if( op4 && *op4 )
		Ops.push_back( op4 );
}
ScpITryParser* ScpTryAxBExpr::getLeftAxBExprParser()
{
	/// \todo fully quantified is to start at unary expression,
	///       ie. if multiplicatve expression here (not derived class). eg:
	///       UnaryExpression->...-> LeftHandSideExpression->...
	///       -> MemberExpression -> PrimaryExpression ...
	return &scpTryPrimaryExpr; // ScpTryPrimaryExpr*
}
ScpTryAdditiveExpr::ScpTryAdditiveExpr()
	: ScpTryAxBExpr("+","-")
{
}
ScpITryParser* ScpTryAdditiveExpr::getLeftAxBExprParser()
{
	return &scpTryMulExpr;
}
ScpTryAsgmntExpr::ScpTryAsgmntExpr()
	: ScpTryAxBExpr("=")
{
}
ScpITryParser* ScpTryAsgmntExpr::getLeftAxBExprParser()
{
	/// \todo more exhaustive order, eg:
	///       LeftHandSideExpression .. PrimaryExpression .. Identifier
	return &scpTryAdditiveExpr;
}
ScpTryGroupingExpr::ScpTryGroupingExpr( const ScpTryProgramParse* program2 )
{
	// this is only to ensure that grouping expr parser is managed
	// by the 'ScpTryProgramParse'. reason: grouping operator
	// stores, as a memeber variable, current token iterator position to prevent
	// infinite recursion.
	program2->getTryGroupingExpr();
}
ScpITryParser* ScpTryGroupingExpr::getInnerExprParser()
{
	/// \todo fully exhaustive would be to parse for the 'Expression'.
	return &scpTryAsgmntExpr;
}
bool ScpTryAxBExpr::tryy( const ScpParse& inp )
{
	if( inp.tokenbgn == inp.tokenend )
		return 1;
	if( !getLeftAxBExprParser()->tryy( inp ) ) // UnaryExpression->...->PrimaryExpression.
		return 0;
	if( !inp.out3.expr2 )
		return 1;
	ScpTCITR op = inp.out3.endded3;
	if( op == inp.tokenend )
		return 1;
	if( op->tpe != SCP_ET_Op )
		return 1;
	if( op->tkn2 == ";" ){
		inp.out3.endded3 = ++op;// + 1;
		return 1;
	}
	std::vector<std::string>::const_iterator b;
	b = std::find( Ops.begin(), Ops.end(), op->tkn2.c_str() );
	if( b == Ops.end() ){ // if no operator match.
		//printf("n:%d [%s] b:%d\n", (int)Ops.size(), Ops[0].c_str(),
		//		!!dynamic_cast<ScpTryAdditiveExpr*>( this ) );
		assert( inp.out3.expr2 );
		return 1;
	}
	ScpTCITR a = op; ++a;
	if( a == inp.tokenend ){ // if EOF after operator.
		inp.errClear( ScpErr( op->tkn, SCP_EE_EofOnAxB ) );
		return 0;
	}
	ScpIExpr* exprA = inp.out3.expr2; // have the left-side expression, store it.
	inp.out3.expr2 = 0;

	// Note: recursive 'tryy' call. search for the right-side expression.
	ScpParse in2( inp, a, inp.tokenend, inp.out3 );
	if( !this->tryy( in2 ) ){
		delete exprA;
		return 0;
	}
	if( !inp.out3.expr2 ){
		delete exprA;
		//printf("SCP_EE_AxBExprParseFailed:%d\n", SCP_EE_AxBExprParseFailed );
		inp.errClear( ScpErr( a->tkn, SCP_EE_AxBExprParseFailed ) );
		return 0;
	}
	ScpIExpr* exprB = inp.out3.expr2; // have the right-side expression, store it.
	inp.out3.expr2 = 0;
	//
	ScpAxBExpr* mulexpr = 0;//new ScpAxBExpr( exprA, *op, exprB );
	if( op->tkn2 != "=" ){
		mulexpr = new ScpAxBExpr( exprA, *op, exprB );
	}else{
		mulexpr = new ScpAssignmentExpr( exprA, *op, exprB );
	}
	mulexpr->setOpnToken( *inp.tokenbgn );
	//
	inp.out3.expr2 = mulexpr;  //inp.out3.endded3 = inp.tokenbgn + 1;
	return 1;
}
/**
	Tries parsing for the grouping expression.
	Note: implementation of parsing of the grouping expression here must
	be done in the respect to the assignment expression, which is AxB type exression.
	that is, there is a certain requirement on recursive calls that is different in
    AxB type expressions vs grouping type expressions.

	parsing must take into account two paths:
    (1) parsing if there is a parenthese open character, and
    (2) parsing if there was no parenthese open character.
    in case of (1), after inner parsing is done, assertion must be made that
    there is the parenthese close character that follows the expression.

	NOTE: checking for recursion only possible in non-overloaded methods
	_     ('LastTokenAt'). when storing member value, try-parser must
	_     be local to program, ie. not static or global.
*/
bool ScpTryGroupingExpr::tryy( const ScpParse& inp )
{
	if( LastTokenAt == inp.tokenbgn )
		return 1;
	LastTokenAt = inp.tokenbgn;
	if( inp.tokenbgn == inp.tokenend )
		return 1;
	ScpTCITR a = inp.tokenbgn;
	if( a->tpe != SCP_ET_Op || a->tkn2 != "\x28" ){ // 0x28: parentheses open character.
		// dispatch the case when there is no parenthese open, not a grouping expr.
		return getInnerExprParser()->tryy( inp );
		//return 1;
	}else{
		ScpTCITR b = a; ++b;// + 1;
		if( b == inp.tokenend ){
			inp.errClear( ScpErr( a->tkn, SCP_EE_EofOnParse ) );
			return 0;
		}
		ScpParse in2( inp, b, inp.tokenend, inp.out3 );
		if( !getInnerExprParser()->tryy( in2 ) ) // ScpTryAsgmntExpr or 'Expression'
			return 0;
		if( !inp.out3.expr2 ){
			inp.errClear( ScpErr( a->tkn, SCP_EE_GroupingNoInner ) );
			return 0;
		}
		b = inp.out3.endded3;
		if( b == inp.tokenend ){
			//printf("SCP_EE_NoGroupCloseHere2:%d\n", SCP_EE_NoGroupCloseHere2 );
			inp.errClear( ScpErr( a->tkn, SCP_EE_NoGroupCloseHere2 ) );
			return 0;
		}
		//printf("[%s]\n", b->tkn2.c_str() );
		// make sure have the matching brace close character.
		if( b->tpe != SCP_ET_Op || b->tkn2 != "\x29" ){ // 0x29: parenthese close character.
			assert( inp.out3.expr2 );
			//printf("SCP_EE_NoGroupCloseHere:%d\n", SCP_EE_NoGroupCloseHere );
			inp.errClear( ScpErr( b->tkn, SCP_EE_NoGroupCloseHere ) );
			return 0;
		}
		inp.out3.expr2 = new ScpGroupingExpr( inp.out3.expr2, *a, *b );
		inp.out3.endded3 = b;// + 1;
		inp.out3.endded3++;
		return 1;
	}
}






