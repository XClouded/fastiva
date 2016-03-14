#include <precompiled_libcore.h>

#include <kernel/Kernel.h>
#include <fastiva/JppException.h>

#define  PACKAGE_com_intwise_i18n
#include "java/lang/Runnable.h"
#include "java/lang/Runnable.inl"
#include <00_java_lang.inl>
#include <00_java_io.inl>
#if 0
#include <00_gnu_java_io.inl>
#include <00_gnu_java_io_decode.inl>
#include <00_gnu_java_io_encode.inl>

#include <string.h>





//------------------- 로컬 함수 선언 --------------------


//-cp C:\sdk\jdk1.3.1_01\jre\lib\classes -ad d:\java2c\browser\classes -od d:/fastiva2/FastCpp/WebJABI2 -immutable_inline -generate_all  -visible_branch -r org
//-cp C:\sdk\jdk1.3.1_01\jre\lib\classes -ad D:/japhar/Project/Japhar/Test/classes -od d:/fastiva2/FastCpp/Test -immutable_inline -generate_all   -visible_branch -r .



Byte_ap fm::charToByteArray(const unicod* pcode, int len);

Unicod_ap fm::byteToCharArray(const ubyte* pbyte, int len);

void fm::writeUnicod(java_io_OutputStream_p out, int ch1);
int fm::readUnicod(java_io_InputStream_p in);
int fm::getMBCSLength(const unicod* str, int len);
int fm::getUnicodeLength(const ubyte* str, int len);








/**====================== body =======================**/







/* 
not needed any more cause style of gnu_java_io_encode is different with com_sun_io
 - jay 2002.11.13


jint com_intwise_i18n_EUC_KR_Reader::read() {
	CHECK_STACK(0, "wish_i18n_EUC_KR_Reader::read");
	
	FASTIVA_SYNCHRONIZE_STATIC();

	return fm::readUnicod(this->m_in);
}
 
*/



/**
 * 내부의 입력스트림으로부터 인코딩된 데이터를 읽어서 주어진 유니코드 버퍼에 
 * 저장하고, 디코딩한 총 유니코드의 갯수를 리턴한다.
 * 
 * 실패했을 경우에는 -1 을 리턴한다.
 */
jint gnu_java_io_decode_DecoderEUC_1KR::read(
//	PTR$ this,
	Unicod_ap aUnicod, 
	jint offset, 
	jint length
) {
	this->assertOpened();
	
	if (length == 0) {
		return 0;
	}

	SYNCHRONIZED_STATIC$();
	FASTIVA_CHECK_POINTER(aUnicod);
	Unicod_A::Buffer buf(aUnicod, offset, length);
	unicod* pCode = buf;//aUnicod->getBuffer(offset, length);//buf;
	int ch_len = length;
	while (ch_len > 0) {
		unicod ch = fm::readUnicod(this->m_in);
		if (ch == (unicod)-1) {
			length -= ch_len;
			break;
		}
		*pCode++ = ch;
		ch_len --;
	}
	if (length == 0) {
		return -1;
	}
	return length;
}

/**
 * 주어진 바이트 배열이 유니코드배열로 몇개인지를 얻는다.
 *
 * 즉, 한글문자열을 유니코드문자열로 변환할경우 크기가 변하게 되는데
 * 그 크기를 얻는것이다. 
 */
jint gnu_java_io_decode_DecoderEUC_1KR::charsInByteArray(
	// PTR$ this,
	Byte_ap aByte, 
	int offset, 
	int length) 
{
	if (length == 0) {
		return 0;
	}
	
	FASTIVA_CHECK_POINTER(aByte);
	Byte_A::Buffer buf(aByte, offset, length);
	return fm::getUnicodeLength((ubyte*)(jbyte*)buf, length);
}


Unicod_ap
gnu_java_io_decode_DecoderEUC_1KR::convertToChars(
//	PTR$ this,
	Byte_ap aByte, 
	jint offset, 
	jint length
) { 
	FASTIVA_CHECK_POINTER(aByte);
	if (length == 0) {
		return Unicod_A::create$(0);
	}
	Byte_A::Buffer buf(aByte, offset, length);

	return fm::byteToCharArray((ubyte*)(jbyte*)buf, length);
}

extern "C" { 
	void debug_textout(const char* fmt, ...);
}


/* 
not needed any more cause style of gnu_java_io_encode is different with com_sun_io
 - jay 2002.11.13
 
 
void com_intwise_i18n_EUC_KR_Writer::write(jint ch1) {
	CHECK_STACK(0, "wish_i18n_EUC_KR_Writer::write");
	
	FASTIVA_SYNCHRONIZE_STATIC();

	fm::writeUnicod(this->m_out, ch1);

	return;
}


void com_intwise_i18n_EUC_KR_Writer::write(java_lang_String_p pString1, int offset, int length) {
	FASTIVA_SYNCHRONIZE_STATIC();
	if (length == 0) {
		return;
	}

	pString1->chk$p();
	unicod* pCode = pString1->getBuffer(offset, length);
	while (length -- > 0) {
		fm::writeUnicod(this->m_out, *pCode++);
	}
}
*/

void gnu_java_io_encode_EncoderEUC_1KR::write(
//	PTR$ this,
	Unicod_ap aUnicod, 
	jint offset, 
	jint length
) {
	this->assertOpened();

	if (length == 0) {
		return;
	}

	SYNCHRONIZED_STATIC$();
	Unicod_A::Buffer buf(aUnicod, offset, length);
	unicod* pCode = buf;//aUnicod->getBuffer(offset, length);//buf;;
	FASTIVA_ENTER_NATIVE_SECTION();
	while (length -- > 0) {
		fm::writeUnicod(this->m_out, *pCode++);
	}
	FASTIVA_LEAVE_NATIVE_SECTION();
}







/**
 * 주어진 유니코드 배열이 바이트배열로 몇 바이트인지를 얻는다.
 *
 * 즉, 유니코드문자열을 한글 문자열로 변환할경우 크기가 변하게게 되는데
 * 그 크기를 얻는것이다. 
 */
jint gnu_java_io_encode_EncoderEUC_1KR::bytesInCharArray(
//	PTR$ this,
	Unicod_ap aUnicod, 
	jint offset, 
	jint length
) {
	if (length == 0) {
		return 0;
	}

	Unicod_A::Buffer buf(aUnicod, offset, length);
	//unicod* pCode = aUnicod->getBuffer(offset, length);//buf;

	// call internal function
	return fm::getMBCSLength(buf, length);
}



Byte_ap gnu_java_io_encode_EncoderEUC_1KR::convertToBytes(
//	PTR$ this,
	Unicod_ap aUnicod, 
	jint uoffset, 
	jint ulen
) {
	// check null pointer
	FASTIVA_CHECK_POINTER(aUnicod);
	
	if (ulen == 0) {
		return Byte_A::create$(0);
	}

	//aUnicod->checkBound(uoffset, ulen);
	Unicod_A::Buffer buf(aUnicod, uoffset, ulen);
	//unicod* pU = aUnicod->getBuffer(uoffset, ulen);

	return fm::charToByteArray(buf, ulen);
}





#endif

#if 0
#include <sun/io/ByteToCharISO8859_1.inl>
#include <sun/io/CharToByteISO8859_1.inl>

jint sun_io_ByteToCharISO8859_11::convert(
	Byte_ap  aByte1,	// src
	jint  b_offset,
	jint  b_end,
	Unicod_ap  aUnicod4, // dst
	jint  c_offset,
	jint  c_end
) {
	int c_length = c_end - c_offset;
	int b_length = b_end - b_offset;
	if (b_length == 0) {
		return 0;//Unicod_A::create$(0);
	}
	Byte_A::Buffer b_buf(aByte1, b_offset, b_length);
	Unicod_A::Buffer c_buf(aUnicod4, c_offset, c_length);
	if (b_length > c_length) {
		b_length = c_length;
	}
	jbyte* pB = b_buf;
	unicod* pC = c_buf;
	for (int i = b_length; i -- > 0; ) {
		*pC ++ = *pB++;
	}
//*
	this->set__charOff(c_offset + b_length);
	return c_length;
/*/
	this->set__charOff(c_offset + b_length);
	return c_length;
//*/
	//return fm::byteToCharArray((ubyte*)(jbyte*)buf, length);
}

jint sun_io_CharToByteISO8859_11::convert(
	Unicod_ap  aUnicod1,	// src
	jint  c_offset,
	jint  c_end,
	Byte_ap  aByte4,		// dst
	jint  b_offset,
	jint  b_end
) {
	int c_length = c_end - c_offset;
	int b_length = b_end - b_offset;
	if (c_length == 0) {
		return 0;//Unicod_A::create$(0);
	}
	Byte_A::Buffer b_buf(aByte4, b_offset, b_length);
	Unicod_A::Buffer c_buf(aUnicod1, c_offset, c_length);
	if (c_length > b_length) {
		c_length = b_length;
	}
	jbyte* pB = b_buf;
	unicod* pC = c_buf;
	for (int i = c_length; i -- > 0; ) {
		*pB ++ = (jbyte)*pC++;
	}
	this->set__byteOff(b_offset + c_length);
	return c_length;
}

#endif
/**====================== end ========================**/