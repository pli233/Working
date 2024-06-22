public class Leetcode26_UniqueList {
    public static void main(String[] args){
        int[] nums = {0,0};
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
        if(nums.length ==1){
            return 1;
        }

        int unique_count = 1;          // count number of unique element
        int current_unique = nums[0];  //set cursor to first element

        for(int i = 1; i<nums.length; i++){
            //Pass same element, get next unique element
            if(current_unique<nums[i]){
                current_unique = nums[i];
                nums[unique_count] = current_unique;
                unique_count ++;
            }
        }
        return unique_count;
    }
}