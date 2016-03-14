#ifndef __FASTIVA_UTIL_H__
#define __FASTIVA_UTIL_H__


#include <fastiva/ClassContext.h>
#include <memory.h>
#include <string.h>

class fastiva_ResourceLoader {
public:
	fastiva_ResourceLoader() {
		m_pNextLoader = ADDR_ZERO;
	}

	fastiva_ResourceLoader* m_pNextLoader;
	virtual java_lang_String_p getURL(java_lang_String_p rscName) = 0;
	virtual Byte_ap loadResource(java_lang_String_p rscName) = 0;
};

struct fastiva_MemoryInputStream {
private:
	ubyte*  m_pStart;
	ubyte*  m_pCurr;
	ubyte*  m_pEnd;

protected:
	fastiva_MemoryInputStream(Byte_ap pData) {
		Byte_A::Buffer buf(pData);
		init((jbyte*)buf, pData->length());
	}

private:
	void init(void* pStart, int length) {
		this->m_pStart = (ubyte*)pStart;
		this->m_pCurr  = (ubyte*)pStart;
		this->m_pEnd   = (ubyte*)pStart + length;
	}

	ubyte* checkEOF(int length) {
		ubyte* src = m_pCurr;
		m_pCurr += length;
		checkEOF();
		return src;
	}

public:
	fastiva_MemoryInputStream(void* pStart, int length) {
		init(pStart, length);
	}

	void checkEOF() {
		if (m_pCurr > m_pEnd) {
			fastiva_throwIOException();
		}
	}

	virtual int getOffset() {
		return m_pCurr - m_pStart;
	}

	bool atEOF() {
		return m_pCurr >= m_pEnd;
	}

	void* getCurrentPoint() {
		return m_pCurr;
	}

	ubyte readUByte() {
		ubyte* src = checkEOF(1);
		ubyte v = *src;
		checkEOF();
		return v;
	}

	short readShort() {
		ubyte* src = checkEOF(2);
		short v = *src ++;
		v = (v << 8) + *src++;
		return v;
	}

	int readInt() {
		ubyte* src = checkEOF(4);
		int v = *src ++;
		v = (v << 8) + *src++;
		v = (v << 8) + *src++;
		v = (v << 8) + *src;
		return v;
	}

	jlonglong readLong64() {
		ubyte* src = checkEOF(8);
		jlonglong v = *src ++;
		v = (v << 8) + *src++;
		v = (v << 8) + *src++;
		v = (v << 8) + *src++;
		v = (v << 8) + *src++;
		v = (v << 8) + *src++;
		v = (v << 8) + *src++;
		v = (v << 8) + *src;
		return v;
	}

	void readFully(void* dst, int length) {
		ubyte* src = checkEOF(length);
		memcpy(dst, src, length);
	}

	void skipBytes(int length) {
		checkEOF(length);
	}
};

/*
struct OzFloatingDecimal {
	jlonglong m_significant;
	short m_exponent;
	short m_type;  

	double	  doubleValue();
};
*/

struct NumericParser {
	unicod* m_pFirstDigit;
	unicod* m_pDigit;
	unicod* m_pEnd;
	union {
		jint		m_int32;
		jlonglong	m_int64;
		jdouble		m_double;
	};
	
	enum NumberType {
		T_INT32,
		T_INT64,
		T_DOUBLE,
	};

	enum ParseMode {
		PARSE_INT32,
		PARSE_INT64,
		PARSE_HUGE_INT,
		PARSE_DECIMAL,
	};


	NumericParser(java_lang_String_p pString);

	bool isEmpty() {
		return m_pDigit == m_pEnd;
	}

	// 주의 JavaScript.parseInt()시에만 whiteSpace를 skip한다.
	bool skipWhitespaces();

	// return true if negative sign is detected.
	bool parseSign();

	int parseRadixPrefix();

	bool skipHexaPrefix();

	void cutDigitsAfterPoint();

	// 0 : int 32, 1 : int 64, 2 : double
	NumberType parseInteger(jbool isNegative, int radix, ParseMode mode);

	bool parsePureInteger(jbool isNegative);

	jdouble parseFloat(jbool isNegative);

	jdouble getCurrentValue(NumberType t);

private:
	jdouble getErrorValue(jbool isNegative, jbool isNetativeExponent);
	int skipFloatSymbol();
	bool skipKeyword(const unicod* pKeyword, int len);
	int parseSignificant();
	int skipDecimalDigits();
};


#endif // __FASTIVA_UTIL_H__
