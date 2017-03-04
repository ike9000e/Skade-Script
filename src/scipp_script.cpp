#include "scipp_script.h"
#include <string.h>
#include <assert.h>
#include <algorithm>
#include <stdio.h>
#include "scipp_program.h"
#include "scipp_expr.h"
#include "scipp_value.h"
#include "scipp_hostobj.h"

/// Constructs script object.
/// \param script_ - input script, internal copy of it is created.
ScpScript::
ScpScript( const std::string& script_ )
	: Flags2(0), Flags3(ESS_HaveLocalScrCopy), ScriptCopy(script_), ScriptCStr(0,0)
	, Prgrm(new ScpTryProgramParse)
{
	ScriptCStr = std::pair<const char*,int>( ScriptCopy.c_str(), (int)ScriptCopy.size() );
}
/// Constructs script object with an option to not to create local
/// copy of the input script.
/// \param script_ - Input script. if 'len' is set to -1, length of the c-string
///                  (aka. null-terminated string) is obtained via 'strlen()'.
/// \param len - Length of the input script in the characters. Set to -1 assumes
///              null-terminated.
/// \param bCreateInternalCopy - If set to true, local copy of the input script is created.
///                  Otherwise, caller must ensure data reamin valid through
///                  the entire life of the object.
ScpScript::
ScpScript( const char* script_, int len, bool bCreateInternalCopy )
	: Flags2(0), Flags3(0), ScriptCStr(0,0)
	, Prgrm(new ScpTryProgramParse)
{
	len = ( len != -1 ? len : strlen(script_) );
	if( !bCreateInternalCopy ){
		Flags3 &= ~ESS_HaveLocalScrCopy;
		ScriptCStr = std::pair<const char*,int>( script_, len );
	}else{
		Flags3 |= ESS_HaveLocalScrCopy;
		ScriptCopy.assign( script_, len );
		ScriptCStr = std::pair<const char*,int>( ScriptCopy.c_str(), (int)ScriptCopy.size() );
	}
}
ScpScript::~ScpScript()
{
	assert(Prgrm);
	delete Prgrm;
	{
		std::vector<SHe>::iterator a;
		for( a = HostEnv2.begin(); a != HostEnv2.end(); ++a ){
			if(a->bOwn)
				delete a->hoper;
		}
		HostEnv2.clear();
	}{
		std::vector<SHo>::iterator a;
		for( a = HostObjs.begin(); a != HostObjs.end(); ++a ){
			if(a->bOwn2)
				delete a->hobj;
		}
		HostObjs.clear();
	}
}
/// Sets pseudo value for pseudo preprocessor.
/// This is done through the occurences of "#ifdef" or "#ifndef" conditional expressions.
/// Value name set here must be a single word.
/// operators such as logical or arithmetic cannot be part of the name.
/// Nesting is not supported, will trigger undefined behaviour. \n
/// WIP.
/// \todo Nesting in pseudo preprocessor.
/// \sa \ref pgPreproc
ScpScript& ScpScript::setCondDef( const char* dname, bool val )
{
	std::vector<std::pair<std::string,std::string> >::iterator a;
	if( (a = std::find_if( Defs.begin(), Defs.end(), ScpSp::PredStrStr(dname) )) != Defs.end() ){
		if(val){
			a->second = "1";
		}else{
			Defs.erase(a);
		}
	}else{
		if(val)
			Defs.push_back( std::pair<std::string,std::string>(dname,"1") );
	}
	return *this;
}
ScpScript& ScpScript::clear2()
{
	clear3( SCP_CF_ClearAll );
	return *this;
}
/// Clears stored results.
/// Clearing only selected results, rather than reinitializing the rntire
/// script object, may be usefull in saving some processing time.
/// \param flags3 - flags as an ORed list, fe. \ref SCP_CF_ClearStack.
ScpScript& ScpScript::clear3( int flags3 )
{
	LastError = ScpErr( ScpSp("",0), SCP_EE_Unknown );
	if( flags3 & SCP_CF_ClearCondDefs ){
		Defs.clear();
	}
	if( flags3 & SCP_CF_ClearParser ){
		Tokens2.clear();
		assert( Prgrm );
		delete Prgrm;
		Prgrm = new ScpTryProgramParse;
		IfDefBlocks.clear();
	}
	if( flags3 & SCP_CF_ClearStack ){
		;
	}
	return *this;
}
/**
	\mainpage

	\section intro_sec Introduction

	Welcome to the C++ scripting mini engine
	that aims to implement a handfull set of features from the
	<a href="https://www.ecma-international.org/ecma-262/5.1/">ECMA Script</a> specification.
	A way to freely manipulate objects exported from the C++ using
	arithmetic, parentheses or assignment operators.
	Enables compilation independent scripting from external sources
	an application can take input from, such as: config files, command-line,
	UI controls, etc.

	\htmlonly
		<style>
			.csScpStrpTblHdr table th{
				background-color: white;
				display: none;
			};
		</style>
		<div class="csScpStrpTblHdr">  <!-- a:[58B6B1FF] -->
	\endhtmlonly

	&nbsp;                                                       | &nbsp;
	------------------------------------------------------------ | ------------------
	\ref ScpScript "ScpScript"                                   | Main class in the scripting engine.\n
	\ref ScpScript::addHostObject() ".addHostObject()"           | Makes C++ objects visible in the scripts.
	\ref ScpScript::addHostOperator() ".addHostOperator()"       | Adds operators, like '+' or '=', to the script environment.
	\ref ScpScript::parse2() ".parse2()"                         | Starts script parsing.\n
	\ref ScpScript::eval2() ".eval2()"                           | Evaluates parsed script.\n
	\ref ScpScript::showMeTheError2() ".showMeTheError2()"       | Gets error info and shows it in the console STDOUT.\n
	\ref ScpScript::showMeTheError3() ".showMeTheError3(bool)"   | Gets error info into the std::string.\n
	\ref SCP_EE_Unknown "SCPEE*"                                 | Enumerated error types, on parse or on eval.
	\htmlonly
		</div> <!-- b:[58B6B1FF] -->
	\endhtmlonly

	<a HREF="annotated.html">Class list</a>\n
	<a HREF="functions.html">Class members</a>\n


	\section instructions_sec Instructions

	Compile all source files with your program.


	\section example_sec Script Parsing Example

	\code
		#include <stdio.h>
		#include <string>
		#include "scipp_script.h"
		#include "scipp_hostobj.h"

		class MyNumber : public ScpHostObject
		{
		public:
			MyNumber( int ResultVal_=0 ) : ResultVal(ResultVal_) {}
			virtual MyNumber operator*( const MyNumber& other ){
				return MyNumber( ResultVal * other.ResultVal );
			}
			virtual MyNumber operator+( const MyNumber& other ){
				return MyNumber( ResultVal + other.ResultVal );
			}
			int ResultVal;
		};

		int main()
		{
			std::string myCcode =
				"// Script example to run. \n"
				"v2 = v3 * v4         // simple arithmetic.\n"
				"v4 = (v2 + v3) * v3  // use of parentheses to force\n"
				"                     // operator '+' before '*'.";

			ScpScript sci( myCcode );
			if( !sci.parse2() ){
				sci.showMeTheError2();
				return 2;
			}
			MyNumber v2(2), v3(3), v4(4);
			sci.addHostOperator( new ScpMultiplicativeOperator<MyNumber>, 1 );
			sci.addHostOperator( new ScpAsssignmentOperator<MyNumber>, 1 );
			sci.addHostOperator( new ScpPlusOperator<MyNumber>, 1 );
			sci.addHostObject( "v2", &v2, 0 );
			sci.addHostObject( "v3", &v3, 0 );
			sci.addHostObject( "v4", &v4, 0 );
			if( !sci.eval2() ){
				sci.showMeTheError2();
				return 3;
			}
			printf("v2: %d\n", v2.ResultVal );
			printf("v3: %d\n", v3.ResultVal );
			printf("v4: %d\n", v4.ResultVal );
			printf("Application exit...\n");
			return 0;
		}
	\endcode
*/

/**
	\page pgFaq F.A.Q.
	.

	Welcome to the FAQ.

	Q: Why numbered function names, eg: \ref ScpScript::parse2() ".parse2()" or \ref ScpScript::eval2() ".eval2()" ?\n
	A: To make documentating process clearer and easier.\n
*/

/**
	\page pgPreproc Pseudo Preprocessor
	.

	Please note that this preprocessor uses rather simple parsing and won't
	provide features at the level same as, for example, C preprocessor.
	This is a simple extension - the
	<a href="https://www.ecma-international.org/ecma-262/5.1/">ECMA Script</a> specification
	tells nothing about - with the purpose to provide some conditional scripting
	in otherwise unconditional expression evaluating (at the current stage
	of the scripting library).

	Pseudo preprocessor values can be set using
	\ref ScpScript::setCondDef() ".setCondDef()" and are limited to boolean
	values, true if variable has been set or false otherwise.
	They can only be modified on the C++ end, using mentioned function.
	It is not possible to set or modify them from the scripts.
	All preprocessor values must be set before any parsing.

	With this functionality it is possible to
	execute only selected parts of the script, rather than having to always
	execute it entirely.
*/

/**
	\page pgHostObjects Host Object Routines
	.

	ScpHostObject \n
	ScpScript::addHostObject() \n
	ScpScript::addHostOperator() \n
	ScpHostOp \n
	ScpHostTOperator \n
	ScpMultiplicativeOperator \n
	ScpPlusOperator \n
	ScpAsssignmentOperator \n
*/

/**
	\page pgParseEval Script Parsing and Evaluating Routines
	.

	ScpScript::parse2() \n
	ScpScript::parse3() \n
	ScpScript::eval2() \n
	ScpScript::eval3() \n
	ScpScript::parseAndEval() \n
	ScpScript::showMeTheError2() \n
	ScpScript::showMeTheError3() \n
	ScpErr class \n
	\ref SCP_AF_AutoParse \n
*/

;
/// Tokenizes the script.
bool ScpScript::tokenize2( ScpErr& err2 )
{
	Tokens2.clear();
	const char* sz2 = ScriptCStr.first, *sz3 = 0;
	int len2 = ScriptCStr.second;
	char chr;
	int tpe2 = 0;
	for( int i=0; i<len2; sz3=0, tpe2=0 ){
		chr = sz2[i];
		if(0){
		}else if( i+1 < len2 && !strncmp( &sz2[i], "//", 2 ) ){
			//printf("'//' at %d \n", i );
			static const char* szNl = "\n";
			const char* endd = &sz2[len2], *x;
			x = std::search( &sz2[i], endd, szNl, szNl+1 );
			// if not found, eat the remaining text, this is the last line, newline hasn't benn found.
			sz3 = ( x == endd ? &sz2[len2] : x + 1 );
			tpe2 = SCP_ET_Wh;
		}else if( i+1 < len2 && !strncmp( &sz2[i], "/*", 2 ) ){
			//printf("'/*' at %d \n", i );
			static const char* szAstSl = "*/";
			const char* endd = &sz2[len2], *x;
			if( (x=std::search( &sz2[i], endd, szAstSl, szAstSl+2 )) == endd ){
				err2 = ScpErr( ScpSp( &sz2[i], 2 ), SCP_EE_CommentNoClose );
				return 0;
			}
			sz3 = x + 2;
			//printf("cl: [%s]\n", std::string(sz3,4).c_str() );
			tpe2 = SCP_ET_Wh;
		}else if( strchr( ScpSp::Ops, chr ) ){
			sz3 = &sz2[i] + 1;
			tpe2 = SCP_ET_Op;
		}else if( strchr( ScpSp::Whs, chr ) ){
			sz3 = ScpSp::strchrAdvance( &sz2[i], len2-i, ScpSp::Whs, 0 );
			tpe2 = SCP_ET_Wh;
		}else if( strchr( ScpSp::Wrd2, chr ) ){
			sz3 = ScpSp::strchrAdvance( &sz2[i], len2-i, ScpSp::Wrd2, ScpSp::Wrd3 );
			tpe2 = SCP_ET_Wrd;
		}else{
			err2 = ScpErr( ScpSp( &sz2[i], 1 ), SCP_EE_UnkToken );
			return 0;
		}
		assert( sz3 );
		int adv = sz3 - &sz2[i];
		ScpSp sp( &sz2[i], adv );
		Tokens2.push_back( ScpToken(sp,tpe2, ( tpe2 == SCP_ET_Wh ? 0:-1 ) ) );
		i += adv;
	}
	// remove unnecessary tokens, fe. whitespaces.
	ScpLsToken::iterator a;
	for( a = Tokens2.begin(); a != Tokens2.end(); ){
		if( a->tpe == SCP_ET_Wh ){
			a = Tokens2.erase(a);
		}else
			++a;
	}
	if( Flags2 & SCP_SF_ShowTokens ){
		ScpLsToken::iterator a;
		for( a = Tokens2.begin(); a != Tokens2.end(); ++a ){
			printf("'%s' %03d\n", a->tkn2.c_str(), a->tpe );
		}
	}
	return 1;
}
/// Parse and evaluates script in one call.
/// Suitable for small text scripts.
/// Equivalent of calling ScpScript::eval3() with \ref SCP_AF_AutoParse flag set.
/// \sa \ref pgParseEval
bool ScpScript::parseAndEval()
{
	if( parse2() )
		return eval2();
	return 0;
}
/**
	Parses the script.
	Script text must be set.
	Eval must not have been yet called.
	\sa \ref pgParseEval
*/
bool ScpScript::parse2()
{
	return parse3( 0, 0 );
}
/// Parses the script with an option to return the error info.
/// \param flags3 - currntly not used, set to 0.
/// \sa \ref pgParseEval
/// \sa ScpScript::parse2()
bool ScpScript::parse3( int flags3, ScpErr* err )
{
	Prgrm->reinit2();
	ScpErr err3, &err2 = ( err ? *err : err3 );
	if( !tokenize2( err2 ) ){
		ScpSp::finalizeErr( err2, ScriptCStr.first, ScriptCStr.second );
		LastError = err2;
		return 0;
	}
	if( !Defs.empty() ){
		// call pseudo preprocessor here, that modifies tokens list if needed.
		if( !preprocessCondDefs( err2 ) ){
			ScpSp::finalizeErr( err2, ScriptCStr.first, ScriptCStr.second );
			LastError = err2;
			return 0;
		}
	}
	ScpTCITR ended3;
	ScpParseOu ou2 = { 0, ended3, err2, };
	ScpParse prse( Tokens2.begin(), Tokens2.end(), ou2, Prgrm );
	if( !Prgrm->tryy( prse ) ){ // ScpTryProgramParse*
		ScpSp::finalizeErr( err2, ScriptCStr.first, ScriptCStr.second );
		LastError = err2;
		return 0;
	}
	if( Flags2 & SCP_SF_ShowExpressions ){
		Prgrm->setOriginalScript( ScriptCStr.first, ScriptCStr.second );
		printf("%s\n", Prgrm->strPrint2( ScpPrnt("") ).c_str() );
	}
	return 1;
}
/// Sets flags that are used durning script processing.
/// \param flags2 - flags as an ORed list, eg. \ref SCP_SF_ShowTokens.
void ScpScript::setOptions( int flags2 )
{
	Flags2 = flags2;
}
/// Prints error to the console STDOUT.
/// \sa ScpScript::showMeTheError3()
void ScpScript::showMeTheError2()const
{
	std::string z = showMeTheError3( 0 );
	printf("%s", z.c_str() );
}
/// Returns error as std::string.
/// \param bShortOutput - if set, shortens textual error from the left by not including
///        error label and text indent.
std::string ScpScript::showMeTheError3( bool bShortOutput )const
{
	std::string z;
	assert( LastError.iCol >= 0 );
	assert( LastError.iRow >= 0 );
	const ScpErr& err = LastError;
	char bfr2[256], bfr3[256], bfr4[256];
	std::string lineAt = err.strErrorLine;
	// TODO: [x] strip leading whitespaces.
	//       [x] replace tabs with spaces.
	//       [ ] shift line left if too long.
	std::string strTorP;
	if( err.tpe3 >= SCP_EE_UserEvalNull ){
		strTorP = "Evaluating";
	}else if( err.tpe3 >= SCP_EE_EofOnParse ){
		strTorP = "Parsing";
	}else if( err.tpe3 >= SCP_EE_NoMatchingEndif ){
		strTorP = "Preprocessing";
	}else{ // assume >= SCP_EE_UnkToken.
		strTorP = "Tokenizing";
	}
	int iColModded = err.iCol;
	{
		lineAt = ScpSp::strReplace(lineAt,"\t"," ");
		// remove any leading whitespaces.
		std::string::iterator a;
		for( a = lineAt.begin(); a != lineAt.end() && *a == '\x20'; ++a, iColModded-- );
		lineAt.erase( lineAt.begin(), a );
		if( !(Flags2 & SCP_SF_NoErrLineRTrim) ){
			// max line length.
			size_t lenOri = lineAt.size();
			if( lenOri > 52 ){ // hardcoded, keeping 42-64 seems to be reasonable.
				lineAt.resize( 52, ' ' );
				lineAt += "...";
			}
		}
	}
	sprintf(bfr2,"ERROR: %s failed; line:%d, column:%d, enum:%d.\n", strTorP.c_str(), err.iRow+1, err.iCol+1, err.tpe3 );
	sprintf(bfr3,"       [%s]\n", lineAt.c_str() );
	sprintf(bfr4,"        %s\n", (std::string(iColModded,' ')+"^").c_str() );
	z += bfr2;
	z += bfr3;
	z += bfr4;
	if( bShortOutput ){
		z = ScpSp::strReplace( z,   "ERROR: ", "" );
		z = ScpSp::strReplace( z, "\n       ", "\n" );
	}
	return z;
}
/// Evaluates the script.
/// Parsing must have been performed.
/// \sa \ref pgParseEval
bool ScpScript::eval2()
{
	return eval3( 0, 0 );
}
/// Evaluates the script.
/// Parsing must have been performed or 'flags4' should contain appropriate
/// flag to do it automatically here.
/// \param flags4 - flags as ORed list, fe. \ref SCP_AF_AutoParse.
/// \sa \ref pgParseEval
/// \sa \ref ScpScript::eval2()
bool ScpScript::eval3( int flags4, ScpErr* err )
{
	if( flags4 & SCP_AF_AutoParse ){
		if( !parse3( 0, err ) )
			return 0;
	}
	ScpErr err3, &err2 = ( err ? *err : err3 );
	if( !eval9( flags4, err2 ) ){
		ScpSp::finalizeErr( err2, ScriptCStr.first, ScriptCStr.second );
		LastError = err2;
		return 0;
	}
	return 1;
}
bool ScpScript::
scanUntilSequenceRecv( int dpth, size_t& ioNum, const ScpTITR& bgn,
						const ScpTITR& endd, int& mode3, SIfs& ifs2,
						ScpTITR& ended2, ScpErr& err, int mode4 )
{
	ScpTITR a = bgn;
	if( mode3 == EIDM_Els ){
		SIfdef ifd( mode4, bgn, ++a ); //EIDT_Ifdef,EIDT_IfNdef
		ifd.cond2 = ++a;
		ioNum -= 3; // ifdef is made out of 3 tokens: '#', "ifdef" and condition.
		if( ioNum < 0 ){
			err = ScpErr( bgn->tkn, SCP_EE_IfdefEof );
			return 0;
		}
		if( ifd.cond2->tpe != SCP_ET_Wrd ){
			err = ScpErr( ifd.cond2->tkn, SCP_EE_BadIfdefCond );
			return 0;
		}
		ifs2.ifdefs2.push_back( ifd );
		++a;
	}
	for( ; a != endd; ){
		assert( ioNum > 0 );
		int mode5 = -1;
		if( ioNum >= myIfdef.size() && std::equal( myIfdef.begin(), myIfdef.end(), a ) )
			mode5 = EIDT_Ifdef;
		if( ioNum >= myIfNdef.size() && std::equal( myIfNdef.begin(), myIfNdef.end(), a ) )
			mode5 = EIDT_IfNdef;
		if( mode5 != -1 ){
			mode3 = EIDM_Els;
			SIfs ifs3;
			if(!scanUntilSequenceRecv( dpth+1, ioNum, a, endd, mode3, ifs3, a, err, mode5 ))
				return 0;
			IfDefBlocks.push_back(ifs3);//ifs2
			continue;
		}
		if( ioNum >= myEndif.size() && std::equal( myEndif.begin(), myEndif.end(), a ) ){
			SIfdef ifd( EIDT_Endif, a, endd );
			ifd.wrd = ++a;
			ifs2.ifdefs2.push_back( ifd );
			ioNum -= myEndif.size();
			if(!dpth){
				err = ScpErr( a->tkn, SCP_EE_UnxpctdEndif );
				return 0;
			}
			ended2 = ++a;
			mode3 = EIDM_Ifs;
			return 1;
		}else{
			++a;
			--ioNum;
		}
	}
	if( mode3 == EIDM_Els ){
		err = ScpErr( bgn->tkn, SCP_EE_UntrmtdIfdef );
		return 0;
	}
	return 1;
}
bool ScpScript::preprocessCondDefs( ScpErr& err )
{
	ScpTITR a, endd = Tokens2.end(); //ScpToken
	IfDefBlocks.clear();
	if( myIfdef.empty() ){
		myIfdef.push_back( ScpToken( ScpSp("#",-1),     SCP_ET_Op ) );
		myIfdef.push_back( ScpToken( ScpSp("ifdef",-1), SCP_ET_Wrd ) );
		myEndif.push_back( ScpToken( ScpSp("#",-1),     SCP_ET_Op ) );
		myEndif.push_back( ScpToken( ScpSp("endif",-1), SCP_ET_Wrd ) );
		myIfNdef.push_back( ScpToken( ScpSp("#",-1),      SCP_ET_Op ) );
		myIfNdef.push_back( ScpToken( ScpSp("ifndef",-1), SCP_ET_Wrd ) );
	}
	{
		size_t n = Tokens2.size();
		int mode3 = EIDM_Ifs;
		SIfs ifs4;
		if( !scanUntilSequenceRecv( 0, n, Tokens2.begin(), endd, mode3, ifs4, a, err, EIDT_Endif ))
			return 0;
	}
	if( Flags2 & SCP_SF_ShowCondDefs ){
		printf("n-IfDefBlocks: %d\n", (int)IfDefBlocks.size() );
		std::vector<SIfs>::const_iterator b;
		std::vector<SIfdef>::const_iterator c;
		for( b = IfDefBlocks.begin(); b != IfDefBlocks.end(); ++b ){
			printf("n:%d\n", (int)b->ifdefs2.size() );
			for( c = b->ifdefs2.begin(); c != b->ifdefs2.end(); ++c ){
				int iCol = -1, iLnn;
				iLnn = ScpSp::convPosToLineAndCol( ScriptCStr.first,
						ScriptCStr.second, -1, c->htg->tkn.ptr, &iCol, 0 );
				std::string cnd = (c->tpe4 != EIDT_Endif ? c->cond2->tkn2.c_str() : "--");
				printf("\t%s%s %s (%d,%d)\n", c->htg->tkn2.c_str(), c->wrd->tkn2.c_str(),
						cnd.c_str(),
						iLnn+1, iCol+1 );
			}
		}
	}
	if( !applyCondDefs( err ) )
		return 0;
	return 1;
}
bool ScpScript::applyCondDefs( ScpErr& err )
{
	std::vector<std::pair<std::string,std::string> >::const_iterator c;

	// perform erasing from reverse to prevent iterator invalidation (not true for std::list ??).
	std::vector<SIfs>::const_reverse_iterator b;
	std::vector<std::pair<std::string,std::string> >::iterator end2 = Defs.end();
	for( b = IfDefBlocks.rbegin(); b != IfDefBlocks.rend(); ++b ){
		assert( b->ifdefs2.size() >= 2 );
		assert( b->ifdefs2.rbegin()->tpe4 == EIDT_Endif );
		const SIfdef& ifdif2  = *b->ifdefs2.begin();
		const SIfdef& ifdend2 = *b->ifdefs2.rbegin();
		c = std::find_if( Defs.begin(), end2, ScpSp::PredStrStr(ifdif2.cond2->tkn2) );
		bool bTrue = ( ifdif2.tpe4 == EIDT_IfNdef ? c == end2 : c != end2 );
		if( !bTrue ){ // if condition false - cond-def not found in the list.
			ScpTITR tknBgn = ifdif2.htg; //get token begin.
			ScpTITR tknEnd = ifdend2.wrd; //get token last.
			std::advance( tknEnd, 1 );
			Tokens2.erase( tknBgn, tknEnd );
		}else{ // else erase only the tokens that make up the if-def syntax.
			ScpTITR endd = ifdend2.wrd;
			std::advance( endd, 1 );
			Tokens2.erase( ifdend2.htg, endd );
			//
			endd = ifdif2.cond2;
			std::advance( endd, 1 );
			Tokens2.erase( ifdif2.htg, endd );
		}
	}
	// once done erasing, iterators if if-def blocks became invalidatd.
	IfDefBlocks.clear();
	return 1;
}
bool ScpScript::eval9( int flags4, ScpErr& err )
{
	ScpEvalOu ou2( err );
	ScpEval sev( ou2, this, "script_scope" );
	if( !Prgrm->eval6( sev ) )
		return 0;
	sev.clear5();
	return 1;
}
/**
	Adds host object.
	All objects must be added before calling one of the eval functions.
	\param objectname - name of the object under which to appear in the text scripts.
	\param hobject - input object.
	\param bOwnAndDelete - if true, input object is "owned", meaning it will
						   be automatcally deleted when this class is destroyed.
*/
void ScpScript::
addHostObject( const char* objectname, ScpHostObject* hobject, bool bOwnAndDelete )
{
	HostObjs.push_back( SHo(objectname,hobject,bOwnAndDelete) );
}
/**
	Adds operator to the script environment.
	Please note that there are more requirements on the actual operator being added than
	just it being derived class from the ScpHostOp.
	Refer to the descriptioon of the ScpHostTOperator template class for more info.
	\param inp - input operator.
	\param bOwnAndDel - if true, input object is "owned", meaning it will
	                    be automatcally deleted when this class is destroyed.

	\sa \ref pgHostObjects
*/
void ScpScript::addHostOperator( ScpHostOp* inp, bool bOwnAndDel )
{
	SHe hoper;
	hoper.hoper = inp;
	hoper.bOwn  = bOwnAndDel;
	HostEnv2.push_back(hoper);
}
bool ScpScript::SHe::ByOper::operator()( const SHe& otherr )const
{
	return !strcmp( szOper, otherr.hoper->getOpName() );
}
bool ScpScript::evalHostObjectPair( const ScpEvalPair& evp )
{
	std::vector<SHo>::iterator endd = HostObjs.end();
	ScpHostObject* hobjA = 0, *hobjB = 0;
	{
		std::vector<SHo>::iterator a;
		SHo shoA( evp.nameA );
		if( (a=std::find_if( HostObjs.begin(), endd, shoA )) == endd ){
			ScpHostObjVal* hov3 = dynamic_cast<ScpHostObjVal*>( evp.lval );
			if( !hov3 || !( hobjA = dynamic_cast<ScpHostObject*>( hov3->getHostObValue() ) )){
				*evp.iErrIs = 0;
				*evp.eErr = SCP_EE_NoSuchHostObject;
				return 0;
			}
		}else
			hobjA = a->hobj;
	}{
		SHo shoB( evp.nameB );
		std::vector<SHo>::iterator b;
		if( (b=std::find_if( HostObjs.begin(), endd, shoB )) == endd ){
			ScpHostObjVal* hov2 = dynamic_cast<ScpHostObjVal*>( evp.rval );
			if( !hov2 || !( hobjB = dynamic_cast<ScpHostObject*>( hov2->getHostObValue() ) )){
				*evp.iErrIs = 1;
				*evp.eErr = SCP_EE_NoSuchHostObject;
				return 0;
			}
		}else
			hobjB = b->hobj;
	}
	assert( hobjA && hobjB );
	//
	std::vector<SHe>::iterator e, end2 = HostEnv2.end();
	e = std::find_if( HostEnv2.begin(), end2, SHe::ByOper(evp.oper.tkn2.c_str()) );
	if( e == end2 ){
		*evp.eErr = SCP_EE_NoSuchHostOper;
		return 0;
	}
	ScpHostObject* resval = 0;
	ScpHof shf = { hobjA, evp.oper.tkn2.c_str(), hobjB, &resval, evp.iErrIs, evp.eErr, };
	if( !e->hoper->callHostOperatorFnc( shf ) )
		return 0;
	assert(resval);
	// in case of operators, like assignment, that modify input value,
	// and does not create new, result value must not be freed.
	// other operators, like multiplicative, create and return new value instance,
	// that later, needs to be freed by the script library.
	// marked here as is-in-script-scope-only, will be deleted at some point.
	bool bIsInScrScopeOnlyy = !( resval == hobjB || resval == hobjA );
	ScpHostObjVal* hov = new ScpHostObjVal( "", evp.oper, resval, bIsInScrScopeOnlyy );
	*evp.val2 = hov;
	return 1;
}





