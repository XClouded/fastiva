inline
JPP_RETURNS(VAL$(jdouble))
java_lang_Math::abs_(jdouble v) {
    fastiva_Convert64 convert;
    convert.dd = v;
    /* clear the sign bit in the (endian-dependent) high word */
    convert.ll &= 0x7fffffffffffffffULL;
    return convert.dd;
}

inline
JPP_RETURNS(VAL$(jfloat))
java_lang_Math::abs_(jfloat v) {
    fastiva_Convert32 convert;
	convert.ff = v;
    /* clear the sign bit; assumes a fairly common fp representation */
    convert.arg &= 0x7fffffff;
    return convert.ff;
}

inline
JPP_RETURNS(VAL$(jint))
java_lang_Math::abs_(jint val) {
    return (val >= 0) ? val : -val;
}

inline
JPP_RETURNS(VAL$(jlonglong))
java_lang_Math::abs_(jlonglong val) {
	return (val >= 0) ? val : -val;
}

inline
JPP_RETURNS(VAL$(jint))
java_lang_Math::max_(jint arg0, jint arg1) {
    return ((s4) arg0 > (s4) arg1) ? arg0 : arg1;
}

inline
JPP_RETURNS(VAL$(jint))
java_lang_Math::min_(jint arg0, jint arg1) {
    return ((s4) arg0 < (s4) arg1) ? arg0 : arg1;
}



