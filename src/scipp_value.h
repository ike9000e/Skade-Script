
#ifndef _HF_SCIPP_VALUE_H_
#define _HF_SCIPP_VALUE_H_
#include <string>

struct ScpEval; class ScpToken; class ScpHostObject;

/// Base for all values.
/// Conversion between value types is to be implemented as
/// overrided methods in derived classes.
class ScpValue {
public:
	;                       ScpValue( const char* szVarname, const ScpToken* );
	virtual                 ~ScpValue() = 0;
	virtual std::string     strPrint3()const {return "ScpValue;";}
	//virtual bool          valToNumber( const ScpEval& in ) {return 0;}
	//virtual bool          valToString( const ScpEval& in ) {return 0;}
	virtual const ScpToken* getTokenIfAny()const {return Token2;}
	const char*             getVarname()const {return Varname.c_str();}
private:
	ScpToken* Token2;
	std::string Varname;
};
class ScpSubtleRefVal : public ScpValue {
public:
	ScpSubtleRefVal( const char* szVarname, const ScpToken& );
};

class ScpHostObjVal : public ScpValue {
public:
	ScpHostObjVal( const char* szVarname, const ScpToken&, ScpHostObject* HostObj_, bool bIsInScrScopeOnly_ );
	virtual ~ScpHostObjVal();
	ScpHostObject* getHostObValue() {return HostObj;}
private:
	ScpHostObject* HostObj;
	bool bIsInScrScopeOnly;
};

#endif // _HF_SCIPP_VALUE_H_
