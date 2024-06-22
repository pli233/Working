class Leetcode5_LongestPalindrome{

    public String longestPalindrome(String s) {
    
    // Check Nullity of the String
    if (s == null || s.length() == 0) {
        return "";
    }
    int n = s.length();
    int[][] dp = new int[n][n];

    int max = 0; // Initialize max with the smallest possible integer value
    int maxI = -1; // Initialize the row index of the maximum element
    int maxJ = -1; // Initialize the column index of the maximum element

    // Single character palindromes
    for (int i = 0; i < n; i++) {
        dp[i][i] = 1;
        max = 1;
        maxI = i;
        maxJ = i;
    }

    // Two character palindromes
    for (int i = 0; i < n - 1; i++) {
        if (s.charAt(i) == s.charAt(i + 1)) {
            dp[i][i + 1] = 2;
            max = 2;
            maxI = i;
            maxJ = i + 1;
        } else {
            dp[i][i + 1] = 0;
        }
    }

    // Palindromes with length >= 3
    for (int i = n - 2; i >= 0; i--) {
        for (int j = i + 2; j < n; j++) {
            int previous = dp[i + 1][j - 1];
            if (s.charAt(i) == s.charAt(j) && previous != 0) {
                int current_length = previous + 2;
                dp[i][j] = current_length;
                if (current_length > max) {
                    max = current_length;
                    maxI = i;
                    maxJ = j;
                }
            } else {
                dp[i][j] = 0;
            }
        }
    }

    // Extract the longest palindrome substring
    if (max != 0) {
        return s.substring(maxI, maxJ + 1);
    }
    return "";
}





    /**
     * @return return 0 when the String is not a palindrome
     * @return return the length of the String otherwise
     */
    public int isPalindrome(String s, int left, int right){
        //Condition1: odd length, like aba, 2-0=2
        if((right-left)%2==0){
            //Initialize pointer to head and midpoint, 2 3 4, give 2 4 return 3
            int midpoint = (right-left)/2+left;
            for(int i=0; left+i<midpoint;i++){
                if(s.charAt(left+i)!=s.charAt(right-i)){
                    return 0;
                }
            }
            return right-left+1;
        }
        //Condition2: even length, like ab, 1-0=1
        else{
            //Initialize pointer to head and midpoint, 2 3 4 5 6 7, return 4
            int midpoint = (right-left)/2+left;
            for(int i=0; left+i<=midpoint;i++){
                if(s.charAt(left+i)!=s.charAt(right-i)){
                    return 0;
                }
            }
            return right-left+1;
        }
    }

    public static void main(String[] args) {
        String s = "aaaaa";
        Leetcode5_LongestPalindrome lp = new Leetcode5_LongestPalindrome();
        String result = lp.longestPalindrome(s);
        System.out.println(result);
    }
}