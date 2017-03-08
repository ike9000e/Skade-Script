#ifndef _SCIPP_EXPR_H_
#define _SCIPP_EXPR_H_
#include <string>
#include <vector>
struct ScpEval; class ScpValue; class ScpToken; struct ScpPrnt;
struct ScpEvalOu; struct ScpErr; class ScpScope;
class ScpHostObject;

/// Base class for all expressions.
class ScpIExpr {
public:
	;                     ScpIExpr( const char* name2, const ScpToken& );
	virtual               ~ScpIExpr();
	virtual bool          eval6( const ScpEval& inp ) = 0;
	virtual std::string   strPrint2( const ScpPrnt& inp )const;
	void                  setExprName( const char* sz ) {ExprName = sz;}
	const char*           getExprName()const {return ExprName.c_str();}
	ScpIExpr&             setOpnToken( const ScpToken& inp );
	const ScpToken&       getOpnToken()const;
	ScpIExpr&             setEnclosingTokens( const ScpToken& ltkn, const ScpToken& rtkn );
	const ScpToken*       getEnclosingTokens( bool bGetSecondInstead )const;
private:
	std::string ExprName;
	ScpToken* OpToken;
	std::pair<ScpToken*,ScpToken*> EnclosingIfAny;
};
/// Params for strPrint2() methods.
struct ScpPrnt{
	std::string tbs;
	ScpPrnt( const std::string& tbs_ ) : tbs(tbs_) {}
	ScpPrnt& addTabs( int num = 1 );
};
/// Params on evaluate. Eg. \ref ScpIExpr::eval6() ".eval6()".
struct ScpEval {
	ScpEvalOu&     out4;
	ScpScope*      scope2;
	;          ScpEval( const ScpEval& inp, ScpEvalOu& out_ );
	;          ScpEval( ScpEvalOu& out_, ScpScope* scope_, const char* scope_name );
	ScpValue*  extractValue()const;
	void       clear5()const;
	void       errClear2( const ScpErr& err_ )const;
};
/// Output params on evaluate. Eg. \ref ScpIExpr::eval6() ".eval6()".
struct ScpEvalOu{
	ScpValue*    value2;
	ScpErr&      err4;
	ScpEvalOu( ScpErr& err_ ) : value2(0), err4(err_) {}
};
struct ScpEvalPair{
	const char*     nameA, *nameB;
	const ScpToken& oper;
	int*            iErrIs, *eErr;
	/// Output value, result of the evaluation.
	ScpValue**      val2;
	ScpValue*       lval, *rval;
};
struct ScpNamedVal{
	std::string name3;
	ScpValue*   val3;
};
struct ScpEvalCallAndArgs{
	/// Call value and arg list. First value is the call expression itself.
	/// Values starting at index 1 construct argument list.
	std::vector<ScpNamedVal>* argvals;
	int*                      iErrIs, *eErr;
	/// Output value, result of the evaluation.
	ScpValue**                val4;
	const ScpToken&           tokenAt;
};

#endif // _SCIPP_EXPR_H_


