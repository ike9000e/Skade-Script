#include "scipp_value.h"
#include <stdio.h>
#include "scipp_tryparser.h"

static const bool bCountStaticVal = 0;
static int        nCountStaticVal = 0;
ScpValue::ScpValue( const char* szVarname, const ScpToken* tknn )
	: Token2( !tknn ? 0 : new ScpToken( *tknn ) ), Varname(szVarname)
{
	if(bCountStaticVal)
		printf("ScpValue() %d\n", ++nCountStaticVal );
}
ScpValue::~ScpValue()
{
	if(bCountStaticVal){
		printf("~ScpValue() %d\n", --nCountStaticVal );
	}
	if( Token2 ){
		delete Token2;
		Token2 = 0;
	}
}
ScpSubtleRefVal::ScpSubtleRefVal( const char* szVarname, const ScpToken& tknn )
	: ScpValue(szVarname,&tknn) //, Varname(szVarname)
{
}
ScpDummyVal::ScpDummyVal( const ScpToken& tknn )
	: ScpValue("ScpDummyVal",&tknn)
{
}
ScpHostObjVal::
ScpHostObjVal( const char* szVarname, const ScpToken& tknn, ScpHostObject* HostObj_, bool bIsInScrScopeOnly_ )
	: ScpValue(szVarname,&tknn), HostObj(HostObj_), bIsInScrScopeOnly(bIsInScrScopeOnly_)
{
}
ScpHostObjVal::~ScpHostObjVal()
{
	if(bIsInScrScopeOnly){
		delete HostObj;
	}
	HostObj = 0;
}
