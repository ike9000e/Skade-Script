
#ifndef _SCIPP_SCRIPT_H_
#define _SCIPP_SCRIPT_H_
#include "scipp_tryparser.h"

class ScpTryProgramParse;

/**
	Main class in the scripting engine.
	Processing of scripts is done in two steps: parsing and evaluating.
	Once the object is initialized, methods parse2() and eval2() can be
	used to process the script in the default way, in these two steps.
	Additional parsing methods are provided so that, for example, both steps can be
	combined into one.

	All parsing methods return bolean value that indicates whenever
	operation succeded or failed.
	Method \ref showMeTheError2() (or alternate \ref showMeTheError3())
	can be used to retrieve textual information, such as: type of error,
	row and column number, or the whole line that the error occured at.
*/
class ScpScript : public ScpScope {
	struct SIfs;
public:
	;                ScpScript( const std::string& script_ );
	;                ScpScript( const char* script_, int len, bool bCreateInternalCopy );
	virtual          ~ScpScript();
	bool             parseAndEval();
	bool             parse2();
	bool             parse3( int flags_, ScpErr* err );
	bool             eval2();
	bool             eval3( int flags4, ScpErr* err );
	void             showMeTheError2()const;
	std::string      showMeTheError3( bool bShortOutput )const;
	//
	void             addHostObject( const char* objectname, ScpHostObject* hobject, bool bOwnAndDelete );
	void             addHostOperator( ScpHostOp* inp, bool bOwnAndDel );
	ScpScript&       clear2();
	ScpScript&       clear3( int flags3 );
	ScpScript&       setCondDef( const char* dname, bool val );
	void             setOptions( int flags2 );
private:
	virtual bool     evalHostObjectPair( const ScpEvalPair& inp );
	bool             tokenize2( ScpErr& err );
	bool             eval9( int flags4, ScpErr& err );
	bool             preprocessCondDefs( ScpErr& err );
	bool             scanUntilSequenceRecv( int dpth, size_t& ioNum, const ScpTITR& bgn, const ScpTITR& endd, int& mode3, SIfs& ifs2, ScpTITR& ended2, ScpErr& err, int mode4 );
	bool             applyCondDefs( ScpErr& err );
private:
	struct SIfdef{
		ScpTITR htg, wrd, cond2;
		int tpe4; // fe. EIDT_Ifdef.
		SIfdef( int tpe4_, ScpTITR htg_, ScpTITR wrd_ ) : htg(htg_), wrd(wrd_), tpe4(tpe4_) {}
	};
	struct SIfs{
		bool bInvalidated;
		std::vector<SIfdef> ifdefs2;
		SIfs() : bInvalidated(0) {}
	};
	enum{ EIDT_Ifdef, EIDT_Endif, EIDT_IfNdef, };
	enum{ EIDM_Ifs, EIDM_Els, };
	enum{ ESS_HaveLocalScrCopy = 0x1, };
	struct SHo{
		std::string    name2;
		ScpHostObject* hobj;
		bool           bOwn2;
		SHo( const char* name_, ScpHostObject* hobj_=0, bool bOwn2_=0 ) : name2(name_), hobj(hobj_), bOwn2(bOwn2_) {}
		bool operator()( const SHo& otherr )const {return name2 == otherr.name2;}
	};
	struct SHe{
		ScpHostOp*        hoper;
		bool              bOwn;
		struct ByOper{ const char* szOper;
			ByOper( const char* szOper_ ) : szOper(szOper_) {}
			bool operator()( const SHe& otherr )const;
		};
	};
	std::vector<std::pair<std::string,std::string> > Defs; // cond-defs for pseudo preprocessor.
	int                           Flags2, Flags3; // fe. ESS_HaveLocalScrCopy.
	std::string                   ScriptCopy;
	std::pair<const char*,int>    ScriptCStr;
	ScpLsToken                    Tokens2; //std::list<ScpToken>
	ScpErr                        LastError;
	ScpTryProgramParse*           Prgrm;
	ScpLsToken                    myIfdef, myEndif, myIfNdef;
	std::vector<SIfs>             IfDefBlocks;
	std::vector<SHo>              HostObjs;
	std::vector<SHe>              HostEnv2;
};
/// Flags for ScpScript::eval3().
enum{
	/**
		Can be set at the evaluation step to perform required parsing automatically.
		Normally, user calls ScpScript::parse2() then ScpScript::eval2(),
		both functions manually.
		Using this flag enables parsing and evaluating to be done in one step,
		durning ScpScript::eval2() call.
		\sa \ref pgParseEval
	*/
	SCP_AF_AutoParse = 0x1,
};
/// Flags for ScpScript::clear3().
enum{
	SCP_CF_ClearParser = 0x2,
	SCP_CF_ClearStack = 0x4,
	SCP_CF_ClearCondDefs = 0x8,
	SCP_CF_ClearAll = SCP_CF_ClearParser|SCP_CF_ClearStack|SCP_CF_ClearCondDefs,
};
/// Flags for ScpScript::setOptions().
enum{
	/// Shows tokens by printing them to the console STDOUT durning parsing.
	SCP_SF_ShowTokens = 0x1,
	/// Similar to \ref SCP_SF_ShowTokens.
	SCP_SF_ShowExpressions = 0x2,
	/// Similar to \ref SCP_SF_ShowTokens.
	SCP_SF_ShowCondDefs = 0x4,
	/// If set, error line is not trimmed if too long.
	/// If not set, when showing the error with
	/// \ref ScpScript::showMeTheError3() ".showMeTheError3()",
	/// the original source code line the error happened at,
	/// is trimmed down from the right, in case if it ends up being
	/// longer than some predefined length.
	SCP_SF_NoErrLineRTrim = 0x8,
};

#endif //_SCIPP_SCRIPT_H_
