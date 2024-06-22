public class Leetcode27_RemoveElement{
    public static void main(String[] args){
        int[] nums = {3,2,2,3};
        int val = 3;
        Solution solution = new Solution();
        for (int i = 0; i < nums.length; i++) {
            System.out.print(nums[i] + " ");
        }
        System.out.println();
        int k = solution.removeElement(nums,val);
        for (int i = 0; i < nums.length; i++) {
            System.out.print(nums[i] + " ");
        }
        System.out.println();
        System.out.println(k);

    }
}

class Solution {
    public int removeElement(int[] nums, int val) {
        int pass = 0;
        for (int i = 0; i <nums.length; i++){
            if(nums[i] != val){
                nums[pass] = nums[i];
                pass++;
            }
        }
        return pass;
    }
}