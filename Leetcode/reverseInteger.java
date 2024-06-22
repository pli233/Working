public class reverseInteger {
    public int reverse(int x) {
        // if number x is negative, there is going to be a - at the first place 
        if(x==0){
            return x;
        }
        else if(x<0){
            return -(reverseHelper(-x));
        }
        else{
            return reverseHelper(x);
        }
    }
    public int reverseHelper(int x){
        int result = 0;
        while(true){
            int remain = x%10;
            if(remain!=0){
                break;
            }
            x = x/10;
        }
        while(x!=0){
            int remain = x%10;
            x = x/10;
            result = result*10+remain;
        }
        if(result> Integer.MAX_VALUE){
            return 0;
        }
        
    }
    
    public static void main(String args[]){
        int s = 1203;
        reverseInteger solution = new reverseInteger();
        System.out.println(solution.reverse(s));
    }
}
