public class Leetcode27_Removeelement {
    public static void main(String[] args) {
        Solution solution = new Solution();
        int[] nums = { 0, 1, 2, 2, 3, 0, 4, 2 };
        int val = 2;
        System.out.println(solution.removeElement(nums, val));
    }
}

class Solution {
    public int removeElement(int[] nums, int val) {
        int count = 0;
        int n = nums.length;
        for(int i = 0; i<n-count;){ 
            if(nums[i] == val){
                nums[i] = nums[n-1-count];
                nums[n-1-count] = val;
                count++;
            }
            else{
                i++;
            }
        }
        return n-count;
    }
}