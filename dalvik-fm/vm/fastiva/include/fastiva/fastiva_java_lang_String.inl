inline
JPP_RETURNS(VAL$(jint))
java_lang_String_I$::length_(
	java_lang_String_p self
) {
	return self->get__count();
}

inline
JPP_RETURNS(VAL$(jbool))
java_lang_String_I$::isEmpty_(
	java_lang_String_p self 
) {
	return self->get__count() == 0;
}

inline
JPP_RETURNS(VAL$(unicod))
java_lang_String_I$::charAt_(
	java_lang_String_p self, 
	jint idx
) {
    int count, offset;
    Unicod_ap chars;

    count = self->get__count(); // @zee may throw null pointer exception 
    if ((u4)idx >= (u4)count) {
		fastiva.throwStringIndexOutOfBoundsException(count, idx);
    } else {
        offset = self->get__offset();
        chars = self->get__value();
		return chars->get_unsafe$(offset+idx);
    }
}
