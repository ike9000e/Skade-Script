
#ifndef _HF_SCIPP_MUL_B_H_
#define _HF_SCIPP_MUL_B_H_
#include "scipp_expr.h"
#include <vector>

class ScpIdentifierExpr : public ScpIExpr {
public:
	;                     ScpIdentifierExpr( const ScpToken& tkn );
	virtual               ~ScpIdentifierExpr() {}
	virtual bool          eval6( const ScpEval& in );
	virtual std::string   strPrint2( const ScpPrnt& inp )const;
};

class ScpAxBExpr : public ScpIExpr {
public:
	;                   ScpAxBExpr( ScpIExpr* leftexpr, const ScpToken& operToken, ScpIExpr* rightexpr );
	virtual             ~ScpAxBExpr();
	virtual bool        eval6( const ScpEval& inp );
	virtual std::string strPrint2( const ScpPrnt& inp )const;

private:
	ScpIExpr* LeftExpr, *RightExpr;
	ScpToken* OpTkn;
};
class ScpAssignmentExpr : public ScpAxBExpr {
public:
	ScpAssignmentExpr( ScpIExpr* leftexpr, const ScpToken& operToken, ScpIExpr* rightexpr );
	virtual bool eval6( const ScpEval& inp );
};

class ScpGroupingExpr : public ScpIExpr {
public:
	;                   ScpGroupingExpr( ScpIExpr* innerExpr, const ScpToken& opn2, const ScpToken& close2 );
	virtual             ~ScpGroupingExpr();
	virtual bool        eval6( const ScpEval& inp );
	virtual std::string strPrint2( const ScpPrnt& inp )const;
private:
	ScpIExpr* InnerExpr;
};

class ScpArgListExpr : public ScpIExpr {
public:
	;                   ScpArgListExpr( const ScpToken& opn2, const ScpToken& close2 );
	virtual             ~ScpArgListExpr();
	virtual bool        eval6( const ScpEval& inp );
	virtual std::string strPrint2( const ScpPrnt& inp )const;
	void                addArgExpressin( ScpIExpr* exprr, bool bOwnAndDel );
	int                 getArgCount()const {return (int)ArgList.size();}
	ScpIExpr*           getArg( int idx );
private:
	std::vector<ScpIExpr*> ArgList;
};
class ScpCallExpr : public ScpIExpr {
public:
	;                   ScpCallExpr( const ScpToken& opn2, ScpIExpr* LeftExpr_, ScpArgListExpr* ArgsExpr_ );
	virtual             ~ScpCallExpr();
	virtual bool        eval6( const ScpEval& inp );
	virtual std::string strPrint2( const ScpPrnt& inp )const;
private:
	ScpIExpr* LeftExpr;//, *ArgsExpr;
	ScpArgListExpr* ArgsExpr;
};

#endif // _HF_SCIPP_MUL_B_H_
