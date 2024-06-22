public class Leetcode88_NonDecreaseMerge {
    public static void main(String[] args) {
        int[] nums1 = { 1, 3, 4, 0, 0, 0 };
        int m = 3;
        int[] nums2 = { 2, 5, 6 };
        int n = 3;
        Solution solution = new Solution();
        solution.merge(nums1, m, nums2, n);
        for (int i = 0; i < nums1.length; i++) {
            System.out.print(nums1[i] + " ");
        }
        System.out.println();
    }

}

class Solution {
    public void merge(int[] nums1, int m, int[] nums2, int n) {
        // Pointer for array1
        int i = m - 1;
        // Pointer for array2
        int j = n - 1;
        for (int k = m + n - 1; k >= 0; k--) {

            if (i < 0) {
                nums1[k] = nums2[j];
                j--;
                continue;
            } else if (j < 0) {
                nums1[k] = nums1[i];
                i--;
                continue;
            } else {
                int next_1 = nums1[i];
                int next_2 = nums2[j];
                if (next_2 >= next_1) {
                    nums1[k] = next_2;
                    j--;
                } else {
                    nums1[k] = next_1;
                    i--;
                }
            }
        }
    }
}