public class Leetcode159_MajorityElement {
    public static void main(String[] args){
        int[] nums = {2,2,1,1,1,2,2};
        Solution solution = new Solution();
        for (int i = 0; i < nums.length; i++) {
            System.out.print(nums[i] + " ");
        }
        System.out.println();
        int k = solution.majorityElement(nums);
        System.out.println(k);
    }
}

class Solution {
    public int majorityElement(int[] nums) {
        if(nums.length == 1){
            return nums[0];
        }

        //boyer-moore algorithm
        int candidate = nums[0];  //current candidate
        int count = 0;      //count of candidate
        for(int i = 0; i<nums.length; i++){
            
            if(candidate == nums[i]){
                count += 1;
            }
            else{
                if(count == 0){
                    candidate = nums[i];
                    count = 1;
                }
                else{
                    count -= 1;
                }
            }
        }
        return candidate;
    }
}