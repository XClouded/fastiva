#if JPP_LIBCORE_MODULE_TYPE == JPP_MODULE_TYPE_LIBRARY

#ifdef _WIN32
		#pragma const_seg(JPP_CURRENT_PACKAGE_SEGMENT)

		// class info array �� ���� marking�Ѵ�.
		//__declspec(allocate(JPP_CURRENT_PACKAGE_SEGMENT)) 
		const int FASTIVA_MERGE_TOKEN(zzzzzz_, JPP_CURRENT_PACKAGE)[1] = { 0 };
		//#pragma const_seg()


		//__declspec(allocate(JPP_CURRENT_PACKAGE_SEGMENT)) 
		const fastiva_PackageInfo FASTIVA_MERGE_TOKEN(JPP_CURRENT_PACKAGE, _Package$) = {								
			FASTIVA_MERGE_TOKEN(JPP_CURRENT_PACKAGE, _hashCode$), 
			FASTIVA_MERGE_TOKEN(JPP_CURRENT_PACKAGE, _path$), 
			(const JNI_HashEntry*)(&FASTIVA_MERGE_TOKEN(A00000_, JPP_CURRENT_PACKAGE) + 1), 
			(const JNI_HashEntry*)(&FASTIVA_MERGE_TOKEN(zzzzzz_, JPP_CURRENT_PACKAGE)), 
			&fastiva_ModuleInfo
		};
		#pragma const_seg()

#undef  PACKAGE_SEG_NAME
#define PACKAGE_SEG_NAME FASTIVA_MERGE_3TOKEN(FASTIVA_MERGE_TOKEN, $, FASTIVA_MERGE_TOKEN(FASTIVA_MERGE_TOKEN, _hashCode$)) 
#define JPP_PACKAGE_SEG_NAME FASTIVA_MAKE_STR(PACKAGE_SEG_NAME)
		#pragma const_seg(JPP_PACKAGE_SEG_NAME)

		struct BBB { void* b; int c; };
		extern const BBB FASTIVA_MERGE_TOKEN(JPP_CURRENT_PACKAGE, _PackageSlot$);
		//__declspec(allocate(FASTIVA_PACKAGE_INFO_SECTION$)) 
		const BBB FASTIVA_MERGE_TOKEN(JPP_CURRENT_PACKAGE, _PackageSlot$) = {
			(void*)&FASTIVA_MERGE_TOKEN(JPP_CURRENT_PACKAGE, _Package$)
		};

		#pragma const_seg()


#elif defined(FASTIVA_TARGET_OS_LINUX)
	#ifdef EMBEDDED_RUNTIME
		/*
		gcc������ Win32�� Ʋ���� const_seg()ó�� __attribute__((section()))�� ���� �ʾƵ� �ȴ�.
		���� ���� ���� �Ұ�� error�� ���´�.
		*/
	#else 
	#endif
#else
	#error unknown target
#endif

#endif
