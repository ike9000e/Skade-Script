#include "scipp_tryparser.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "scipp_expr.h"

const char* ScpSp::Whs = "\r\n\t\x20";
const char* ScpSp::Ops = ";*/%=+-()#,";
const char* ScpSp::Wrd2 = "_$abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char* ScpSp::Wrd3 = "0123456789";

static const bool bCountStaticParser = 0;
static int        nCountStaticParser = 0;
ScpITryParser::ScpITryParser()
{
	if(bCountStaticParser)
		printf("ScpIExpr() %d\n", ++nCountStaticParser );
}
ScpITryParser::~ScpITryParser()
{
	if(bCountStaticParser)
		printf("~ScpIExpr() %d\n", --nCountStaticParser );
}


ScpErr::ScpErr() : tpe3(SCP_EE_Unknown), nErrorPos(0), iRow(-1), iCol(-1)
{
}
ScpTry1stOkExpr::~ScpTry1stOkExpr()
{
	std::vector<std::pair<ScpITryParser*,bool> >::const_iterator a;
	for( a = Parsers2.begin(); a != Parsers2.end(); ++a ){
		if(a->second)
			delete a->first;
	}
}
ScpTry1stOkExpr& ScpTry1stOkExpr::
addTryParser( ScpITryParser* inp, bool bOwnAndDelete )
{
	Parsers2.push_back( std::pair<ScpITryParser*,bool>(inp,bOwnAndDelete) );
	return *this;
}
bool ScpTry1stOkExpr::tryy( const ScpParse& inp )
{
	assert( !Parsers2.empty() );
	assert( !inp.out3.expr2 );
	std::vector<std::pair<ScpITryParser*,bool> >::const_iterator a;
	for( a = Parsers2.begin(); a != Parsers2.end(); ++a ){
		if( !a->first->tryy(inp) )
			return 0;
		if( inp.out3.expr2 )
			return 1;
	}
	return 1;
}
ScpParse::
ScpParse( const ScpParse& inp, const ScpTCITR& tb2, const ScpTCITR& te2, ScpParseOu& ou2 )
	: tokenbgn(tb2), tokenend(te2), out3(ou2), prgrm(inp.prgrm)
{
}
ScpParse::
ScpParse( const ScpParse& inp, const ScpTCITR& tb2 )
	: tokenbgn(tb2), tokenend(inp.tokenend), out3(inp.out3), prgrm(inp.prgrm)
{
}
ScpParse::
ScpParse( const ScpTCITR& tb2, const ScpTCITR& te2, ScpParseOu& ou2, ScpTryProgramParse* prgrm_ )
	: tokenbgn(tb2), tokenend(te2), out3(ou2), prgrm(prgrm_)
{
}
void ScpParse::errClear( const ScpErr& err2 )const
{
	//if( out3.expr2 ){
	//	delete out3.expr2;
	//	out3.expr2 = 0;
	//}
	clear2();
	out3.err = err2;
}
void ScpParse::clear2()const
{
	if( out3.expr2 ){
		delete out3.expr2;
		out3.expr2 = 0;
	}
	out3.err = ScpErr();
}
ScpIExpr* ScpParse::extractExpr()const
{
	ScpIExpr* x = out3.expr2;
	out3.expr2 = 0;
	return x;
}

const std::string ScpStr::argname = "%a";
ScpStr::ScpStr( const char* fmt ) : StrVal(fmt)
{
}
void ScpStr::replaceArg( const char* inp )
{
	std::string::size_type pos = 0u;
	if( (pos = StrVal.find( argname, pos)) != std::string::npos ){
		StrVal.replace( pos, argname.length(), inp );
	}else{
		assert(!"ScpStr: couldnt replace argument entry. Too few '%a' entries?");
	}
}
ScpStr& ScpStr::a( int inp )
{
	char bfr[128];
	sprintf(bfr,"%d", inp );
	replaceArg( bfr );
	return *this;
}
ScpStr& ScpStr::a( const std::string& inp )
{
	replaceArg( inp.c_str() );
	return *this;
}
std::string ScpStr::operator*()const
{
	std::string::size_type pos = 0u;
	if( (pos = StrVal.find( argname, pos)) != std::string::npos ){
		assert(!"ScpStr: Not all args replaced. Too many '%a' entries?");
	}
	return StrVal;
}
ScpSp::
ScpSp( const char* ptr_, int len_ )
	: ptr(ptr_), len(len_)
{
	len = ( len == -1 ? strlen(ptr) : len );
}
/// Calculates line and column number given position in the original string.
/// Returned line number and column number are both 0-based.
/// \param szOriginal - original c-string the position 'pos' was from.
/// \param length2  - length of 'szOriginal', if -1 then null terminated.
/// \param pos - 0-based position.
/// \param szPosAlt - if 'pos' set to -1, alternate position as a pointer inside 'szOriginal'.
/// \param col - optional, output. column number.
/// \param ouLine - optional, output. the whole line extracted from the 'szOriginal'.
/// \return 0-based line number, aka row number.
int ScpSp::
convPosToLineAndCol( const char* szOriginal, int length2, int pos,
				const char* szPosAlt, int* col, std::string* ouLine )
{
	//const char* szPosAlt = 00;
	length2 = ( length2 == -1 ? strlen(szOriginal) : length2 );
	if( pos == -1 ){
		assert( szPosAlt >= szOriginal );
		assert( szPosAlt < szOriginal+length2 );
		pos = std::distance( szOriginal, szPosAlt );
	}
	int i, cnLn = 0, iPrv = 0;
	for( i=0; i<length2; i++ ){
		if( szOriginal[i] == '\n' ){
			if( pos < i )
				break;
			iPrv = i;
			cnLn++;
		}
	}
	int iPrvAdjusated = ( iPrv && szOriginal[iPrv] == '\n' ? iPrv+1 : iPrv );
	if(col)
		*col = ( pos - iPrvAdjusated );
	if( ouLine ){
		int iBgn = iPrvAdjusated;
		int iEnd = i;//( i && szOriginal[i] == '\n' ? i-0 : i );
		ouLine->assign( &szOriginal[iBgn], iEnd-iBgn );
	}
	return cnLn;
}
std::string ScpSp::
strReplace( const std::string& inp, const std::string& oldStr, const std::string& newStr )
{
	std::string outp = inp;
	std::string::size_type pos = 0u;
	while( (pos = outp.find(oldStr, pos)) != std::string::npos ){
		outp.replace( pos, oldStr.length(), newStr );
		pos += newStr.length();
	}
	return outp;
}
bool ScpSp::PredStrStr::operator()( const std::pair<std::string,std::string>& otherr )const
{
	return Str == otherr.first;
}
/// Advances string characters using strchr method.
/// only 'chrs2' has the set that is tried at the start pos.
const char* ScpSp::
strchrAdvance( const char* inp, int lnn, const char* chrs2, const char* chrs3 )
{
	lnn = ( lnn == -1 ? strlen(inp) : lnn ); int i;
	for( i=0; i<lnn; i++ ){
		if( strchr( chrs2, inp[i] ) || ( i && chrs3 && strchr( chrs3, inp[i] ) ) ){
		}else
			break;
	}
	return &inp[i];
}
void ScpSp::finalizeErr( ScpErr& err, const char* orig_subj, int len )
{
	assert( err.errpos.ptr );
	int pos = err.errpos.ptr - orig_subj;
	err.iRow = ScpSp::convPosToLineAndCol( orig_subj, len, pos, 0, &err.iCol, &err.strErrorLine );
}
int ScpSp::strcmpReimpl( const char* str2, const char* str3 )
{
	return strcmp(str2,str3);
}
ScpToken::
ScpToken( const ScpSp& tkn_, int tpe_, int maxSave )
	: tkn(tkn_), tpe(tpe_)
{
	tkn2.assign( tkn.ptr, (maxSave >= 0 ? std::min(tkn.len,maxSave) : tkn.len ) );
}
bool ScpToken::operator==( const ScpToken& otherr )const
{
	if( tpe == otherr.tpe && tkn.len == otherr.tkn.len ){
		if( !strncmp( tkn.ptr, otherr.tkn.ptr, tkn.len ) )
			return 1;
	}
	return 0;
}
/**
	Called when, in turn, function call is performed on the host object
	from the script environment.

	\param inp - See description of ScpHoc structure.

	\return Return value indicates whenever script evaluating should
	continue. Returning 0 stops evauating process of the entire script.

	Defaul implementaion returns 0 and causes evaluating to stop.
	A behaviour as if it was illegal to "call" the object from the script.

	Currently, funtion call should not be part of other expression,
	this feature requires more testing.
*/
bool ScpHostObject::evaluateFunctionCall( const ScpHoc& inp )
{
	*inp.iErrIs = 0;
	*inp.eErr = SCP_EE_UsrCallNotImpl;
	return 0;
}







