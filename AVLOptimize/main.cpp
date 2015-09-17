//
//  main.cpp
//  AVLOptimize
//
//  Created by tiff on 9/16/15.
//  Copyright (c) 2015 icondb. All rights reserved.
//

#include <iostream>

struct DKMemoryDefaultAllocator
{
	static void* Alloc(size_t s)	{return ::malloc(s);}
	static void Free(void* p)		{::free(p);}
};

template <typename T> struct DKTypeTraits
{
};

template <typename T> struct DKFunctionType
{
	struct Signature
	{
		template <typename ...Ts>
		constexpr static bool CanInvokeWithParameterTypes(void)
		{
			return true;
		}
	};
};

#define FORCEINLINE


#include "DKAVLTree.h"
#include "DKAVLTree2.h"

template <typename V, typename K> using Tree1 = DKFoundation::DKAVLTree<V, K>;
template <typename V, typename K> using Tree2 = DKFoundation2::DKAVLTree<V, K>;

template <typename T, size_t Num> size_t NumArrayItems(T(&)[Num])
{
	return Num;
}

int main(int argc, const char * argv[]) {
	// insert code here...
	std::cout << "Hello, World!\n";

	Tree1<int, int> t1;
	Tree2<int, int> t2;

	int aa[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
	printf("num array: %lu\n", NumArrayItems(aa));

	for (int i : aa)
	{
		t1.Insert(i);
		t2.Insert(i);
	}

    return 0;
}
