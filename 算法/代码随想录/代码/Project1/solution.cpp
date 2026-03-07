#include "solution.h"
#include <math.h>
using namespace std;

// 2-2
int Solution::search(vector<int>& nums, int target) {
	int len = nums.size();
	int left = 0, right = len - 1, mid;
	while(left <= right) {
		mid = (left + right) / 2;
		if(target == nums[mid]) {
			return mid;
		} else if(target < nums[mid]) {
			right --;
		} else {
			left ++;
		}
	}
	return -1;
}

// 2-3
int Solution::removeElement(vector<int>& nums, int val) {
	int len = nums.size();
	int left = 0, right	= 0;
	while(right < len) {
		if(nums[right] != val) {
			nums[left++] = nums[right];
		}
		right++;
	}
	return left;
}

// 2-4
vector<int> Solution::sortedSquares(vector<int>& nums) {
	int len = nums.size();
	int left = 0, right = len - 1, p = len - 1;
	vector<int> ret(len);
	while(left <= right) {
		if(abs(nums[left]) > abs(nums[right])) {
			ret[p--] = nums[left] * nums[left];
			left++;
		} else {
			ret[p--] = nums[right] * nums[right];
			right--;
		}
	}
	return ret;
}

// 2-5
int Solution::minSubArrayLen(int target, vector<int>& nums) {
	int len = nums.size();
	int left = 0, right = 0;
	int sum = 0;
	int minLen = INT_MAX;
	while(right < len) {
		// 扩展右边
		sum += nums[right];
		right++; // right指向的是当前窗口的后一个
		
		// 缩小左边
		while(sum >= target && left < right) {
			minLen = min(minLen, right - left); // 先更新长度
			sum -= nums[left];
			left++;
		}
	}
	return minLen==INT_MAX ? 0 : minLen;
}

// 2-6
vector<vector<int>> Solution::generateMatrix(int n) {
	int start_row, start_col, width;
	vector<vector<int>> matrix(n, vector<int>(n));
	int num = 1, i = 0; // i是圈数
	while(i < n / 2) {
		// 填充
		width = n - 2 * i;
		
		// 左上
		start_row = i, start_col = i;
		for(int bias_col = 0; bias_col < width - 1; bias_col++) {
			matrix[start_row][start_col + bias_col] = num++;
		}
		// 右上
		start_row = i, start_col = i + width - 1;
		for(int bias_row = 0; bias_row < width - 1; bias_row++) {
			matrix[start_row + bias_row][start_col] = num++;
		}
		// 右下
		start_row = i + width - 1, start_col = i + width - 1;
		for(int bias_col = 0; bias_col < width - 1; bias_col++) {
			matrix[start_row][start_col - bias_col] = num++;
		}
		// 左下
		start_row = i + width - 1, start_col = i;
		for(int bias_row = 0; bias_row < width - 1; bias_row++) {
			matrix[start_row - bias_row][start_col] = num++;
		}		
		
		// 圈数+1
		i++;
	}
	
	// 补充奇数中心
	if(n%2 == 1) {
		matrix[n / 2][n / 2] = num;
	}
	
	return matrix;
}
