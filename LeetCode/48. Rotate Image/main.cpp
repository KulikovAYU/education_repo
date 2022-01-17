#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <deque>
#include <string>
#include <set>

using namespace std;

class Solution {
public:
	void rotate(vector<vector<int>>& matrix) {

		int lenght = matrix.size();

		for (int row = 0 ; row < lenght / 2; ++row)
		{
			for (int col = row; col < lenght - row - 1; ++col)
			{
				
				swap(matrix[row][col], matrix[col][lenght - 1 - row ]);
				swap(matrix[row][col], matrix[lenght - 1 - row][lenght - 1 - col ]);
				swap(matrix[row][col], matrix[lenght - 1 - col][row]);
				
			}
		}

	}
};


int main()
{
	Solution sol;
	vector<vector<int>> matrix = { {1,2,3} ,{4,5,6},{7,8,9} };
	sol.rotate(matrix);

    return 0;
}