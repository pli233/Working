public class Leetcode189_RotateArray {
    public static void main(String[] args){
        int[] nums = {1,2,3,4,5,6,7};
        Solution solution = new Solution();
        for (int i = 0; i < nums.length; i++) {
            System.out.print(nums[i] + " ");
        }
        System.out.println();
        solution.rotate(nums,3);
        for (int i = 0; i < nums.length; i++) {
            System.out.print(nums[i] + " ");
        }
    }

}

class Solution {
    //双层反转
    public void rotate(int[] nums, int k) {
        k = k% nums.length;
        reverse(nums, 0, nums.length-1);
        reverse(nums, 0, k-1);
        reverse(nums, k, nums.length-1);
    }


    public void reverse(int[] nums, int start, int end) {
        while (start < end) {
            int temp = nums[start];
            nums[start] = nums[end];
            nums[end] = temp;
            start += 1;
            end -= 1;
        }
    }
} 