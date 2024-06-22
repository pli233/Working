import javax.swing.plaf.synth.SynthUI;

public class Leetcode10_regExpression {
    public boolean isMatch(String s, String p) {
        int s_len = s.length();
        int p_len = p.length();
        if(!p.contains("*") && !p.contains(".")){
            return s.equals(p);
        }
        //We build a 2D table, in which dp[i][j] tells us if s[0-i] match p[0-j]
        //Solution at dp[s_len][p_len]
        boolean[][] dp = new boolean[s_len][p_len];
        for(int i=0; i<s_len;i++){
            dp[i][0] = (s.charAt(i) == p.charAt(0) || p.charAt(0) =='.');
        }
        for(int i=0; i<s_len;i++){
            char next_p = p.charAt(1);
            if(next_p =='*' || next_p == '.'){
                dp[i][1] = dp[i][0];
            }
            else{
                dp[i][1] = (next_p == s.charAt(i));
            }
           
        }




        
        for(int i =0; i<s_len;i++){
            for(int j=0; j<p_len;j++){
                System.out.print(dp[i][j]+" ");
            }
            System.out.println();
        }
        return true;
    }
    public static void main(String args[]){
        Leetcode10_regExpression solution = new Leetcode10_regExpression();
        System.out.println(solution.isMatch("aaa", "a*"));
    }
}
