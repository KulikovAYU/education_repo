#include "..\includes\Allocator.hpp"
#include <vector>


int main()
{

	/*MemoryArea area;
	void *pFoo = area.allocate(100);*/


	std::vector<int, CAllocator<int>> v;
	v.reserve(100);
}