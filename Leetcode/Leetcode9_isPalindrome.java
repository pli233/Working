public class Leetcode9_isPalindrome {
    /**
     * @return return 0 when the String is not a palindrome
     * @return return the length of the String otherwise
     */
    public boolean Solution(int x) {
        if(x<0){
            return false;
        }
        else{
            int rev = 0;
            int org = x;
            while(org>0){
                rev = rev*10+org%10;
                org = org/10;
            }
            return rev == x;
        }
    }

    public static void main(String args[]) {
        int s = 123321;
        Leetcode9_isPalindrome solution = new Leetcode9_isPalindrome();
        System.out.println(solution.Solution(s));
    }
}
