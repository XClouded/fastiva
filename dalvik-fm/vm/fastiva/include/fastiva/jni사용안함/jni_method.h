/**
* CallXXX<Type>Method(A,V)�� �����ϱ� ���ؼ� ���Ǵ� MACRO�̴�.
* void Type�� RETURN Type�� �ִ� Method�� �����ϱ� ���ؼ� ���� �Ǿ���.
* �̰��� ������ #include�Ǳ� ������ #ifndef __FOX_JNI_H�ۿ� ���Ǹ� �Ѵ�.
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
