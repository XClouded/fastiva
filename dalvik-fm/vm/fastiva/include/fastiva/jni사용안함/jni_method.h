/**
* CallXXX<Type>Method(A,V)를 구현하기 위해서 사용되는 MACRO이다.
* void Type과 RETURN Type이 있는 Method를 구분하기 위해서 정의 되었다.
* 이것은 여러번 #include되기 때문에 #ifndef __FOX_JNI_H밖에 정의를 한다.
*
*/
#ifdef VOID_METHOD
	#ifdef NULL_OBJECT
		#undef NULL_OBJECT
	#endif
	#ifdef RETURN_OBJECT
		#undef RETURN_OBJECT
	#endif
	#define NULL_OBJECT
	#define RETURN_OBJECT
#else 
	#ifdef NULL_OBJECT
		#undef NULL_OBJECT
	#endif
	#ifdef RETURN_OBJECT
		#undef RETURN_OBJECT
	#endif

	#define NULL_OBJECT  NULL
	#define RETURN_OBJECT return
#endif
