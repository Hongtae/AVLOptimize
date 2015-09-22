//
//  main.cpp
//  AVLOptimize
//
//  Created by tiff on 9/16/15.
//  Copyright (c) 2015 icondb. All rights reserved.
//

#include <iostream>
#include <vector>

struct DKMemoryDefaultAllocator
{
	static void* Alloc(size_t s)	{return ::malloc(s);}
	static void Free(void* p)		{::free(p);}
	static void* Realloc(void* p, size_t s)	{return ::realloc(p, s);}
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

#ifdef DEBUG
#define FORCEINLINE
const int debugMode = true;
#else
#define FORCEINLINE __attribute__((always_inline))
const int debugMode = false;
#endif


#include "DKAVLTree.h"
#include "DKAVLTree2.h"

#include "DKTimer.h"
#include "DKFixedSizeAllocator.h"


using Tree1Alloc = DKFoundation::DKFixedSizeAllocator<DKFoundation::DKAVLTree<u_int32_t, u_int32_t>::NodeSize()>;
using Tree2Alloc = DKFoundation::DKFixedSizeAllocator<DKFoundation2::DKAVLTree<u_int32_t>::NodeSize()>;

Tree1Alloc t1alloc;
Tree2Alloc t2alloc;


struct Tree1Allocator
{
	static void* Alloc(size_t s) { return t1alloc.Alloc(s); }
	static void Free(void* p)		{ t1alloc.Dealloc(p); }
};

struct Tree2Allocator
{
	static void* Alloc(size_t s) { return t2alloc.Alloc(s); }
	static void Free(void* p)		{ t2alloc.Dealloc(p); }
};

using Tree1 = DKFoundation::DKAVLTree<u_int32_t, u_int32_t,
	DKFoundation::DKTreeComparison<u_int32_t, u_int32_t>,
	DKFoundation::DKTreeComparison<u_int32_t, u_int32_t>,
	DKFoundation::DKTreeCopyValue<u_int32_t>, Tree1Allocator>;

using Tree2 = DKFoundation2::DKAVLTree<u_int32_t>;

using Timer = DKFoundation::DKTimer;

template <typename T, size_t Num> size_t NumArrayItems(T(&)[Num])
{
	return Num;
}

template <typename Node>
void EnumerateTreeNode(Node* node, std::vector<Node*>& vec)
{
	if (node->left)
		EnumerateTreeNode(node->left, vec);
	vec.push_back(node);
	if (node->right)
		EnumerateTreeNode(node->right, vec);
}

int main(int argc, const char * argv[]) {
	// insert code here...

	printf("Debug Mode: %d\n", debugMode);

	size_t numSamples = 0x1ffffff;
	std::vector<u_int32_t> samples;
	samples.reserve(numSamples);
	for (size_t i = 0; i < numSamples; ++i)
	{
		u_int32_t v = arc4random();
		v = v % 0xffff;
		samples.push_back(v);
	}

	printf("Reserving memory...\n");
	t1alloc.Reserve(numSamples);
	t2alloc.Reserve(numSamples);
	printf("Done!\n");

	Tree1 t1;
	Tree2 t2;

	auto test1 = [&]()
	{
		Timer timer;

		t1.Update(1);	// warm up.
		t1.Clear();

		printf("Testing tree1... (%lu items)\n", samples.size());

		timer.Reset();
		for (u_int32_t v : samples)
		{
			if (!t1.Insert(v))
			{
				t1.Remove(v);
			}
		}
		double d = timer.Elapsed();
		printf("tree1 insert elapsed: %f\n", d);
	};

	auto test2 = [&]()
	{
		Timer timer;
		auto t2Comp = DKFoundation::DKTreeComparison<u_int32_t, uint32_t>();

		t2.Update(1);	// warm up.
		t2.Clear();

		printf("Testing tree2... (%lu items)\n", samples.size());
		timer.Reset();
		for (u_int32_t v : samples)
		{
			if (!t2.Insert(v))
			{
				t2.Remove(v, t2Comp);
			}
		}
		double d = timer.Elapsed();
		printf("tree2 insert elapsed: %f\n", d);
	};

	if (arc4random() % 2)
	{
		test1();
		test2();
	}
	else
	{
		test2();
		test1();
	}

	if (t1.Count() == t2.Count() && t1.rootNode && t2.rootNode)
	{
		size_t count = t1.Count();
		printf("Tree size: %lu\n", count);
		std::vector<Tree1::Node*> t1nodes;
		std::vector<Tree2::Node*> t2nodes;
		t1nodes.reserve(count);
		t2nodes.reserve(count);
		EnumerateTreeNode(t1.rootNode, t1nodes);
		EnumerateTreeNode(t2.rootNode, t2nodes);

		Tree1::Node** t1n = &t1nodes.at(0);
		Tree2::Node** t2n = &t2nodes.at(0);

		size_t i = 0;
		while ( i < count )
		{
			if (t1n[i]->value == t2n[i]->value &&
				t1n[i]->leftHeight == t2n[i]->leftHeight &&
				t1n[i]->rightHeight == t2n[i]->rightHeight)
			{
				++i;
			}
			else
			{
				break;
			}
		}
		if (i == count)
		{
			printf("--- Tree is same! ---\n");
		}
		else
		{
			printf("ERROR Tree is different!!! (i:%lu)\n", i);
		}
	}
	else
	{
		printf("ERROR Tree is different!!! (t1.count:%lu, t2.count:%lu)\n", t1.Count(), t2.Count());
	}



    return 0;
}
