
#ifdef WIN32_MSG
	#pragma message ("--------- prolog " __FILE__)
#endif



#define FASTIVA_FIELD(type, acc, value_type, name)							\
	FASTIVA_JNI_INSTANCE_FIELD_INFO(type, value_type, name,					\
									FASTIVA_JNI_ACC_##acc)

#define FASTIVA_FIELD_EX(type, acc, value_type, name, flags)				\
	FASTIVA_JNI_INSTANCE_FIELD_INFO(type, value_type, name, 				\
									flags | FASTIVA_JNI_ACC_##acc)

#define FASTIVA_FIELD_VOLATILE(type, acc, value_type, name)					\
	FASTIVA_JNI_INSTANCE_FIELD_INFO(type, value_type, name, 				\
									ACC_VOLATILE$ | FASTIVA_JNI_ACC_##acc)

#define FASTIVA_FIELD_VOLATILE_EX(type, acc, value_type, name, flags)		\
	FASTIVA_JNI_INSTANCE_FIELD_INFO(type, value_type, name, 				\
									flags|ACC_VOLATILE$ | FASTIVA_JNI_ACC_##acc)


#define FASTIVA_FIELD_CONSTANT(type, acc, value_type, name)					\
	FASTIVA_JNI_CONSTANT_FIELD_INFO(type, value_type, name, 				\
									FASTIVA_JNI_ACC_##acc)

#define FASTIVA_FIELD_CONSTANT_EX(type, acc, value_type, name, flags)		\
	FASTIVA_JNI_CONSTANT_FIELD_INFO(type, value_type, name, 				\
									flags | FASTIVA_JNI_ACC_##acc)


#define FASTIVA_FIELD_STATIC(type, acc, value_type, name)					\
	FASTIVA_JNI_STATIC_FIELD_INFO(type, value_type, name, 					\
									FASTIVA_JNI_ACC_##acc)

#define FASTIVA_FIELD_STATIC_EX(type, acc, value_type, name, flags)			\
	FASTIVA_JNI_STATIC_FIELD_INFO(type, value_type, name, 					\
									flags | FASTIVA_JNI_ACC_##acc)

#define FASTIVA_FIELD_STATIC_VOLATILE(type, acc, value_type, name)			\
	FASTIVA_JNI_STATIC_FIELD_INFO(type, value_type, name, 					\
									ACC_VOLATILE$ | FASTIVA_JNI_ACC_##acc)

#define FASTIVA_FIELD_STATIC_VOLATILE_EX(type, acc, value_type, name, flags)\
	FASTIVA_JNI_STATIC_FIELD_INFO(type, value_type, name,  					\
									flags|ACC_VOLATILE$ | FASTIVA_JNI_ACC_##acc)

#if FASTIVA_USE_TOKENIZED_FIELD_INFO
static const Fastiva::FieldInfo FASTIVA_REVERSE_MERGE(FASTIVA_REVERSE_MERGE(
								$, FASTIVA_THIS_CLASS), aFieldInfo_)[] = {
#else
static Fastiva::FieldInfo FASTIVA_REVERSE_MERGE(FASTIVA_REVERSE_MERGE(
								$, FASTIVA_THIS_CLASS), aFieldInfo_)[] = {
#endif

#define FASTIVA_METHOD_CREATE(acc, cnt, sig, args)							\
		// IGNORE

#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
		// IGNORE

#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)				\
		// IGNORE

#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
		// IGNORE

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
		// IGNORE


