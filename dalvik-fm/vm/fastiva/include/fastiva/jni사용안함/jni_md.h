/*
 * @(#)jni_md.h	1.12 01/12/03
 *
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _JAVASOFT_JNI_MD_H_
#define _JAVASOFT_JNI_MD_H_

#ifdef _WIN32
    #define JNIEXPORT __declspec(dllexport)
    #define JNIIMPORT __declspec(dllimport)
    #define JNICALL __stdcall
#else
	#define JNIEXPORT
	#define JNIIMPORT
	#define JNICALL
#endif

// original JDK's jint was just "long" change it to "signed int" for fastiva.
#ifndef EMBEDDED_RUNTIME
	// typedef long jint;
#endif

#ifdef __GNUC__
typedef long long jlong;
#else
typedef __int64 jlong;
#endif

typedef signed char jbyte;

#endif /* !_JAVASOFT_JNI_MD_H_ */
