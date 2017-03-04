
#ifndef _HF_SCIPP_MUL_A_H_
#define _HF_SCIPP_MUL_A_H_
#include "scipp_tryparser.h"

class ScpTryProgramParse;

/*
	Expression -> AssignmentExpression->...-> BitwiseANDExpression
	->...-> EqualityExpression->...-> AdditiveExpression
	-> MultiplicativeExpression-> UnaryExpression-> PostfixExpression
	-> LeftHandSideExpression->...-> MemberExpression -> PrimaryExpression

	MultiplicativeExpression : See section 11.5
		UnaryExpression
		MultiplicativeExpression * UnaryExpression
		MultiplicativeExpression / UnaryExpression
		MultiplicativeExpression % UnaryExpression

	Note: in simplified parsing, UnaryExpression is reduced to a PrimaryExpression,
	_     whitch in turn, in even more simplier parsing, can be
	_     just identifier and literals.
*/
class ScpTryAxBExpr : public ScpITryParser {
public:
	;                       ScpTryAxBExpr( const char* op1="*", const char* op2=0, const char* op3=0, const char* op4=0 );
	virtual                 ~ScpTryAxBExpr() {}
	virtual bool            tryy( const ScpParse& inp );
private:
	virtual ScpITryParser*  getLeftAxBExprParser();
private:
	std::vector<std::string> Ops;
};
/*
	AdditiveExpression : See section 11.6
		MultiplicativeExpression
		AdditiveExpression + MultiplicativeExpression
		AdditiveExpression - MultiplicativeExpression
*/
class ScpTryAdditiveExpr : public ScpTryAxBExpr {
public:
	ScpTryAdditiveExpr();
private:
	virtual ScpITryParser* getLeftAxBExprParser();
};
/*
	PrimaryExpression : See section 11.1
		this
		Identifier
		Literal
		ArrayLiteral
		ObjectLiteral
		( Expression )
*/
class ScpTryPrimaryExpr : public ScpITryParser {
public:
	virtual ~ScpTryPrimaryExpr() {}
	virtual bool          tryy( const ScpParse& inp );
};
class ScpTryIdentifierExpr : public ScpITryParser
{
public:
	virtual bool tryy( const ScpParse& inp );
};
/*
	AssignmentExpression : See section 11.13
		ConditionalExpression
		LeftHandSideExpression AssignmentOperator AssignmentExpression
*/
class ScpTryAsgmntExpr : public ScpTryAxBExpr
{
public:
	ScpTryAsgmntExpr();
	virtual ScpITryParser* getLeftAxBExprParser();
};
/*
	Grouping expression using parentheses.
	The '( Expression )' expression.

	11.1.6 The Grouping Operator
	The production PrimaryExpression : ( Expression ) is evaluated as follows:
		1. Evaluate Expression. This may be of type Reference.
		2. Return Result(1).

	ref: CTryGroupingOpExpr, ETK_PN_PARENTHESES_OPEN, jsc_GlobTryTheExpr
*/
class ScpTryGroupingExpr : public ScpITryParser {
public:
	ScpTryGroupingExpr( const ScpTryProgramParse* program2 );
	virtual bool tryy( const ScpParse& inp );
	virtual ScpITryParser* getInnerExprParser();
private:
	ScpTCITR LastTokenAt;
};

#endif // _HF_SCIPP_MUL_A_H_
