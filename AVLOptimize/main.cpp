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
	DKFoundation::DKTreeCopyValue<u_int32_t>,
	Tree1Allocator>;

using Tree2 = DKFoundation2::DKAVLTree<u_int32_t,
	DKFoundation::DKTreeComparison<u_int32_t, u_int32_t>,
	DKFoundation::DKTreeCopyValue<u_int32_t>,
	Tree2Allocator>;

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

int main(int argc, const char * argv[])
{
	printf("Debug Mode: %d\n", debugMode);

	size_t numSamples = 0xffffff;
	std::vector<u_int32_t> samples;
	samples.reserve(numSamples);
	for (size_t i = 0; i < numSamples; ++i)
	{
		u_int32_t v = arc4random();
		v = v % (numSamples/2);
		samples.push_back(v);
	}

	printf("Reserving memory...\n");
	t1alloc.Reserve(numSamples);
	t2alloc.Reserve(numSamples);
	printf("Done!\n");

	Tree1 t1;
	Tree2 t2;

	const int numLoops = 1;

	auto ir_test1 = [&]()
	{
		Timer timer;
		size_t numInsert = 0;
		size_t numRemove = 0;

		t1.Update(1);	// warm up.
		t1.Clear();

		printf("Testing insert/remove Tree1... (%lu items x %d)\n", samples.size(), numLoops);

		timer.Reset();
		for (int i = 0; i < numLoops; ++i)
		{
			for (u_int32_t v : samples)
			{
				if (t1.Insert(v))
					numInsert++;
				else
				{
					t1.Remove(v);
					numRemove++;
				}
			}
		}
		double d = timer.Elapsed();
		printf("Tree1 insert: %zu / remove: %zu elapsed: %f\n", numInsert, numRemove, d);
	};

	auto ir_test2 = [&]()
	{
		Timer timer;
		auto t2Comp = DKFoundation2::DKTreeComparator<u_int32_t, uint32_t>();
		size_t numInsert = 0;
		size_t numRemove = 0;

		t2.Update(1);	// warm up.
		t2.Clear();

		printf("Testing insert/remove Tree2... (%lu items x %d)\n", samples.size(), numLoops);
		timer.Reset();
		for (int i = 0; i < numLoops; ++i)
		{
			for (u_int32_t v : samples)
			{
				if (t2.Insert(v))
					numInsert++;
				else
				{
					t2.Remove(v, t2Comp);
					numRemove++;
				}
			}
		}
		double d = timer.Elapsed();
		printf("Tree2 insert: %zu / remove: %zu elapsed: %f\n", numInsert, numRemove, d);
	};

	auto sr_test1 = [&]()
	{
		Timer timer;

		printf("Testing Search Tree1(Count: %lu)... (%lu items x %d)\n", t1.Count(), samples.size(), numLoops);

		size_t found = 0;
		size_t missed = 0;

		timer.Reset();
		for (int i = 0; i < numLoops; ++i)
		{
			for (u_int32_t v : samples)
			{
				auto p = t1.Find(v);
				if (p)
					found++;
				else
					missed++;
			}
		}
		double d = timer.Elapsed();
		printf("Tree1 search (found: %zu, missed: %zu) elapsed: %f\n", found, missed, d);
	};

	auto sr_test2 = [&]()
	{
		Timer timer;
		auto t2Comp = DKFoundation2::DKTreeComparator<u_int32_t, uint32_t>();

		printf("Testing Search Tree2(Count: %lu)... (%lu items x %d)\n", t2.Count(), samples.size(), numLoops);

		size_t found = 0;
		size_t missed = 0;

		timer.Reset();
		for (int i = 0; i < numLoops; ++i)
		{
			for (u_int32_t v : samples)
			{
				auto p = t2.Find(v, t2Comp);
				if (p)
					found++;
				else
					missed++;
			}
		}
		double d = timer.Elapsed();
		printf("Tree2 search (found: %zu, missed: %zu) elapsed: %f\n", found, missed, d);
	};

	printf("\nInsert/Remove test...\n");
	if (arc4random() % 2)
	{
		ir_test1();
		ir_test2();
	}
	else
	{
		ir_test2();
		ir_test1();
	}

	printf("\nSearch test...\n");
	if (arc4random() % 2)
	{
		sr_test1();
		sr_test2();
	}
	else
	{
		sr_test2();
		sr_test1();
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
