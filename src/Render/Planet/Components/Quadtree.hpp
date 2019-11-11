#pragma once

#include <map>
#include <queue>
#include <array>
#include <vector>
#include <memory>
#include <functional>

struct Square
{
	float x, y;
	float size;
};

template <class T>
struct Quadtree
{
	public:
        enum EQuad { NW, NE, SE, SW };
        enum EDir { North, East, South, West };

        Quadtree(EQuad quad, T* parent)
            : Depth(0),
              Bounds { -1.0f, -1.0f, 2.0f },
              Parent(parent),
              Quad(quad)
        {}

		virtual ~Quadtree() {}

		bool IsLeaf() const { return ChildNodes[0] == NULL; }
		void Split();
		void Merge();
		void Update(float dt);
		void ApplyRecursiveFunction(std::function<void(Quadtree<T>*)> f);

		void SetParent(T* parent) { Parent = parent; }
		T* GetParent() const { return Parent; }

		void SetBounds(const Square& bounds) { Bounds = bounds; }
		Square GetBounds() const { return Bounds; }

		int GetDepth() const { return Depth; }
		T* GetChild(int quad) const { return ChildNodes[quad]; }

		T* GetGreaterThanOrEqualNeighbour(int dir) const;
        std::vector<T*> GetSmallerNeighbours(T* neighbour, int dir) const;

		int FaceID, Quad;
        std::vector<T*> RootNeighbourList;

		static std::map<int, std::map<int, int>> FaceCorrection;
		static std::map<int, std::map<int, int>> InternalCorrection;

	protected:
		std::array<T*, 4> ChildNodes;

		int Depth;
		Square Bounds;
		T* Parent;

		virtual void SplitFunction() {}
		virtual void MergeFunction() {}
		virtual void TickFunction(float dt, int child)  {}
		virtual bool DistanceFunction() { return false; }
};

template <class T>
void Quadtree<T>::Split()
{
	if (IsLeaf())
		SplitFunction();
	else
	{
		for (const auto& child : ChildNodes)
			child->Split();
	}
}

template <class T>
void Quadtree<T>::Merge()
{
	if (IsLeaf())
		return;

	if (ChildNodes[0]->IsLeaf())
		MergeFunction();
	else
	{
		for (const auto& child : ChildNodes)
			child->Merge();
	}
}

template <class T>
void Quadtree<T>::Update(float dt)
{
	//if(visible)
	{
		bool divide = DistanceFunction();

		if (!divide)
			Merge();

		if (IsLeaf() && divide)
			Split();
		else if (!IsLeaf())
		{
			for (int i = 0; i < 4; ++i)
				TickFunction(dt, i);
		}
	}
}

template<class T>
void Quadtree<T>::ApplyRecursiveFunction(std::function<void(Quadtree<T>*)> f)
{
	for(int i = 0; i < 4; ++i)
	{
		f(this);

		if(!IsLeaf())
			ChildNodes[i]->ApplyRecursiveFunction(f);
	}
}

template <class T>
T* Quadtree<T>::GetGreaterThanOrEqualNeighbour(int dir) const
{
	int childDirIndexA = dir;
	int childDirIndexB = (dir + 1) % 4;
	int oppChildDirIndexA = (dir + 3) % 4;
	int oppChildDirIndexB = (dir + 2) % 4;

	if (!Parent) return RootNeighbourList[dir];
	if (Parent->GetChild(oppChildDirIndexA) == this) return Parent->GetChild(childDirIndexA);
	if (Parent->GetChild(oppChildDirIndexB) == this) return Parent->GetChild(childDirIndexB);

	auto node = Parent->GetGreaterThanOrEqualNeighbour(dir);

	if (!node || node->IsLeaf())
		return node;

	int correction = FaceCorrection[FaceID][node->FaceID];

	if (Parent->GetChild(childDirIndexA) == this) return node->GetChild((oppChildDirIndexA + correction) % 4);
	if (Parent->GetChild(childDirIndexB) == this) return node->GetChild((oppChildDirIndexB + correction) % 4);

	return nullptr;
}

template <class T>
std::vector<T*> Quadtree<T>::GetSmallerNeighbours(T* neighbour, int dir) const
{
	std::vector<T*> neighbours;
	std::queue<T*> nodes;

	if (neighbour)
		nodes.push(neighbour);

	int correction = InternalCorrection[FaceID][static_cast<Quadtree*>(neighbour)->FaceID];

	while (nodes.size() > 0) {
		if (static_cast<Quadtree*>(nodes.front())->IsLeaf())
			neighbours.push_back(nodes.front());
		else {
			nodes.push(static_cast<Quadtree*>(nodes.front())->GetChild((dir + 2 + correction) % 4));
			nodes.push(static_cast<Quadtree*>(nodes.front())->GetChild((dir + 3 + correction) % 4));
		}

		nodes.pop();
	}

	return neighbours;
}
