#ifndef __FASTIVA_FLOAT_POINT_H__
#define __FASTIVA_FLOAT_POINT_H__


inline double REMAINDER$(double a, double b) {
	return fastiva.IEEEremainder(a, b);
}

inline float REMAINDER$(float a, float b) {
	return fastiva.IEEEremainderF(a, b);
}

inline jfloat fastiva_Float__NEGATIVE_INFINITY() {
	uint v = 0xff800000;
	return *(jfloat*)(void*)&v;
}

inline jfloat fastiva_Float__POSITIVE_INFINITY() {
	uint v = 0x7f800000;
	return *(jfloat*)(void*)&v;
}
inline jfloat fastiva_Float__NaN() {
	uint v = 0x7fc00000;
	return *(jfloat*)(void*)&v;
}
inline jfloat fastiva_Float__NEGATIVE_ZERO() {
	uint v = 0x80000000;
	return *(jfloat*)(void*)&v;
}
inline jfloat fastiva_Float__MAX_VALUE() {
	uint v = 0x7f7fffff;
	return *(jfloat*)(void*)&v;
}
inline jfloat fastiva_Float__MIN_VALUE() {
	uint v = 1;
	return *(jfloat*)(void*)&v;
}


inline jdouble fastiva_Double__NEGATIVE_INFINITY() { 
	ulonglong v = 0xfff0000000000000LL;
	return *(jdouble*)(void*)&v;
}
inline jdouble fastiva_Double__POSITIVE_INFINITY() { 
	ulonglong v = 0x7ff0000000000000LL;
	return *(jdouble*)(void*)&v;
}
inline jdouble fastiva_Double__NaN() { 
	ulonglong v = 0x7ff8000000000000LL;
	return *(jdouble*)(void*)&v;
}
inline jdouble fastiva_Double__NEGATIVE_ZERO() { 
	ulonglong v = 0x8000000000000000LL;
	return *(jdouble*)(void*)&v;
}
inline jdouble fastiva_Double__MAX_VALUE() { 
	ulonglong v = 0x7fefffffffffffffLL;
	return *(jdouble*)(void*)&v;
}
inline jdouble fastiva_Double__MIN_VALUE() { 
	ulonglong v = 1L;
	return *(jdouble*)(void*)&v;
}


inline jint F2I$(float v)			{ 
	if (v <= (int)0x80000000) {
		return (int)0x80000000;
	}
	if (v >= (int)0x7fffffff) {
		return (int)0x7fffffff;
	}
	int res = (jint)v; 
	if (res == 0 && v != v) {
		return 0;
	}
	return res;
}

inline jint D2I$(double v)		{ 
	if (v <= (int)0x80000000) {
		return (int)0x80000000;
	}
	if (v >= (int)0x7fffffff) {
		return (int)0x7fffffff;
	}
	int res = (jint)v; 
	if (res == 0 && v != v) {
		return 0;
	}
	return res;
}
inline jlonglong F2L$(float v)	{ 
	if (v <= (jlonglong)0x8000000000000000LL) {
		return (jlonglong)0x8000000000000000LL;
	}
	if (v >= (jlonglong)0x7fffffffffffffffLL) {
		return (jlonglong)0x7fffffffffffffffLL;
	}
	jlonglong res = (jlonglong)v; 
	if (res == 0 && v != v) {
		return 0;
	}
	return res;
}
inline jlonglong D2L$(double v)	{ 
	if (v <= (jlonglong)0x8000000000000000LL) {
		return (jlonglong)0x8000000000000000LL;
	}
	if (v >= (jlonglong)0x7fffffffffffffffLL) {
		return (jlonglong)0x7fffffffffffffffLL;
	}
	jlonglong res = (jlonglong)v; 
	if (res == 0 && v != v) {
		return 0;
	}
	return res;
}


inline jint FCMPL$(float v1, float v2) {
	//if (v1 < v2)  return -1;
	if (v1 == v2) return 0;
	if (v1 > v2) return +1;
	return -1;
}

inline jint FCMPG$(float v1, float v2) {
	if (v1 < v2)  return -1;
	if (v1 == v2) return 0;
	//if (v1 > v2) return +1;
	return +1;
}
inline jint DCMPL$(double v1, double v2) {
	//if (v1 < v2)  return -1;
	if (v1 == v2) return 0;
	if (v1 > v2) return +1;
	return -1;
}
inline jint DCMPG$(double v1, double v2) {
	if (v1 < v2)  return -1;
	if (v1 == v2) return 0;
	//if (v1 > v2) return +1;
	return +1;
}

/*
inline jlonglong LCMP$(jlonglong v1, jlonglong v2) {
	return (v1 - v2);
}
//*/
inline jint LCMP$(jlonglong v1, jlonglong v2) {
	if (v1 < v2) {
		return -1;
	}
	if (v1 > v2) {
		return +1;
	}
	return 0;
}
//*/
#endif // __FASTIVA_FLOAT_POINT_H__
