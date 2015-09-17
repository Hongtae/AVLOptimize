#pragma once

#include <new>

namespace DKFoundation2
{
	template <typename VALUE, typename KEY> struct DKTreeComparison
	{
		int operator () (const VALUE& lhs, const KEY& rhs) const
		{
			if (lhs > rhs)				return 1;
			else if (lhs < rhs)			return -1;
			return 0;
		}
	};
	template <typename VALUE> struct DKTreeCopyValue
	{
		void operator () (VALUE& dst, const VALUE& src) const
		{
			dst = src;
		}
	};

	template <
		typename Value,										// value-type
		typename Key,										// key-type (Lookup key)
		typename ValueComparator = DKTreeComparison<Value, Value>,	// value comparison
		typename KeyComparator = DKTreeComparison<Value, Key>,	// value, key comparison (lookup only)
		typename CopyValue = DKTreeCopyValue<Value>,		// value copy
		typename Allocator = DKMemoryDefaultAllocator		// memory allocator
	>
	class DKAVLTree
	{
		class Node
		{
		public:
			Node(const Value& v)
			: value(v), left(NULL), right(NULL), leftHeight(0), rightHeight(0)
			{
			}
			Value		value;
			Node*		left;
			Node*		right;
			int			leftHeight;		// left-tree weights
			int			rightHeight;	// right-tree weights

			FORCEINLINE int Height(void) const
			{
				return leftHeight > rightHeight ? (leftHeight + 1) : (rightHeight + 1);
			}
			Node* Duplicate(void) const
			{
				Node* node = new(Allocator::Alloc(sizeof(Node))) Node(value);
				if (left)
				{
					node->left = left->Duplicate();
				}
				if (right)
				{
					node->right = right->Duplicate();
				}
				node->leftHeight = leftHeight;
				node->rightHeight = rightHeight;
				return node;
			}
			template <typename R> bool EnumerateForward(R&& enumerator)
			{
				if (left && left->EnumerateForward(std::forward<R>(enumerator)))	return true;
				if (enumerator(value))												return true;
				if (right && right->EnumerateForward(std::forward<R>(enumerator)))	return true;
				return false;
			}
			template <typename R> bool EnumerateBackward(R&& enumerator)
			{
				if (right && right->EnumerateBackward(std::forward<R>(enumerator)))	return true;
				if (enumerator(value))												return true;
				if (left && left->EnumerateBackward(std::forward<R>(enumerator)))	return true;
				return false;
			}
			template <typename R> bool EnumerateForward(R&& enumerator) const
			{
				if (left && left->EnumerateForward(std::forward<R>(enumerator)))	return true;
				if (enumerator(value))												return true;
				if (right && right->EnumerateForward(std::forward<R>(enumerator)))	return true;
				return false;
			}
			template <typename R> bool EnumerateBackward(R&& enumerator) const
			{
				if (right && right->EnumerateBackward(std::forward<R>(enumerator)))	return true;
				if (enumerator(value))												return true;
				if (left && left->EnumerateBackward(std::forward<R>(enumerator)))	return true;
				return false;
			}
		};

	public:
		using ValueTraits = DKTypeTraits<Value>;

		constexpr static size_t NodeSize(void)	{ return sizeof(Node); }

		DKAVLTree(void)
		: rootNode(NULL), count(0)
		{
		}
		DKAVLTree(DKAVLTree&& tree)
		: rootNode(NULL), count(0)
		{
			rootNode = tree.rootNode;
			count = tree.count;
			tree.rootNode = NULL;
			tree.count = 0;
		}
		// Copy constructor. accepts same type of class.
		// templates not works on MSVC (bug?)
		DKAVLTree(const DKAVLTree& s)
		: rootNode(NULL), count(0)
		{
			if (s.rootNode)
				rootNode = s.rootNode->Duplicate();
			count = s.count;
		}
		~DKAVLTree(void)
		{
			Clear();
		}
		// Update: insertion if not exist or overwrite if exists.
		const Value* Update(const Value& v)
		{
			if (rootNode)
			{
				LocationContext ctxt = {v};
				LocateNodeForValue(rootNode, &ctxt);
				if (ctxt.balancedNode)
					rootNode = ctxt.balancedNode;
				else
					copyValue(ctxt.locatedNode, v);
				return &(ctxt.locatedNode->value);
			}
			count = 1;
			rootNode = new(Allocator::Alloc(sizeof(Node))) Node(v);
			return &(rootNode->value);
		}
		// Insert: insert if not exist or fail if exists.
		//  returns NULL if function failed. (already exists)
		const Value* Insert(const Value& v)
		{
			if (rootNode)
			{
				LocationContext ctxt = {v};
				LocateNodeForValue(rootNode, &ctxt);
				if (ctxt.balancedNode)
				{
					rootNode = ctxt.balancedNode;
					return &(ctxt.locatedNode->value);
				}
				return NULL;
			}
			count = 1;
			rootNode = new(Allocator::Alloc(sizeof(Node))) Node(v);
			return &(rootNode->value);
		}
		void Remove(const Key& k)
		{
			Node* parent = NULL;
			Node* node = LookupNodeForKey(k, &parent);
			if (node == NULL)
				return;

			Node* retrace = NULL;	// entry node to begin rotation.

			if (node->left && node->right)
			{
				Node* replace = NULL;

				// find biggest from left, smallest from right, and swap, remove node.
				// after remove, rotate again

				if (node->leftHeight > node->rightHeight)
				{
					// finding biggest from left and swap.
					replace = node->left;
					retrace = replace;

					while (replace->right)
					{
						retrace = replace;
						replace = replace->right;
					}
					if (replace != retrace)
					{
						retrace->right = replace->left;
						replace->left = node->left;
					}
					replace->right = node->right;
				}
				else
				{
					// finding smallest from right and swap.
					replace = node->right;
					retrace = replace;

					while (replace->left)
					{
						retrace = replace;
						replace = replace->left;
					}
					if (replace != retrace)
					{
						retrace->left = replace->right;
						replace->right = node->right;
					}
					replace->left = node->left;
				}
				// set 'node's parent with 'replace' as child-node
				if (parent)
				{
					if (parent->left == node)
						parent->left = replace;
					else
						parent->right = replace;
				}
			}
			else
			{
				retrace = parent;
				if (retrace)
				{
					if (retrace->left == node)
					{
						if (node->left)
							retrace->left = node->left;
						else if (node->right)
							retrace->left = node->right;
						else
							retrace->left = NULL;
					}
					else
					{
						if (node->left)
							retrace->right = node->left;
						else if (node->right)
							retrace->right = node->right;
						else
							retrace->right = NULL;
					}
				}
				else
				{
					if (node->left)
						rootNode = node->left;
					else
						rootNode = node->right;
				}
			}

			node->left = NULL;
			node->right = NULL;
			DeleteNode(node);
			if (retrace)
				Balancing(retrace);
		}
		void Clear(void)
		{
			if (rootNode)
				DeleteNode(rootNode);
			rootNode = NULL;
			count = 0;
		}
		const Value* Find(const Key& k) const
		{
			const Node* node = LookupNodeForKey(k);
			if (node)
				return &node->value;
			return NULL;
		}
		size_t Count(void) const
		{
			return count;
		}
		DKAVLTree& operator = (DKAVLTree&& tree)
		{
			if (this != &tree)
			{
				Clear();

				rootNode = tree.rootNode;
				count = tree.count;
				tree.rootNode = NULL;
				tree.count = 0;
			}
			return *this;
		}
		DKAVLTree& operator = (const DKAVLTree& s)
		{
			if (this == &s)	return *this;

			Clear();

			if (s.rootNode)
				rootNode = s.rootNode->Duplicate();
			count = s.count;
			return *this;
		}
		// lambda enumerator (VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator)
		{
			static_assert(DKFunctionType<T&&>::Signature::template CanInvokeWithParameterTypes<Value&, bool*>(),
						  "enumerator's parameter is not compatible with (VALUE&, bool*)");

			if (count > 0)
			{
				bool stop = false;
				auto func = [=, &enumerator](Value& v) mutable -> bool {enumerator(v, &stop); return stop;};
				rootNode->EnumerateForward(func);
			}
		}
		template <typename T> void EnumerateBackward(T&& enumerator)
		{
			static_assert(DKFunctionType<T&&>::Signature::template CanInvokeWithParameterTypes<Value&, bool*>(),
						  "enumerator's parameter is not compatible with (VALUE&, bool*)");

			if (count > 0)
			{
				bool stop = false;
				auto func = [=, &enumerator](Value& v) mutable -> bool {enumerator(v, &stop); return stop;};
				rootNode->EnumerateBackward(func);
			}
		}
		// lambda enumerator bool (const VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator) const
		{
			static_assert(DKFunctionType<T&&>::Signature::template CanInvokeWithParameterTypes<const Value&, bool*>(),
						  "enumerator's parameter is not compatible with (const VALUE&, bool*)");

			if (count > 0)
			{
				bool stop = false;
				auto func = [=, &enumerator](const Value& v) mutable -> bool {enumerator(v, &stop); return stop;};
				rootNode->EnumerateForward(func);
			}
		}
		template <typename T> void EnumerateBackward(T&& enumerator) const
		{
			static_assert(DKFunctionType<T&&>::Signature::template CanInvokeWithParameterTypes<const Value&, bool*>(),
						  "enumerator's parameter is not compatible with (const VALUE&, bool*)");

			if (count > 0)
			{
				bool stop = false;
				auto func = [=, &enumerator](const Value& v) mutable -> bool {enumerator(v, &stop); return stop;};
				rootNode->EnumerateBackward(func);
			}
		}
	private:
		void DeleteNode(Node* node)
		{
			if (node->right)
				DeleteNode(node->right);
			if (node->left)
				DeleteNode(node->left);

			count--;

			(*node).~Node();
			Allocator::Free(node);
		}
		FORCEINLINE Node* LeftRotate(Node* node)
		{
			Node* right = node->right;
			node->right = right->left;
			right->left = node;
			return right;
		}
		FORCEINLINE Node* RightRotate(Node* node)
		{
			Node* left = node->left;
			node->left = left->right;
			left->right = node;
			return left;
		}
		FORCEINLINE void UpdateHeight(Node* node)
		{
			node->leftHeight = node->left ? node->left->Height() : 0;
			node->rightHeight = node->right ? node->right->Height() : 0;
		}
		// balance tree weights.
		Node* Balance(Node* node)
		{
			Node* node2 = node;
			int left = node->left ? node->left->Height() : 0;
			int right = node->right ? node->right->Height() : 0;

			int d = left - right;
			if (d > 1)
			{
				if (node->left->rightHeight > 0 && node->left->rightHeight > node->left->leftHeight)
				{
					// do left-rotate with 'node->left' and right-rotate recursively.
					node->left = LeftRotate(node->left);
					UpdateHeight(node->left->left);
				}
				// right-rotate with 'node' and 'node->left'
				node2 = RightRotate(node);
			}
			else if (d < -1)
			{
				if (node->right->leftHeight > 0 && node->right->leftHeight > node->right->rightHeight)
				{
					// right-rotate with 'node->right' and left-rotate recursively.
					node->right = RightRotate(node->right);
					UpdateHeight(node->right->right);
				}
				// left-rotate with 'node' and 'node->right'
				node2 = LeftRotate(node);
			}
			UpdateHeight(node);
			if (node != node2)
				UpdateHeight(node2);
			return node2;
		}
		struct LocationContext
		{
			const Value& value;
			Node* locatedNode;
			Node* balancedNode;	// valid only if tree needs to be balanced.
			int cmp;
		};
		// find item and delete node.
		void DeleteNodeForValue(Node* node, LocationContext* ctxt)
		{
			ctxt->cmp = valueComparator(node->value, ctxt->value);
			if (ctxt->cmp > 0)
			{
				if (node->left)
				{
					DeleteNodeForValue(node->left, ctxt);
					if (ctxt->balancedNode)
					{
						node->left = ctxt->balancedNode;
						ctxt->balancedNode = Balance(node);
					}
				}
			}
			else if (ctxt->cmp < 0)
			{
				if (node->right)
				{
					DeleteNodeForValue(node->right, ctxt);
					if (ctxt->balancedNode)
					{
						node->right = ctxt->balancedNode;
						ctxt->balancedNode = Balance(node);
					}
				}
			}
			else
			{
			}
		}
		// locate node for value. (create if not exists)
		// this function uses 'LocationContext' value instead of
		// stack variables, because of called recursively.
		void LocateNodeForValue(Node* node, LocationContext* ctxt)
		{
			ctxt->cmp = valueComparator(node->value, ctxt->value);
			if (ctxt->cmp > 0)
			{
				if (node->left)
				{
					LocateNodeForValue(node->left, ctxt);
					if (ctxt->balancedNode)
					{
						node->left = ctxt->balancedNode;
						ctxt->balancedNode = Balance(node);
					}
				}
				else
				{
					node->left = new(Allocator::Alloc(sizeof(Node))) Node(ctxt->value);
					node->leftHeight = 1;
					ctxt->locatedNode = node->left;
					ctxt->balancedNode = node->right ? NULL : node;
					count++;
					return;
				}
			}
			else if (ctxt->cmp < 0)
			{
				if (node->right)
				{
					LocateNodeForValue(node->right, ctxt);
					if (ctxt->balancedNode)
					{
						node->right = ctxt->balancedNode;
						ctxt->balancedNode = Balance(node);
					}
				}
				else
				{
					node->right = new(Allocator::Alloc(sizeof(Node))) Node(ctxt->value);
					node->rightHeight = 1;
					ctxt->locatedNode = node->right;
					ctxt->balancedNode = node->left ? NULL : node;
					count++;
					return;
				}
			}
			else
			{
				ctxt->locatedNode = node;
				ctxt->balancedNode = NULL;
			}
		}
		// find node 'k' and return. (return NULL if not exists)
		Node* LookupNodeForKey(const Key& k, Node** parent)
		{
			return const_cast<Node*>(static_cast<const DKAVLTree&>(*this).LookupNodeForKey(k, parent));
		}
		const Node* LookupNodeForKey(const Key& k, const Node** parent)
		{
			Node* pnode = NULL;
			Node* node = rootNode;
			while (node)
			{
				int cmp = keyCompareFunc(node->value, k);
				if (cmp > 0)
				{
					pnode = node;
					node = pnode->left;
				}
				else if (cmp < 0)
				{
					pnode = node;
					node = pnode->right;
				}
				else
				{
					if (parent)
						*parent = pnode;
					return node;
				}
			}
			return NULL;
		}
		Node* LookupNodeForKey(const Key& k)
		{
			return const_cast<Node*>(static_cast<const DKAVLTree&>(*this).LookupNodeForKey(k));
		}
		const Node* LookupNodeForKey(const Key& k) const
		{
			Node* node = rootNode;
			while (node)
			{
				int cmp = keyCompareFunc(node->value, k);
				if (cmp > 0)
					node = node->left;
				else if (cmp < 0)
					node = node->right;
				else
					return node;
			}
			return NULL;
		}

		Node*			rootNode;
		size_t			count;
		ValueComparator		valueComparator;
		KeyComparator		keyComparator;
		CopyValue			copyValue;
	};
}