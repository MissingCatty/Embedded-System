#ifndef SOLUTION_H
#define SOLUTION_H

#include<vector>
using namespace std;

#define TEST 2-6

class Solution {
public:
    int search(vector<int>& nums, int target);	// 2-2
	int removeElement(vector<int>& nums, int val); // 2-3
	vector<int> sortedSquares(vector<int>& nums); // 2-4
	int minSubArrayLen(int target, vector<int>& nums); // 2-5
	vector<vector<int>> generateMatrix(int n); // 2-6
};

#endif
