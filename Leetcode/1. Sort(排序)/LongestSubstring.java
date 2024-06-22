
public class LongestSubstring{


    public static String lengthOfLongestSubstring(String s) {
        return LLS_helper(s,0,s.length()-1);
    }

    public static String LLS_helper(String s, int left, int right){
        //Base Case: when the length of String is 1, it self should be the longest Substring, return it 
        if(s.length()==1){
            return s;
        }
        //Recursive Case: when the length of String is not 1, divide it half and half, run function on each half and merge 
        else{
            int mid = left+(right-left)/2;
            String firsthalf = s.substring(0, mid+1);
            String secondhalf = s.substring(mid+1, right+1);
            String left_max = LLS_helper(firsthalf, 0, firsthalf.length()-1);
            String right_max = LLS_helper(secondhalf, 0, secondhalf.length()-1);
            String mid_max = Mid_MaxSubstring(s, mid);
            
        }
    }

    public static String Mid_MaxSubstring(String s, int mid){

        return s;
    }
    public static void main(String[] args){
        String test = "pwwkew";
        System.out.println(lengthOfLongestSubstring(test));
    }   

}