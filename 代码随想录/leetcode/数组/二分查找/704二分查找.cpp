#include "../header.h"

class Solution {
public:
    int search(vector<int>& nums, int target) {
        int l = 0, h = nums.size();
        while (l < h) {
            int mid = (h - l) / 2 + l;
            if (nums[mid] == target) {
                return mid;
            } else if (nums[mid] < target) {
                l = mid + 1;
            } else {
                h = mid;
            }
        }
        return -1;
    }
};