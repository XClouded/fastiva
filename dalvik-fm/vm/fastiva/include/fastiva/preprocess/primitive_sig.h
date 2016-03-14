#ifndef __FASTIVA_PROLOG_PRIMITIVE_SIG_H__
#define __FASTIVA_PROLOG_PRIMITIVE_SIG_H__

#define FASTIVA_SIG_jbool				("[[[[Z"+4)
#define FASTIVA_SIG_jbyte				("[[[[B"+4)
#define FASTIVA_SIG_jshort				("[[[[S"+4)
#define FASTIVA_SIG_unicod				("[[[[C"+4)
#define FASTIVA_SIG_jint				("[[[[I"+4)
#define FASTIVA_SIG_jlonglong			("[[[[J"+4)
#define FASTIVA_SIG_jfloat				("[[[[F"+4)
#define FASTIVA_SIG_jdouble				("[[[[D"+4)
#define FASTIVA_SIG_void				("V")


#define FASTIVA_SIG_jbool_p				("[[[[Z"+4)
#define FASTIVA_SIG_jbyte_p				("[[[[B"+4)
#define FASTIVA_SIG_jshort_p				("[[[[S"+4)
#define FASTIVA_SIG_unicod_p				("[[[[C"+4)
#define FASTIVA_SIG_jint_p				("[[[[I"+4)
#define FASTIVA_SIG_jlonglong_p			("[[[[J"+4)
#define FASTIVA_SIG_jfloat_p				("[[[[F"+4)
#define FASTIVA_SIG_jdouble_p				("[[[[D"+4)
#define FASTIVA_SIG_void_p				("V")
#define FASTIVA_SIG_void_p_p				("V")

#define FASTIVA_SIG_VAL$(type)			FASTIVA_SIG_##type##_p
#define FASTIVA_SIG_JOBJ$(type)			FASTIVA_SIG_##type##_p
#define FASTIVA_SIG_PTR$(type)			FASTIVA_SIG_##type##_p
#define FASTIVA_SIG_ARRAY$(dim, type)	(FASTIVA_SIG_PTR$(type) - dim)


/*
#define FASTIVA_PRIMITIVE_CLASS_NAME_jbool				Boolean
#define FASTIVA_PRIMITIVE_CLASS_NAME_jbyte				Byte
#define FASTIVA_PRIMITIVE_CLASS_NAME_jshort				Short
#define FASTIVA_PRIMITIVE_CLASS_NAME_unicod				Char
#define FASTIVA_PRIMITIVE_CLASS_NAME_jint				Int
#define FASTIVA_PRIMITIVE_CLASS_NAME_jlonglong			Long
#define FASTIVA_PRIMITIVE_CLASS_NAME_jfloat				Float
#define FASTIVA_PRIMITIVE_CLASS_NAME_jdouble			Double
#define FASTIVA_PRIMITIVE_CLASS_NAME_void				Void

#define jbool_PARAM_T$				    jbool
#define jbyte_PARAM_T$					jbyte
#define jshort_PARAM_T$					jshort
#define unicod_PARAM_T$					unicod
#define jint_PARAM_T$					jint
#define jlonglong_PARAM_T$				jlonglong
#define jfloat_PARAM_T$					jfloat
#define jdouble_PARAM_T$				jdouble
#define void_PARAM_T$					void



#define FASTIVA_PARAM_TYPE_jbool			0
#define FASTIVA_PARAM_TYPE_jbyte			0
#define FASTIVA_PARAM_TYPE_jshort			0
#define FASTIVA_PARAM_TYPE_unicod			0
#define FASTIVA_PARAM_TYPE_jint				0
#define FASTIVA_PARAM_TYPE_jlonglong		2
#define FASTIVA_PARAM_TYPE_jfloat			1
#define FASTIVA_PARAM_TYPE_jdouble			3
#define FASTIVA_PARAM_TYPE_void				0

#define FASTIVA_PARAM_TYPE_Bool_A			0
#define FASTIVA_PARAM_TYPE_Byte_A			0
#define FASTIVA_PARAM_TYPE_Short_A			0
#define FASTIVA_PARAM_TYPE_Unicod_A			0
#define FASTIVA_PARAM_TYPE_Int_A			0
#define FASTIVA_PARAM_TYPE_Longlong_A		0
#define FASTIVA_PARAM_TYPE_Float_A			0
#define FASTIVA_PARAM_TYPE_Double_A			0

#define FASTIVA_PARAM_TYPE_Bool_AA			0
#define FASTIVA_PARAM_TYPE_Byte_AA			0
#define FASTIVA_PARAM_TYPE_Short_AA			0
#define FASTIVA_PARAM_TYPE_Unicod_AA		0
#define FASTIVA_PARAM_TYPE_Int_AA			0
#define FASTIVA_PARAM_TYPE_Longlong_AA		0
#define FASTIVA_PARAM_TYPE_Float_AA			0
#define FASTIVA_PARAM_TYPE_Double_AA		0

#define FASTIVA_PARAM_TYPE_Bool_AAA			0
#define FASTIVA_PARAM_TYPE_Byte_AAA			0
#define FASTIVA_PARAM_TYPE_Short_AAA		0
#define FASTIVA_PARAM_TYPE_Unicod_AAA		0
#define FASTIVA_PARAM_TYPE_Int_AAA			0
#define FASTIVA_PARAM_TYPE_Longlong_AAA		0
#define FASTIVA_PARAM_TYPE_Float_AAA		0
#define FASTIVA_PARAM_TYPE_Double_AAA		0

#define FASTIVA_PARAM_TYPE_Bool_AAAA		0
#define FASTIVA_PARAM_TYPE_Byte_AAAA		0
#define FASTIVA_PARAM_TYPE_Short_AAAA		0
#define FASTIVA_PARAM_TYPE_Unicod_AAAA		0
#define FASTIVA_PARAM_TYPE_Int_AAAA			0
#define FASTIVA_PARAM_TYPE_Longlong_AAAA	0
#define FASTIVA_PARAM_TYPE_Float_AAAA		0
#define FASTIVA_PARAM_TYPE_Double_AAAA		0
/*/
#define FASTIVA_SIG_Bool_p			FASTIVA_SIG_jbool
#define FASTIVA_SIG_Byte_p			FASTIVA_SIG_jbyte
#define FASTIVA_SIG_Short_p			FASTIVA_SIG_jshort
#define FASTIVA_SIG_Unicod_p		FASTIVA_SIG_unicod
#define FASTIVA_SIG_Int_p			FASTIVA_SIG_jint
#define FASTIVA_SIG_Longlong_p		FASTIVA_SIG_jlonglong
#define FASTIVA_SIG_Float_p			FASTIVA_SIG_jfloat
#define FASTIVA_SIG_Double_p		FASTIVA_SIG_jdouble

#if 0
#define FASTIVA_SIG_Bool_ap				(FASTIVA_SIG_jbool - 1)
#define FASTIVA_SIG_Byte_ap				(FASTIVA_SIG_jbyte - 1)
#define FASTIVA_SIG_Short_ap			(FASTIVA_SIG_jshort - 1)
#define FASTIVA_SIG_Unicod_ap			(FASTIVA_SIG_unicod	- 1)
#define FASTIVA_SIG_Int_ap				(FASTIVA_SIG_jint - 1)
#define FASTIVA_SIG_Longlong_ap			(FASTIVA_SIG_jlonglong - 1)
#define FASTIVA_SIG_Float_ap			(FASTIVA_SIG_jfloat - 1)
#define FASTIVA_SIG_Double_ap			(FASTIVA_SIG_jdouble - 1)

#define FASTIVA_SIG_Bool_aap			(FASTIVA_SIG_jbool - 2)
#define FASTIVA_SIG_Byte_aap			(FASTIVA_SIG_jbyte - 2)
#define FASTIVA_SIG_Short_aap			(FASTIVA_SIG_jshort - 2)
#define FASTIVA_SIG_Unicod_aap			(FASTIVA_SIG_unicod - 2)
#define FASTIVA_SIG_Int_aap				(FASTIVA_SIG_jint - 2)
#define FASTIVA_SIG_Longlong_aap		(FASTIVA_SIG_jlonglong - 2)
#define FASTIVA_SIG_Float_aap			(FASTIVA_SIG_jfloat - 2)
#define FASTIVA_SIG_Double_aap			(FASTIVA_SIG_jdouble - 2)

#define FASTIVA_SIG_Bool_aaap			(FASTIVA_SIG_jbool - 3)
#define FASTIVA_SIG_Byte_aaap			(FASTIVA_SIG_jbyte - 3)
#define FASTIVA_SIG_Short_aaap			(FASTIVA_SIG_jshort - 3)
#define FASTIVA_SIG_Unicod_aaap			(FASTIVA_SIG_unicod - 3)
#define FASTIVA_SIG_Int_aaap			(FASTIVA_SIG_jint - 3)
#define FASTIVA_SIG_Longlong_aaap		(FASTIVA_SIG_jlonglong - 3)
#define FASTIVA_SIG_Float_aaap			(FASTIVA_SIG_jfloat - 3)
#define FASTIVA_SIG_Double_aaap			(FASTIVA_SIG_jdouble - 3)

#define FASTIVA_SIG_Bool_aaaap			(FASTIVA_SIG_jbool - 4)
#define FASTIVA_SIG_Byte_aaaap			(FASTIVA_SIG_jbyte - 4)
#define FASTIVA_SIG_Short_aaaap			(FASTIVA_SIG_jshort - 4)
#define FASTIVA_SIG_Unicod_aaaap		(FASTIVA_SIG_unicod - 4)
#define FASTIVA_SIG_Int_aaaap			(FASTIVA_SIG_jint - 4)
#define FASTIVA_SIG_Longlong_aaaap		(FASTIVA_SIG_jlonglong - 4)
#define FASTIVA_SIG_Float_aaaap			(FASTIVA_SIG_jfloat - 4)
#define FASTIVA_SIG_Double_aaaap		(FASTIVA_SIG_jdouble - 4)

//#if 0 //ANDROID
#define FASTIVA_SIG_Bool_A_p				"[" FASTIVA_SIG_jbool
#define FASTIVA_SIG_Byte_A_p				"[" FASTIVA_SIG_jbyte
#define FASTIVA_SIG_Short_A_p				"[" FASTIVA_SIG_jshort	
#define FASTIVA_SIG_Unicod_A_p			"[" FASTIVA_SIG_unicod	
#define FASTIVA_SIG_Int_A_p				"[" FASTIVA_SIG_jint	
#define FASTIVA_SIG_Longlong_A_p			"[" FASTIVA_SIG_jlonglong
#define FASTIVA_SIG_Float_A_p				"[" FASTIVA_SIG_jfloat
#define FASTIVA_SIG_Double_A_p			"[" FASTIVA_SIG_jdouble

#define FASTIVA_SIG_Bool_AA_p				"[[" FASTIVA_SIG_jbool
#define FASTIVA_SIG_Byte_AA_p				"[[" FASTIVA_SIG_jbyte	
#define FASTIVA_SIG_Short_AA_p			"[[" FASTIVA_SIG_jshort
#define FASTIVA_SIG_Unicod_AA_p			"[[" FASTIVA_SIG_unicod
#define FASTIVA_SIG_Int_AA_p				"[[" FASTIVA_SIG_jint
#define FASTIVA_SIG_Longlong_AA_p			"[[" FASTIVA_SIG_jlonglong
#define FASTIVA_SIG_Float_AA_p			"[[" FASTIVA_SIG_jfloat
#define FASTIVA_SIG_Double_AA_p			"[[" FASTIVA_SIG_jdouble

#define FASTIVA_SIG_Bool_AAA_p			"[[[" FASTIVA_SIG_jbool
#define FASTIVA_SIG_Byte_AAA_p			"[[[" FASTIVA_SIG_jbyte
#define FASTIVA_SIG_Short_AAA_p			"[[[" FASTIVA_SIG_jshort	
#define FASTIVA_SIG_Unicod_AAA_p			"[[[" FASTIVA_SIG_unicod	
#define FASTIVA_SIG_Int_AAA_p				"[[[" FASTIVA_SIG_jint
#define FASTIVA_SIG_Longlong_AAA_p		"[[[" FASTIVA_SIG_jlonglong
#define FASTIVA_SIG_Float_AAA_p			"[[[" FASTIVA_SIG_jfloat	
#define FASTIVA_SIG_Double_AAA_p			"[[[" FASTIVA_SIG_jdouble

#define FASTIVA_SIG_Bool_AAAA_p			"[[[[" FASTIVA_SIG_jbool
#define FASTIVA_SIG_Byte_AAAA_p			"[[[[" FASTIVA_SIG_jbyte
#define FASTIVA_SIG_Short_AAAA_p			"[[[[" FASTIVA_SIG_jshort
#define FASTIVA_SIG_Unicod_AAAA_p			"[[[[" FASTIVA_SIG_unicod
#define FASTIVA_SIG_Int_AAAA_p			"[[[[" FASTIVA_SIG_jint
#define FASTIVA_SIG_Longlong_AAAA_p		"[[[[" FASTIVA_SIG_jlonglong
#define FASTIVA_SIG_Float_AAAA_p			"[[[[" FASTIVA_SIG_jfloat
#define FASTIVA_SIG_Double_AAAA_p			"[[[[" FASTIVA_SIG_jdouble
#endif

#define FASTIVA_PARAM_TYPE_jbool			0
#define FASTIVA_PARAM_TYPE_jbyte			0
#define FASTIVA_PARAM_TYPE_jshort			0
#define FASTIVA_PARAM_TYPE_unicod			0
#define FASTIVA_PARAM_TYPE_jint				0
#define FASTIVA_PARAM_TYPE_jlonglong		2
#define FASTIVA_PARAM_TYPE_jfloat			1
#define FASTIVA_PARAM_TYPE_jdouble			3
#define FASTIVA_PARAM_TYPE_void				0

#define FASTIVA_PARAM_TYPE_Bool_ap			0
#define FASTIVA_PARAM_TYPE_Byte_ap			0
#define FASTIVA_PARAM_TYPE_Short_ap			0
#define FASTIVA_PARAM_TYPE_Unicod_ap		0
#define FASTIVA_PARAM_TYPE_Int_ap			0
#define FASTIVA_PARAM_TYPE_Longlong_ap		0
#define FASTIVA_PARAM_TYPE_Float_ap			0
#define FASTIVA_PARAM_TYPE_Double_ap		0

#define FASTIVA_PARAM_TYPE_Bool_aap			0
#define FASTIVA_PARAM_TYPE_Byte_aap			0
#define FASTIVA_PARAM_TYPE_Short_aap		0
#define FASTIVA_PARAM_TYPE_Unicod_aap		0
#define FASTIVA_PARAM_TYPE_Int_aap			0
#define FASTIVA_PARAM_TYPE_Longlong_aap		0
#define FASTIVA_PARAM_TYPE_Float_aap		0
#define FASTIVA_PARAM_TYPE_Double_aap		0

#define FASTIVA_PARAM_TYPE_Bool_aaap		0
#define FASTIVA_PARAM_TYPE_Byte_aaap		0
#define FASTIVA_PARAM_TYPE_Short_aaap		0
#define FASTIVA_PARAM_TYPE_Unicod_aaap		0
#define FASTIVA_PARAM_TYPE_Int_aaap			0
#define FASTIVA_PARAM_TYPE_Longlong_aaap	0
#define FASTIVA_PARAM_TYPE_Float_aaap		0
#define FASTIVA_PARAM_TYPE_Double_aaap		0

#define FASTIVA_PARAM_TYPE_Bool_aaaap		0
#define FASTIVA_PARAM_TYPE_Byte_aaaap		0
#define FASTIVA_PARAM_TYPE_Short_aaaap		0
#define FASTIVA_PARAM_TYPE_Unicod_aaaap		0
#define FASTIVA_PARAM_TYPE_Int_aaaap		0
#define FASTIVA_PARAM_TYPE_Longlong_aaaap	0
#define FASTIVA_PARAM_TYPE_Float_aaaap		0
#define FASTIVA_PARAM_TYPE_Double_aaaap		0

//*/

#endif // __FASTIVA_PROLOG_PRIMITIVE_SIG_H__