inline
JPP_RETURNS(VAL$(jint))
java_lang_Float::floatToIntBits_(jfloat v) {
    fastiva_Convert32 convert;
    convert.ff = v;
	if ((convert.arg & 0x7fc00000) == 0x7fc00000) {
		return 0x7fc00000;
	}
    return convert.arg;
}

inline
JPP_RETURNS(VAL$(jint))
java_lang_Float::floatToRawIntBits_(jfloat v) {
    fastiva_Convert32 convert;
    convert.ff = v;
    return convert.arg;
}

 
inline
JPP_RETURNS(VAL$(jfloat))
java_lang_Float::intBitsToFloat_(jint v) {
    fastiva_Convert32 convert;
    convert.arg = v;
    return convert.ff;
}

