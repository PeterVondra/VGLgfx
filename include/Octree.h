#pragma once

#include <vector>

namespace Utils
{
	struct Node
	{
		T data;
		Node* children[8];
	};
	template<typename T>class Octree
	{
		public:
			Octree();
			~Octree();

			void destroy();

			// Per partition
			uint64_t maxObjectCount;

			// Per partition
			uint64_t minObjectCount;

		private:

			Node* allocateNode();
			Node* allocateNode8x();

			Node* m_Nodes[8]; // Base nodes
	};
	template<typename T>
	inline Node* Octree<T>::allocateNode(){
		return new Node;
	}
	template<typename T>
	inline Node* Octree<T>::allocateNode8x(){
		return new Node[8];
	}
}