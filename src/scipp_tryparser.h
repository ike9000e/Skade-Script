#ifndef _SCIPP_TRYPARSER_H_
#define _SCIPP_TRYPARSER_H_
#include <string>
#include <vector>
#include <list>

struct ScpParse; struct ScpParseOu; class ScpIExpr; class ScpErr;
class ScpTryProgramParse; struct ScpEvalPair; struct ScpHof;
struct ScpEvalCallAndArgs; struct ScpHoc;

/// String piece class and some utility routines.
struct ScpSp {
	const char* ptr;
	int         len;
	ScpSp( const char* ptr_=0, int len_=0 );
	//
	static int         convPosToLineAndCol( const char* szOriginal, int length2, int pos, const char* szPosAlt, int* col, std::string* ouLine );
	static std::string strReplace( const std::string& inp, const std::string& oldStr, const std::string& newStr );
	static const char* strchrAdvance( const char* inp, int lnn, const char* chrs2, const char* chrs3 );
	static void        finalizeErr( ScpErr& err, const char* orig_subj, int len = -1 );
	static int         strcmpReimpl( const char* str2, const char* str3 );
	static const char* Whs, *Ops, *Wrd2, *Wrd3;
	struct PredStrStr{ std::string Str;
		PredStrStr( const std::string& Str_ ) : Str(Str_) {}
		bool operator()( const std::pair<std::string,std::string>& otherr )const;
	};
};
enum {
	SCP_ET_Empty = 1,
	SCP_ET_Op,
	SCP_ET_Wh,
	SCP_ET_Wrd,
};
struct ScpToken{
	ScpSp       tkn;
	std::string tkn2;
	int         tpe; //fe. SCP_ET_Empty
	ScpToken() : tpe(SCP_ET_Empty) {}
	ScpToken( const ScpSp& tkn_, int tpe_, int maxStrCopy = -1 );
	bool operator==( const ScpToken& otherr )const;
	struct ByTyText{ int Tpe; std::string Tkn2;
		ByTyText( int tpe_, const char* Tkn2_) : Tpe(tpe_), Tkn2(Tkn2_) {}
		bool operator()( const ScpToken& otherr )const {
			return ( Tpe == otherr.tpe && Tkn2 == otherr.tkn2 );
		}
	};
};

typedef std::list<ScpToken>        ScpLsToken;
typedef ScpLsToken::iterator       ScpTITR;
typedef ScpLsToken::const_iterator ScpTCITR;

/// Base class for all try-parsers.
/// try-parsers must reimplement their 'tryy' method that returns true on
/// successfull parsing. also, they also must return
/// an expression they produce.
class ScpITryParser {
public:
	ScpITryParser();
	virtual ~ScpITryParser();
	/// Tries parsing for expression at the input position.
	/// Returns 0 on error, which causes parsing of the program to stop.
	/// If return is 1, then either (1) parsing succeeded but expression hasn't
	/// been found or (2) there is a new expression as result of parsing.
	/// In case of (2), returned must be the expression and the end-position
	/// (see members of \ref ScpParseOu).
	/// End pos is a pos at which parsing should continue, the iterator to pos that
	/// is not part of the returned expression but part of further parsing.
	virtual bool tryy( const ScpParse& inp ) = 0;
};

/// Holds one or more random try-parsers of type \ref ScpITryParser.
class ScpTry1stOkExpr : public ScpITryParser
{
public:
	virtual ~ScpTry1stOkExpr();
	ScpTry1stOkExpr& addTryParser( ScpITryParser* inp, bool bOwnAndDelete );
	virtual bool tryy( const ScpParse& inp );
private:
	std::vector<std::pair<ScpITryParser*,bool> > Parsers2;
};

struct ScpParse {
	const ScpTCITR&      tokenbgn;   //ScpToken*
	const ScpTCITR&      tokenend;  //ScpToken*
	ScpParseOu&          out3;
	ScpTryProgramParse*  prgrm;
	ScpParse( const ScpParse& inp, const ScpTCITR& tb2, const ScpTCITR& te2, ScpParseOu& ou2 );
	ScpParse( const ScpParse& inp, const ScpTCITR& tb2 );
	ScpParse( const ScpTCITR& tb2, const ScpTCITR& te2, ScpParseOu& ou2, ScpTryProgramParse* prgrm_ );
	void errClear( const ScpErr& err2 )const;
	void clear2()const;
	ScpIExpr* extractExpr()const;
};

struct ScpParseOu {
	ScpIExpr*   expr2;
	ScpTCITR&   endded3; //ScpToken*
	ScpErr&     err;
};

struct ScpErr{
	ScpSp           errpos;
	int             tpe3;                  ///< error code - fe. \ref SCP_EE_Unknown.
	int             nErrorPos, iRow, iCol; ///< available only after finalize.
	std::string     strErrorLine;          ///< available only after finalize.
	ScpErr();
	ScpErr( const ScpSp errpos_, int tpe3_ ) : errpos(errpos_), tpe3(tpe3_), nErrorPos(0), iRow(-1), iCol(-1) {}
};
/// Flags for error type. \ref ScpErr::tpe3.
enum {
	SCP_EE_Unknown = 0,
	// on tokenizer.
	SCP_EE_UnkToken = 1, SCP_EE_CommentNoClose,
	// pseudo preprocessor, still countet as tokenizer.
	SCP_EE_NoMatchingEndif = 30, SCP_EE_UnxpctdEndif, SCP_EE_UntrmtdIfdef, SCP_EE_IfdefEof,
	SCP_EE_BadIfdefCond,
	// on parser.
	SCP_EE_EofOnParse = 100, SCP_EE_UnexpTk2, SCP_EE_UnexpTk3, SCP_EE_EofNoBrace,
	SCP_EE_UnkExpr, SCP_EE_NoSemicln, SCP_EE_UnparsableExpr, SCP_EE_EofOnAxB,
	SCP_EE_EofOnAxB2, SCP_EE_NotOperOnAxB, SCP_EE_NotOperOnAsgnmt, SCP_EE_NotOperOnAsgnmt2,
	SCP_EE_AxBExprParseFailed, SCP_EE_NoGroupCloseHere, SCP_EE_NoGroupCloseHere2, SCP_EE_GroupingNoInner,
	SCP_EE_ArgListNoAsgmntExpr, SCP_EE_NoCommaOrPClHere, SCP_EE_NoCommaOrPClHere2,
	// on evaluator.
	SCP_EE_UserEvalNull = 200, SCP_EE_UndefVar2, SCP_EE_UnknownOp, SCP_EE_UnkExpr3,
	SCP_EE_NoSuchHostObject, SCP_EE_NoSuchHostObject2, SCP_EE_NoSuchHostOper, SCP_EE_NoSuchHostObject3,
	SCP_EE_UnkErrOnUsrCall, SCP_EE_UsrCallNotImpl,
};
/**
	String-argument-replacement class.
	Uses '%a' text-tag entries as a replacement in the input format string.
	Simple text str-replace matching, escape characters not supported.
	Triggers runtime assertions when detects wrong number of tag occurences
	compared to the number of arguments inserted with the
	\ref ScpStr::a() ".a()" method(s).

	\code
		// Example
		str += *ScpStr("The item count is %a and color is %a.")
				.a( nCount );
				.a( bBlack ? "black" : "white" )
	\endcode
*/
class ScpStr{ public:
	;           ScpStr( const char* fmt );
	ScpStr&     a( int inp );
	ScpStr&     a( const std::string& inp );
	std::string operator*()const;
private:
	void replaceArg( const char* inp );
	std::string StrVal;
	static const std::string argname;
};

/**
	Abstract class that ScpScript implements.
	Used fe. by ScpAxBExpr::eval6().
	Memeber of evaluating structure, ScpEval::scope2.
*/
class ScpScope{ public:
	virtual bool evalHostObjectPair( const ScpEvalPair& inp ) = 0;
	virtual bool evalHostFunctionCall( const ScpEvalCallAndArgs& inp ) = 0;
};
/**
	Interface for building operators that get imported from the
	host environment (C++ end) into the scripts.
*/
class ScpHostOp{ public:
	virtual             ~ScpHostOp() {}
	virtual bool        callHostOperatorFnc( const ScpHof& inp ) = 0;
	virtual const char* getOpName()const = 0;
};

/// Same functionality as std::auto_ptr. (Not sure if std::auto_ptr is
/// future-proof, since marked as deprecated in the unofficial docs).
template<class T>
struct ScpAutoPtr{
	ScpAutoPtr( T* inp ) : Ptr(inp) {}
	virtual ~ScpAutoPtr()      {if(Ptr) delete Ptr; Ptr = 0;}
	T* operator()()const       {return Ptr;}
	void setPointer( T* inp )  {Ptr = inp;}
private:
	T* Ptr;
};
/// Class that must be base of all objects that can be imported to the
/// scripts.
/// \sa \ref pgHostObjects
class ScpHostObject { public:
	virtual      ~ScpHostObject() {}
	virtual bool evaluateFunctionCall( const ScpHoc& inp );
};
struct ScpHof{
	ScpHostObject*  lval;
	const char*     oper;
	ScpHostObject*  rval;
	/// Return value of the operation must be the pointer saved here.
	ScpHostObject** retval;
	/// On error, number that tells which part of the input is the problem.
	/// 0: 'lval', 1: 'rval' or -1: operator.
	int*            iErrIs;
	/// Error code, eg: \ref SCP_EE_Unknown.
	int*            eErr;
};

/**
	Parameters on host object function call.
	In particuar, parameters for ScpHostObject::evaluateFunctionCall().
	This is the structute that, on functiona call, passes
	function arguments from the script into the C++ object.
*/
struct ScpHoc {
	/// Number of arguments in the 'argvv' member.
	int                   argvc;
	/// The arguments. Pointers are likely to be upcasted (dynamic_cast)
	/// into expected user types.
	ScpHostObject*const*  argvv;
	/// Used on error. Tells which argument triggers the error or if error
	/// is caused by the object call is performed on.
	/// Value 0 indicates error caused by the host object itself.
	/// Value >= 1 instead, tells that error is caused by one of the arguments
	/// ('argvv').
	int*                  iErrIs;
	/// Error code, defaults to \ref SCP_EE_UnkErrOnUsrCall.
	int*                  eErr;
	/// Return value, if any, as result of the evaluation.
	/// Defaults to 0, which indicates no return value,
	/// aka. 'void' return value.
	/// Currently, should be set to 0. Other functonality requires
	/// more testing.
	ScpHostObject**       retval;
};

#endif // _SCIPP_TRYPARSER_H_
