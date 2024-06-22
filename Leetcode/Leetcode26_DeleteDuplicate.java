public class Leetcode26_DeleteDuplicate {
    public static void main(String[] args) {
        int[] input = { 0, 0, 1, 1, 1, 2, 2, 3, 3, 4 };
        int[] test = {};
        Solution solution = new Solution();
        solution.removeDuplicates(test);
    }
}

class Solution {
    public int removeDuplicates(int[] nums) {
        int n = nums.length;
        if (n == 0) {
            return 0;
        } 
        else {
            int cur = nums[0];
            int count = 1;
            for (int i = 0; i<n;i++) {
                int next = nums[i];
                if(next>cur){
                    nums[count] = next;
                    cur = next;
                    count++;
                }
                // for (int j = 0; j < n; j++) {
                //     System.out.print(nums[j] + " ");
                // }
                // System.out.println("next: "+next+"count: "+count);
            }
            return count;
        }
    }
}