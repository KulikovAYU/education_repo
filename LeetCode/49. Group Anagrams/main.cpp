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
	vector<vector<string>> groupAnagrams(vector<string>& strs) {

		vector<vector<string>> res;
		unordered_map<string, vector<string>> anagrams; //sorted str and src

		for (const string& str : strs)
		{
			string sorted = str;
			sort(sorted.begin(), sorted.end(), std::less<char>());

			anagrams[sorted].push_back(str);
		}

		for (const auto& it : anagrams)
			res.push_back(it.second);

		return res;
	}
};


int main()
{
	Solution sol;
	vector<string> v = { "eat","tea","tan","ate","nat","bat" };
	sol.groupAnagrams(v);

    return 0;
}