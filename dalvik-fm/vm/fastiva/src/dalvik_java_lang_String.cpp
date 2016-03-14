#include <Fastiva.h>
#include <java/lang/String.inl>
#include <java/lang/StringIndexOutOfBoundsException.inl>

/*
unicod fastiva_String_charAt(java_lang_String_p arg0, jint arg1) {
    int count, offset;
    ArrayObject* chars;

    count = arg0->get__count(); // @zee may throw null pointer exception 
    if ((s4) arg1 < 0 || (s4) arg1 >= count) {
        THROW_EX_NEW$(java_lang_StringIndexOutOfBoundsException, (count, arg1));
    } else {
        offset = arg0->get__offset();
        chars = (ArrayObject*)arg0->get__value();
		return ((const u2*)(void*)chars->contents)[arg1 + offset];
    }
}
*/

jint fastiva_String_compareTo(java_lang_String_p arg0, java_lang_String_p arg1) {
    int thisCount, thisOffset, compCount, compOffset;
    ArrayObject* thisArray;
    ArrayObject* compArray;
    const u2* thisChars;
    const u2* compChars;
    int minCount, countDiff;

    thisCount = arg0->get__count(); // @zee may throw null pointer exception 
    compCount = arg1->get__count(); // @zee may throw null pointer exception 

    if (arg0 == arg1) {
        return 0;
    }

    countDiff = thisCount - compCount;
    minCount = (countDiff < 0) ? thisCount : compCount;
    thisOffset = arg0->get__offset();
    compOffset = arg1->get__offset();
    thisArray = (ArrayObject*)arg0->get__value();
    compArray = (ArrayObject*)arg1->get__value();
    thisChars = ((const u2*)(void*)thisArray->contents) + thisOffset;
    compChars = ((const u2*)(void*)compArray->contents) + compOffset;

    int i;
    for (i = 0; i < minCount; i++) {
		int res;
        if ((res = thisChars[i] - compChars[i]) != 0) {
            return res;
        }
    }
	return countDiff;
}

jbool fastiva_String_equals(java_lang_String_p arg0, java_lang_Object_p arg1) {
    /*
     * This would be simpler and faster if we promoted StringObject to
     * a full representation, lining up the C structure fields with the
     * actual object fields.
     */
    int thisCount, thisOffset, compCount, compOffset;
    ArrayObject* thisArray;
    ArrayObject* compArray;
    const u2* thisChars;
    const u2* compChars;

    thisCount = arg0->get__count(); // zee can throw null pointer exception 
	
    if (arg0 == arg1) {
        return true;
    }

	if (arg1 == 0 || ((Object*) arg0)->clazz != ((Object*) arg1)->clazz) {
        return false;
    }

    /* quick length check */
    compCount = ((java_lang_String_p)arg1)->get__count();
    if (thisCount != compCount) {
        return false;
    }

    thisOffset = arg0->get__offset();
    compOffset = ((java_lang_String_p)arg1)->get__offset();
    thisArray = (ArrayObject*)arg0->get__value();
    compArray = (ArrayObject*)((java_lang_String_p)arg1)->get__value();
    thisChars = ((const u2*)(void*)thisArray->contents) + thisOffset;
    compChars = ((const u2*)(void*)compArray->contents) + compOffset;

    int i;
    //for (i = 0; i < thisCount; i++)
    for (i = thisCount-1; i >= 0; --i) {
        if (thisChars[i] != compChars[i]) {
            return false;
        }
    }
    return true;
}

jint fastiva_String_fastIndexOf(java_lang_String_p self, jint ch, jint start) {
    ArrayObject* charArray = (ArrayObject*)self->get__value();
    const u2* chars = (const u2*)(void*)charArray->contents;
    int offset = self->get__offset();
    int count = self->get__count();

	chars += offset;

    if (start < 0)
        start = 0;
    else if (start > count)
        start = count;

    /* 16-bit loop, slightly better on ARM */
    const u2* ptr = chars + start;
    const u2* endPtr = chars + count;
    while (ptr < endPtr) {
        if (*ptr++ == ch)
            return (ptr-1) - chars;
    }

    return -1;
}

/*
jbool fastiva_String_isEmpty(java_lang_String_p self) {
    return self->get__count() == 0;
}

jint fastiva_String_length(java_lang_String_p self) {
	return self->get__count();
}


jint fastiva_Dalvik_java_lang_System_identityHashCode(java_lang_Object_p obj) {
	return (jint)obj;
}
*/