public class Leetcode121_BuySellStock {
    public static void main(String[] args){
        int[] prices = {7,1,5,3,6,4};
        Solution solution = new Solution();
        int k = solution.maxProfit(prices);
        System.out.println(k);
    }   
}

class Solution {
    //简单DP
    public int maxProfit(int[] prices) {
        int min = prices[0];
        int max = 0;

        for(int i = 0; i < prices.length; i++){
            max = Math.max(max, prices[i] - min);
            min = Math.min(min, prices[i]);
        }
        return max;
    }
}
