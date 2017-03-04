
#ifndef _HF_SCIPP_PROGRAM_H_
#define _HF_SCIPP_PROGRAM_H_
#include "scipp_tryparser.h"
#include "scipp_expr.h"
class ScpTryGroupingExpr;

/// Try-parser that, given begin and end, parses for all expressions.
/// Can be implemented in the root script parsing.
class ScpTryProgramParse : public ScpITryParser, public ScpIExpr
{
public:
	;                         ScpTryProgramParse();
	virtual                   ~ScpTryProgramParse();
	virtual bool              tryy( const ScpParse& inp );
	virtual bool              eval6( const ScpEval& inp );
	virtual std::string       strPrint2( const ScpPrnt& inp )const;
	void                      setOriginalScript( const char* inp, int len );
	void                      reinit2();
	int                       getExpressionCount()const {return (int)Expressions2.size();}
	const ScpTryGroupingExpr* getTryGroupingExpr()const {return TryGroupingExpr;}
	ScpTryGroupingExpr*       getTryGroupingExpr() {return TryGroupingExpr;}
private:
	std::string               getLineAndCol( const char* szPos )const;
	void                      clear4();
private:
	ScpTry1stOkExpr*           Parsers3;
	std::vector<ScpIExpr*>     Expressions2;
	std::pair<const char*,int> OrigScrIfAny;
	ScpTryGroupingExpr*        TryGroupingExpr;
};

#endif //_HF_SCIPP_PROGRAM_H_
