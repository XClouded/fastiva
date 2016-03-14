inline
JPP_RETURNS(VAL$(jlonglong))
java_lang_Double::doubleToLongBits_(jdouble v) {
    fastiva_Convert64 convert;
	convert.dd = v;
	int hibit = (convert.ll >> 32) & 0x7ff80000;
	if (hibit == 0x7ff80000) {
		return 0x7ff8000000000000LL;
	}
	return convert.ll;
}

 
inline
JPP_RETURNS(VAL$(jlonglong))
java_lang_Double::doubleToRawLongBits_(jdouble v) {
    fastiva_Convert64 convert;
	convert.dd = v;
	return convert.ll;
}

 
inline
JPP_RETURNS(VAL$(jdouble))
java_lang_Double::longBitsToDouble_(jlonglong v) {
    fastiva_Convert64 convert;
	convert.ll = v;
	return convert.dd;
}



