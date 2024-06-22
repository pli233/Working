


public class Leetcode6_zigzagConversion {

    /**
     * 
     * @param s this is the input String that we are about to handle
     * @param numRows this is the number of rows that our zigzag is about to have 
     * @return a string that can describe our zigzag pattern line by line
     */
    public String convert(String s, int numRows) {
        if(numRows ==1){
            return s;
        }
        int assign_line = 0;
        int n = s.length();
        char [][] map = new char[numRows][n];
        boolean forward = true;
        for(int i=0; i<s.length(); i++){
            if(assign_line == numRows-1){
                forward = false;
            }
            else if(assign_line == 0){
                forward = true;
            }
            map[assign_line][i] = s.charAt(i);
            
            if(forward){
                assign_line++;
            }else{
                assign_line--;
            }
        }
        String result = "";


        for (int i=0; i< numRows; i++){
            for (int j = 0; j<n; j++){
                result = result+map[i][j];
            }
        }
        return result;
    }
    public static void main(String args[]){
        String test = "PAYPALISHIRING";
        Leetcode6_zigzagConversion solution = new Leetcode6_zigzagConversion();
        String result = solution.convert(test, 3);
        System.out.println(result);
    }
}
