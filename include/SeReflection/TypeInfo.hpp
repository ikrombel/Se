#pragma once

#include <typeinfo>

//#include "Base.h"

namespace Se {

template <int v> struct IntToType {
	enum { Value = v };
};

template <class T> struct TypeToType {
	typedef T Type;
};

class TypeInfo {

public:

    TypeInfo() : type_info(nullptr) { }
    TypeInfo(const TypeInfo &type) : type_info(type.type_info) { }
    explicit TypeInfo(const std::type_info &type_info) : type_info(&type_info) { }
    ~TypeInfo() { }

    TypeInfo &operator=(const TypeInfo &type) {
        type_info = type.type_info;
        return *this;
    }

    const char *name() const {
        if(type_info == 0)
			return "unknown";
        return type_info->name();
    }

    const std::type_info &get() const {
        if (type_info == 0)
			return typeid(void);
        return *type_info;
    }

private:

    friend bool operator==(const TypeInfo &t0,const TypeInfo &t1);
    friend int operator<(const TypeInfo &t0,const TypeInfo &t1);

    const std::type_info *type_info;
};

inline bool operator==(const TypeInfo &t0,const TypeInfo &t1) {
	if (t0.type_info == 0 || t1.type_info == 0)
        return false;
	return t0.type_info->operator==(*(t1.type_info));
}

inline bool operator!=(const TypeInfo &t0, const TypeInfo &t1) {
	return !operator==(t0,t1);
}

inline int operator<(const TypeInfo &t0,const TypeInfo &t1) {
	if (t0.type_info == 0 || t1.type_info == 0) 
        return 0;
	return t0.type_info->before(*(t1.type_info));
}

inline int operator>(const TypeInfo &t0,const TypeInfo &t1) {
	return !operator<(t0,t1);
}


struct TypeListEnd {

};

template <class T,class U> struct TypeList {
	typedef T Head;
	typedef U Tail;
};

template <class T0 = TypeListEnd,class T1 = TypeListEnd,class T2 = TypeListEnd,class T3 = TypeListEnd,
class T4 = TypeListEnd,class T5 = TypeListEnd,class T6 = TypeListEnd,class T7 = TypeListEnd,
class T8 = TypeListEnd,class T9 = TypeListEnd,class T10 = TypeListEnd,class T11 = TypeListEnd,
class T12 = TypeListEnd,class T13 = TypeListEnd,class T14 = TypeListEnd,class T15 = TypeListEnd>
struct MakeTypeList {
	typedef TypeList<T0,typename MakeTypeList<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15>::Type> Type;
};

template <> struct MakeTypeList<> {
	typedef TypeListEnd Type;
};

template <class T> struct Length;

template <> struct Length<TypeListEnd> {
	enum { Value = 0 };
};

template <class T,class U> struct Length<TypeList<T,U> > {
	enum { Value = 1 + Length<U>::Value };
};

template <class T,unsigned int> struct TypeAt {
	typedef TypeListEnd Type;
};

template <class T,class U> struct TypeAt<TypeList<T,U>,0> {
	typedef T Type;
};

template <class T,class U,unsigned int i> struct TypeAt<TypeList<T,U>,i> {
	typedef typename TypeAt<U,i-1>::Type Type;
};

template <class Type> struct IsClassFunction {
	enum { Value = 0 };
};

template <class Class,class Type> struct IsClassFunction<Type (Class::*)()> { enum { Value = 1 }; };
template <class Class,class Type> struct IsClassFunction<Type (Class::*)() const> { enum { Value = 1 }; };
template <class Class,class Type,class A0> struct IsClassFunction<Type (Class::*)(A0)> { enum { Value = 1 }; };
template <class Class,class Type,class A0> struct IsClassFunction<Type (Class::*)(A0) const> { enum { Value = 1 }; };
template <class Class,class Type,class A0,class A1> struct IsClassFunction<Type (Class::*)(A0,A1)> { enum { Value = 1 }; };
template <class Class,class Type,class A0,class A1> struct IsClassFunction<Type (Class::*)(A0,A1) const> { enum { Value = 1 }; };
template <class Class,class Type,class A0,class A1,class A2> struct IsClassFunction<Type (Class::*)(A0,A1,A2)> { enum { Value = 1 }; };
template <class Class,class Type,class A0,class A1,class A2> struct IsClassFunction<Type (Class::*)(A0,A1,A2) const> { enum { Value = 1 }; };
template <class Class,class Type,class A0,class A1,class A2,class A3> struct IsClassFunction<Type (Class::*)(A0,A1,A2,A3)> { enum { Value = 1 }; };
template <class Class,class Type,class A0,class A1,class A2,class A3> struct IsClassFunction<Type (Class::*)(A0,A1,A2,A3) const> { enum { Value = 1 }; };
template <class Class,class Type,class A0,class A1,class A2,class A3,class A4> struct IsClassFunction<Type (Class::*)(A0,A1,A2,A3,A4)> { enum { Value = 1 }; };
template <class Class,class Type,class A0,class A1,class A2,class A3,class A4> struct IsClassFunction<Type (Class::*)(A0,A1,A2,A3,A4) const> { enum { Value = 1 }; };
template <class Class,class Type,class A0,class A1,class A2,class A3,class A4,class A5> struct IsClassFunction<Type (Class::*)(A0,A1,A2,A3,A4,A5)> { enum { Value = 1 }; };
template <class Class,class Type,class A0,class A1,class A2,class A3,class A4,class A5> struct IsClassFunction<Type (Class::*)(A0,A1,A2,A3,A4,A5) const> { enum { Value = 1 }; };
template <class Class,class Type,class A0,class A1,class A2,class A3,class A4,class A5,class A6> struct IsClassFunction<Type (Class::*)(A0,A1,A2,A3,A4,A5,A6)> { enum { Value = 1 }; };
template <class Class,class Type,class A0,class A1,class A2,class A3,class A4,class A5,class A6> struct IsClassFunction<Type (Class::*)(A0,A1,A2,A3,A4,A5,A6) const> { enum { Value = 1 }; };
template <class Class,class Type,class A0,class A1,class A2,class A3,class A4,class A5,class A6,class A7> struct IsClassFunction<Type (Class::*)(A0,A1,A2,A3,A4,A5,A6,A7)> { enum { Value = 1 }; };
template <class Class,class Type,class A0,class A1,class A2,class A3,class A4,class A5,class A6,class A7> struct IsClassFunction<Type (Class::*)(A0,A1,A2,A3,A4,A5,A6,A7) const> { enum { Value = 1 }; };

}