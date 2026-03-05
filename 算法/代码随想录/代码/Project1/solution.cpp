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
