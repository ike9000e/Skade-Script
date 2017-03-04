#ifndef _SCIPP_HOSTOBJ_H_
#define _SCIPP_HOSTOBJ_H_
#include "scipp_tryparser.h"

/**
    Generic Template class for implementing operators
	like assignment or addition. For importing objects
	from the host environment (C++ end) into the scripts.
	Requirements on a class that is passed as templeate parameter 'TVar':
	- must be derived from \ref ScpHostObject.
	- must provide appropriate, one or more, C++ operator functions,
	  that, in turn, classes derived from this (ScpHostTOperator) use.

    There already exist, less generic, handfull list of template classes for operators:
	"*", "+" and "=", respectivelly: ScpPlusOperator,
	ScpMultiplicativeOperator or ScpAsssignmentOperator.
    If they can't provide sufficient features, this class can be used to
    reimplement them.
    \sa \ref pgHostObjects
*/
template<class TVar>
class ScpHostTOperator : public ScpHostOp {
	struct SOper;
public:
	/// Main method of the host operator class.
	/// After performing it's operation, and either creating new object, or
	/// modyfying one of operands, result must be returned as a member of the
	/// input structire.
	/// \return Returns true on success.
	virtual ScpHostObject* evaluateAxB( TVar& A, TVar& B ) = 0;
	/// Returns just operator as c-string (aka null-terminated string).
	/// Eg. "+" or "=".
	virtual const char* getOpName()const = 0;
	/// Called from the library, default implementation should be functional enough.
	virtual bool callHostOperatorFnc( const ScpHof& inp )
	{
		TVar* A = dynamic_cast<TVar*>( inp.lval );
		TVar* B = dynamic_cast<TVar*>( inp.rval );
		if( !A || !B ){
			*inp.iErrIs = ( !A? 0 : 1 );
			*inp.eErr = SCP_EE_NoSuchHostObject;
			return 0;
		}
		*inp.retval = evaluateAxB( *A, *B );
		return 1;
	}
};
/// Multiplicative operator ("*").
/// \sa \ref pgHostObjects
template<class TVar>
class ScpMultiplicativeOperator : public ScpHostTOperator<TVar>
{
public:
	virtual const char* getOpName()const {return "*";}
	virtual ScpHostObject* evaluateAxB( TVar& A, TVar& B ){
		TVar* C = new TVar;
		*C = A * B;
		return C;
	}
};
/// Plus (or additive) operator ("+").
/// \sa \ref pgHostObjects
template<class TVar>
class ScpPlusOperator : public ScpHostTOperator<TVar> { public:
	virtual const char* getOpName()const {return "+";}
	virtual ScpHostObject* evaluateAxB( TVar& A, TVar& B ){
		TVar* C = new TVar;
		*C = A + B;
		return C;
	}
};
/// Assignment operator, the '='.
/// Note that there is a difference in comparision versus plus or multiplicative
/// operators. Instead of creating a new object instance, one of the operand objects,
/// normally left, must be modified and pointer to it returned.
/// \sa \ref pgHostObjects
template<class TVar>
class ScpAsssignmentOperator : public ScpHostTOperator<TVar> { public:
	virtual const char* getOpName()const {return "=";}
	virtual ScpHostObject* evaluateAxB( TVar& A, TVar& B ){
		A = B;
		return &A;
	}
};

#endif // _SCIPP_HOSTOBJ_H_
