public class Leetcode80_Remove2Duplicates {
    public static void main(String[] args){
        int[] nums = {0,0,1,1,1,1,2,3,3};
        Solution solution = new Solution();
        for (int i = 0; i < nums.length; i++) {
            System.out.print(nums[i] + " ");
        }
        System.out.println();
        int k = solution.removeDuplicates(nums);
        for (int i = 0; i < nums.length; i++) {
            System.out.print(nums[i] + " ");
        }
        System.out.println();
        System.out.println(k);

    }
}

class Solution {
    public int removeDuplicates(int[] nums) {
        if (nums.length <=2){
            return nums.length;
        }
        int pass = 2;
        int i = 2;

        while (i < nums.length){
            // 0 0 1 1 1 1
            if (nums[pass-2] != nums[i]){
                nums[pass] = nums[i];
                pass++;
            }
            i++;
        }
        return pass;
    }
}