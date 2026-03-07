#include <iostream>
#include "solution.h"
#include "utils.h"
using namespace std;

int main(void)
{
	Solution sol;
	Utils utils;
#if TEST == 2-2
	vector<int> nums = {-1,0,3,5,9,12};
	cout << sol.search(nums, 9) << endl;
#elif TEST == 2-3
	vector<int> nums = {0,1,2,2,3,0,4,2};
	cout << sol.removeElement(nums, 2) << endl;
#elif TEST == 2-4
	vector<int> nums1 = {-4, -1, 0, 3, 10};
    auto print = [](vector<int> v) {
		for (int x : v) cout << x << " ";
		cout << endl;
	};
	print(sol.sortedSquares(nums1));
#elif TEST == 2-5
	vector<int> nums1 = {2,3,1,2,4,3};
	cout << sol.minSubArrayLen(-1, nums1) << endl;
#elif TEST == 2-6
	vector<vector<int>> matrix = sol.generateMatrix(5);
	utils.printMatrix(matrix);
#endif
}
