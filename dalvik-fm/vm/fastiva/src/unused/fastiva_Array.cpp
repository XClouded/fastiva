#include <precompiled_libcore.h>

#include <kernel/Kernel.h>
//#include <kernel/HeapMark.h>
#include <fox/Heap.h>

#include <java/lang/InternalError.inl>

#include <string.h>
#include <java/lang/Object.inl>
#include <java/lang/NegativeArraySizeException.inl>

/**
* method or field signatures.
*/


void fm::setArrayDimension(java_lang_Class_p pClass, int dimension) { 
	KASSERT(dimension <= 4);
	pClass->m_mark$ = (pClass->m_mark$ & ~0x0700) | (dimension << 8);
}

int fm::getArrayDimension(java_lang_Class_p pClass) { 
	return (pClass->m_mark$ & 0x0700) >> 8;
}

int fm::getArrayDimension_asObjectArray(
	java_lang_Class_p pClass
) { 
	int dim = fm::getArrayDimension(pClass);
	if (dim > 0) {
		if (pClass->m_pContext$->isPrimitive()) {
			dim --;
		}
	}
	return dim;
}


/**====================== body =======================**/


java_lang_Class_p fm::makeCompositeType(
	java_lang_Class_p itemType
) {
#ifdef _DEBUG
	if (0) {
		bool isPrimitive = itemType->m_pContext$->isPrimitive();
		int dim = fm::getArrayDimension(itemType);
		KASSERT(!isPrimitive || dim > 1);
		dim = fm::getArrayDimension(itemType);
	}
#endif
	java_lang_Class_p pArrayClass = itemType->m_pCompositeType$;

	if (pArrayClass == ADDR_ZERO) {
		pArrayClass = (java_lang_Class_p)fox_heap_malloc(sizeof(java_lang_Class));
		memcpy(pArrayClass, itemType, sizeof(java_lang_Class));
		//new (pArrayClass) fm::ArrayClass();
		//pArrayClass->m_pIVTables = kernelData.arrayIVTables;
		pArrayClass->obj.vtable$ = &kernelData.pointerArrayVTable;
		itemType->m_pCompositeType$ = (pArrayClass);
		pArrayClass->m_pComponentType$ = (itemType);
		int dimension = fm::getArrayDimension(itemType);
		fm::setArrayDimension(pArrayClass, dimension + 1);
		KASSERT(pArrayClass->m_pCompositeType$ == ADDR_ZERO);
	}
	KASSERT(fm::getArrayDimension(itemType) + 1 == fm::getArrayDimension(pArrayClass));
	return pArrayClass;
}


java_lang_Class_p fastiva_Runtime::getArrayClass(
	const fastiva_ClassContext* pContext,
	int dimension
) {
	KASSERT(dimension >= 0 && dimension <= FASTIVA_MAX_ARRAY_DIMENSION);
	//if (dimension < 0) {
	//	return (java_lang_Class_p)(FASTIVA_NULL);
	//}
	java_lang_Class_p pClass = pContext->m_pClass;
	KASSERT(pClass->m_pContext$ == pContext);

	if (dimension == 0) {
		return pClass;
	}

	java_lang_Class_p pArrayClass = fm::makeCompositeType(pClass);
	int d = 0;
	while (++d < dimension) {
		pArrayClass = fm::makeCompositeType(pArrayClass);
	}
	return pArrayClass;
}


fastiva_ArrayHeader* fastiva_Runtime::allocateInitializedArray(
	fastiva_PrimitiveClass_p pClass, 
	const void* data,
	int length
) {
	CHECK_STACK_FAST();
	
	java_lang_Object_ap pArray = (java_lang_Object_ap)
			fastiva.allocatePrimitiveArray(pClass, length);
	int item_size = fm::getPrimitiveArrayItemSize(pClass->m_pContext$);
	
	void* dst = fm::getUnsafeBuffer(pArray); // pArray 가 return value 로 사용된다.
#ifdef FASTIVA_NOT_SUPPORT_BOUND_CHECK
	if (item_size == 1) {
		dst = *(void**)dst;
	}
#endif
	memcpy(dst, data, length * item_size);
	return pArray;
}

/*
생성된 Array-Object는 (Item-Class가 Loading되지 않았어도)Class를 가져야 한다.
다음 문장은 Item-Class의 Loading 없이 수행이 가능하다.
        Base[] aBase = new Base[10];
        System.out.println("array of Base created.");
        System.out.println(aBase.getClass().toString());
        System.out.println("class-name of array of Base prinited.");
        aBase[0] = new Base(); // 이 때, cinit$() 가 수행된다.
*/



fastiva_ArrayHeader* fastiva_Runtime::allocatePrimitiveArray(
	fastiva_PrimitiveClass_p pArrayClass,
	int  length
) {
	if (length < 0) {
		THROW_EX_NEW$(java_lang_NegativeArraySizeException, ());
	}

	int item_size = fm::getPrimitiveArrayItemSize(pArrayClass->m_pContext$);
	int alloc_size = item_size * length;
	java_lang_Object_ap pArray = (java_lang_Object_ap)
		fm::allocArrayInstance(item_size, pArrayClass, length);
	return pArray;
}

fastiva_ArrayHeader* fastiva_Runtime::allocatePointerArray(
	const fastiva_ClassContext* pContext, 
	int  length
) {
	java_lang_Class* pArrayClass = fm::makeCompositeType(pContext->m_pClass);
	java_lang_Object_ap pArray = (java_lang_Object_ap)
		fm::allocArrayInstance(sizeof(void*), pArrayClass, length);
	if (ADDR_ZERO != FASTIVA_NULL) {//m_pContext > fm::jbool) {
		const void** pItem = (const void**)fm::getUnsafeBuffer(pArray); // pArray 는 return-value로 사용된다.
		for (int i = length; i > 0; i --) {
			*pItem = FASTIVA_NULL;
			pItem ++;
		}
	}
	return pArray;
}


fastiva_ArrayHeader* fastiva_Runtime::allocateMultiArray(
	const fastiva_ClassContext* pContext, 
	int dimension,
    const int* aLength
) {
	KASSERT(dimension > 1);
	java_lang_Class_p	pArrayClass = fastiva.getArrayClass(pContext, dimension);
	return fastiva.allocateMultiArray(pArrayClass, aLength);
}

fastiva_ArrayHeader* fm::allocateArray(
	java_lang_Class_p componentType, 
	int length
) {
	if (componentType->isPrimitive()) {
		return fastiva.allocatePrimitiveArray((fastiva_PrimitiveClass_p)componentType, length);
	}
	if (length < 0) {
		THROW_EX_NEW$(java_lang_NegativeArraySizeException, ());
	}

	int dimension = fm::getArrayDimension(componentType);
	java_lang_Class_p pArrayClass = fm::makeCompositeType(componentType);
	fastiva_ArrayHeader* pArray = fm::allocArrayInstance(sizeof(void*), pArrayClass, length);
	return (java_lang_Object_ap)pArray;
/*
	const fastiva_ClassContext *pContext = componentType->m_pContext$;

	if (dimension > 0) {
		dimension += 1;
		java_lang_Class_p	pArrayClass = fastiva.getArrayClass(pContext, dimension);
		int aLength[2];
		aLength[0] = length;
		aLength[1] = fastiva_NO_ARRAY_ITEM$;
		return (java_lang_Object_ap)fm::allocateMultiArray(
			pContext, dimension, aLength);
	}

	if (pContext->isPrimitive()) {
		return  (java_lang_Object_ap)fm::allocatePrimitiveArray(
			pContext->m_id, length);
	}
	else {
		return (java_lang_Object_ap)fm::allocatePointerArray(
			pContext, length);
	}
*/
}

fastiva_ArrayHeader* fastiva_Runtime::allocateMultiArray(
	java_lang_Class_p pArrayClass, 
    const int* aLength
) {
	CHECK_STACK_FAST();
	java_lang_Object_ap pArray;

	int array_length = *aLength++;
	if (array_length < 0) {
		// 가장 처음의 인자는 반드시 그 값을 가져야 한다.
		THROW_EX_NEW$(java_lang_NegativeArraySizeException, ());
	}


	pArray = (java_lang_Object_ap)fm::allocArrayInstance(sizeof(void*), pArrayClass, array_length);
	// item 값을 설정하기 전에 array-intialization을 마무리해야 한다.
	// m_pClass$와 m_length 등의 값을 미리 최기화해주지 않으면, 
	// allocation 도중 GC가 일어날 때, item 들이 GC 되어 버릴 수 있다.
	
	void** ppItem = (void**)fm::getUnsafeBuffer(pArray); // pArray는 return-value로 사용된다.
	int item_array_length = *aLength;
	pArrayClass = pArrayClass->m_pComponentType$;

	if (item_array_length == fastiva_NO_ARRAY_ITEM$) {
		if (ADDR_ZERO != FASTIVA_NULL) {
			const void** pItem = (const void**)fm::getUnsafeBuffer(pArray);
			for (int i = array_length; i > 0; i --) {
				*pItem = FASTIVA_NULL;
				pItem ++;
			}
		}
	}
	else if (pArrayClass->m_pComponentType$->m_pComponentType$ != ADDR_ZERO) {
		for (int i = array_length; i -- > 0; ) {
			fastiva_ArrayHeader* pItem = fastiva.allocateMultiArray(pArrayClass, aLength);
			pArray->setField_$$(pItem, ppItem);//, (fastiva_Instance_p)pItem, pArray);
			ppItem ++;
		}
	}
	else {
		const fastiva_ClassContext* pContext = pArrayClass->m_pContext$;
		int item_siz;
		if (pContext->isPrimitive()) {
			item_siz = fm::getPrimitiveArrayItemSize(pContext);
		}
		else {
			item_siz = sizeof(void*);
		}
		for (int i = array_length; i -- > 0; ) {
			java_lang_Object_ap pItemArray = (java_lang_Object_ap)
				fm::allocArrayInstance(item_siz, pArrayClass, item_array_length);
			pArray->setField_$$(pItemArray, ppItem);
			ppItem ++;
		}
	}
	return pArray;
}

//struct Interface_p$ {
//	java_lang_Object_p m_pObj;
//	void* m_dtable;
//};
/*
void* fm::getInterfaceArrayItem(
	const fastiva_ArrayHeader* pArray, 
	int idx, 
	const fastiva_ClassMoniker* pAccessMoniker
) {
	const fastiva_ClassContext* pContext = fm::loadMoniker(pAccessMoniker);
	return getInterfaceArrayItem(pArray, idx, pContext);
}

void* fm::getInterfaceArrayItem(
	const fastiva_ArrayHeader* pArrayHeader, 
	int idx, 
	const fastiva_ClassContext* accessContext
) {
	CHECK_STACK_FAST();
	// m_pItemContext와 accessContext가 모두 interface라 하더라도 
	// multi-inhritance에 의해 casting-pointer 가 달라질 수 있다.
	java_lang_Object_ap pArray = (java_lang_Object_ap)pArrayHeader;
	FASTIVA_CHECK_POINTER(pArray);
	pArray->checkBound(idx);

	fastiva_Instance_p pObj = fm::getPointerBuffer(pArray)[idx];
	return fm::checkImplemented(pObj, accessContext);
}
*/

/*
fm::Unknown_p fm::castArrayItem(
	java_lang_Object_p pItem,
	const fastiva_ClassContext* arrayContext
) {
	const fastiva_InstanceContext* itemContext = fm::getInstanceContext(pItem);
	if ((void*)itemContext != (void*)arrayContext) {
		int cast_offset = arrayContext->getCastingOffsetFrom(itemContext);
		if (cast_offset < 0) {
			fastiva_throwArrayStoreException();
		}
		fm::Unknown_p pRes = (fm::Unknown_p)((char*)pItem + cast_offset);
		//KASSERT(fm::getVirtualContext$(pRes) == arrayContext);
		return pRes;
	}
	return pItem;
}
*/

/*
void fm::setArrayItem(
	fastiva_ArrayHeader* pArrayHeader, 
	const fastiva_ClassMoniker* pAccessType,
	void* pItemSlot,
	fastiva_Instance_p pItem
) {
	fm::setArrayItem(pArrayHeader, fm::loadMoniker(pAccessType), 
		pItemSlot, pItem);
}
*/
FOX_BOOL FOX_FASTCALL(fox_heap_checkObject)(void* ptr);

void fastiva_Runtime::setArrayItem(
	fastiva_ArrayHeader* pArrayHeader, 
	const fastiva_ClassContext* pAccessType,
	void* pItemSlot,
	fastiva_Instance_p pItem
) {
	KASSERT(fox_heap_checkObject(pArrayHeader));
	KASSERT(fox_heap_checkObject(pArrayHeader));
	KASSERT(pItem == ADDR_ZERO || pItem->getClass$() == java_lang_Class_C$::getRawStatic$() || fox_heap_checkObject(pItem));
	java_lang_Object_p oldItem = *(java_lang_Object_p*)pItemSlot;
	KASSERT(oldItem == ADDR_ZERO || fox_heap_checkObject(oldItem) || oldItem->getClass$() == java_lang_Class_C$::getRawStatic$());

	CHECK_STACK_FAST();
	java_lang_Object_ap pArray = (java_lang_Object_ap)(pArrayHeader);
	FASTIVA_CHECK_POINTER(pArray);

	// Object 1D array를 비롯한 모든 Object-Array item 변경시 사용된다.
	// Object array의 실제적인 Item-Context는 Object가 아닐 수 있으므로.

	if (pItem != FASTIVA_NULL) {
		// AccessContext가 java_lang_Object가 아닌 경우, accessContext는 
		// arrayContext와 동일하거나 super-context 이므로,
		// compiler에 의해 다음의 조건이 만족된다.

		java_lang_Class_p pItemClass = pItem->getClass$();
		java_lang_Class_p pArrayClass = pArray->getClass();
		KASSERT(fm::getArrayDimension(pItemClass)
				== fm::getArrayDimension(pArrayClass)-1);

		const fastiva_ClassContext* arrayContext = pArrayClass->m_pContext$;
		if (arrayContext == pAccessType) {
			goto set_array_field;
		}
		/*
		const fastiva_Class_p pItemClass = pItem->getClass();
		fastiva_Instance_p pFirstItem = ((fastiva_Instance_p*)fm::getUnsafeBuffer(pArray))[0];
		// pArray가 다시 사용된다.

		if (pFirstItem != ADDR_ZERO && pFirstItem->getClass() == pItemClass) {
			goto set_array_field;
		}

		const fastiva_ClassContext* itemContext = pItemClass->m_pContext$;
		*/
		if (!arrayContext->isInstance(pItem)) {
			fastiva_throwArrayStoreException();
		}
	}
set_array_field:
	pArray->setField_$$((java_lang_Object_p)pItem, pItemSlot);//, (fastiva_Instance_p)pItem, pArray);
	return;
}

void fastiva_Runtime::setAbstractArrayItem(
	fastiva_ArrayHeader* pArrayHeader, 
	const fastiva_ClassContext* pBaseType,
	void* pItemSlot,
	fastiva_Instance_p pItem
) {

	CHECK_STACK_FAST();
	java_lang_Object_ap pArray = (java_lang_Object_ap)(pArrayHeader);
	FASTIVA_CHECK_POINTER(pArray);

	// Object 1D array를 비롯한 모든 Object-Array item 변경시 사용된다.
	// Object array의 실제적인 Item-Context는 Object가 아닐 수 있으므로.

	if (pItem != FASTIVA_NULL) {
		java_lang_Class_p pItemClass = pItem->getClass$();
		java_lang_Class_p pArrayClass = pArray->getClass();
		const fastiva_ClassContext* arrayContext = pArrayClass->m_pContext$;

		if (arrayContext == pBaseType) {
			// Compile 단계에서 모든 조건이 검사되었다.
#ifdef _DEBUG
			if (arrayContext == java_lang_Object::getRawContext$()) {
				KASSERT( fm::getArrayDimension_asObjectArray(pItemClass) + 1
						>= fm::getArrayDimension(pArrayClass));
			}
			else if (arrayContext != pItemClass->m_pContext$) { // clonable or serializable
				KASSERT( fm::getArrayDimension(pItemClass)
					>= fm::getArrayDimension(pArrayClass));
			}
			else { // clonable or serializable
				KASSERT( fm::getArrayDimension(pItemClass)
					>= fm::getArrayDimension(pArrayClass) - 1);
			}
#endif
			// Pure-Object-Arry에는 interface ref를 저장해선 안되고,
			pItem = pItem->getInstance$();
		}
		else {
			int itemDimension = fm::getArrayDimension(pArrayClass) - 1;
			if (fm::getArrayDimension(pItemClass) != itemDimension) {
				goto throw_array_store_exception;
			}

			const fastiva_ClassContext* itemContext = pItemClass->m_pContext$;
			if (arrayContext != itemContext && !arrayContext->isAssignableFrom(itemContext)) {
				goto throw_array_store_exception;
			}
		}
	}
	pArray->setField_$$((java_lang_Object_p)pItem, pItemSlot);
	return;

throw_array_store_exception:
	fastiva_throwArrayStoreException();
}




void fastiva_Runtime::setCustomArrayItem(
	fastiva_ArrayHeader * pArray,
	void* pItemSlot,
	fastiva_BytecodeProxy_p pItem
) {
	pArray->setField_$$(pItem, pItemSlot);//, (fastiva_Instance_p)pItem, pArray);
}

/*
static const fastiva_ImplementInfo aArrayIVTables = {
	{ FASTIVA_RAW_CLASS_CONTEXT_PTR(java_lang_Cloneable), FASTIVA_EMPTY_IVTABLE }, 
	{ FASTIVA_RAW_CLASS_CONTEXT_PTR(java_io_Serializable), FASTIVA_EMPTY_IVTABLE }, 
	{ NULL, NULL }
};
*/

void fm::initArrayVTable() {
	//kerenlData.arrayIVTables = fm::createIVtables(aArrayIVTables);

	java_lang_Object::VTABLE$* obj_vtable = (java_lang_Object::VTABLE$*)java_lang_Object_C$::getRawStatic$()->obj.vtable$;
	kernelData.primitiveArrayVTable = *obj_vtable;
	kernelData.primitiveArrayVTable.FASTIVA_METHOD_SLOT_NAME(ret_t, clone, 0, ()) = (void*)fm::cloneArray;

	kernelData.pointerArrayVTable = kernelData.primitiveArrayVTable;
	kernelData.pointerArrayVTable.scanInstance_$$ = (void*)fm::scanPointerArray;  

	// JavaScript 지원을 위한 부분이다. 추후 정리.
	//주의) g_pArrayClass_GenericVT는 initScript() 수행시에 변경된다.
	//      현재는 단순히 java_lang_Class의 vtable을 가지고 있다.
	kernelData.g_pArrayClass_GenericVT = *(int*)java_lang_Object_C$::getRawStatic$();

#ifdef FASTIVA_SUPPORT_JNI_STUB
	kernelData.proxyArrayVTable = *obj_vtable;
	kernelData.proxyArrayVTable.FASTIVA_METHOD_SLOT_NAME(ret_t, clone, 0, ()) = (void*)fm::cloneJavaProxyArray;
	kernelData.proxyArrayVTable.scanJavaProxyFields_$$ = (void*)fm::scanJavaProxyArray;
#endif
}