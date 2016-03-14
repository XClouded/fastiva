#include <Fastiva.h>
#include <java/lang/Math.inl>

union Convert64 {
    u4 arg[2];
    s8 ll;
    double dd;
};

union Convert32 {
    u4 arg;
    float ff;
};



jdouble fastiva_Math_d_abs(jdouble v) {
    Convert64 convert;
    convert.dd = v;
    /* clear the sign bit in the (endian-dependent) high word */
    convert.ll &= 0x7fffffffffffffffULL;
    return convert.dd;
}

jfloat fastiva_Math_f_abs(jfloat v) {
    Convert32 convert;
	convert.ff = v;
    /* clear the sign bit; assumes a fairly common fp representation */
    convert.arg &= 0x7fffffff;
    return convert.ff;
}

jint fastiva_Math_i_abs(jint val) {
    return (val >= 0) ? val : -val;
}

jlonglong fastiva_Math_j_abs(jlonglong val) {
	return (val >= 0) ? val : -val;
}

jint fastiva_Math_max(jint arg0, jint arg1) {
    return ((s4) arg0 > (s4) arg1) ? arg0 : arg1;
}

jint fastiva_Math_min(jint arg0, jint arg1) {
    return ((s4) arg0 < (s4) arg1) ? arg0 : arg1;
}




jlonglong fastiva_Double_doubleToLongBits(jdouble v) {
    Convert64 convert;
	convert.dd = v;
	int hibit = (convert.ll >> 32) & 0x7ff80000;
	if (hibit == 0x7ff80000) {
		return 0x7ff8000000000000LL;
	}
	return convert.ll;
}

jlonglong fastiva_Double_doubleToRawLongBits(jdouble v) {
    Convert64 convert;
	convert.dd = v;
	return convert.ll;
}

jdouble fastiva_Double_longBitsToDouble(jlonglong v) {
    Convert64 convert;
	convert.ll = v;
	return convert.dd;
}


jint fastiva_Float_floatToIntBits(jfloat v) {
    Convert32 convert;
    convert.ff = v;
	if ((convert.arg & 0x7fc00000) == 0x7fc00000) {
		return 0x7fc00000;
	}
    return convert.arg;
}

jint fastiva_Float_floatToRawIntBits(jfloat v) {
    Convert32 convert;
    convert.ff = v;
    return convert.arg;
}

jfloat fastiva_Float_intBitsToFloat(jint v) {
    Convert32 convert;
    convert.arg = v;
    return convert.ff;
}