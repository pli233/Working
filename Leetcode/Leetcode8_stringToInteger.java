public class Leetcode8_stringToInteger {

    public int myAtoi(String s) {
        int len = s.length();
        if (len == 0) {
            return 0;
        }

        int sign = 1;
        int i = 0;

        // 1. Check leading white space
        // Read in and ignore any leading whitespace.
        for (; i < len; i++) {
            if (s.charAt(i) != ' ') {
                break;
            }
        }

        // Check if the next character (if not already at the end of the string) is '-'
        // or '+'.
        if (i < len) {
            char next = s.charAt(i);
            if (next == '+') {
                sign = 1;
                i++;
            } else if (next == '-') {
                sign = -1;
                i++;
            }
        } else {
            return 0;
        }

        // 2. Check remaining characters
        long result = 0;
        for (; i < len; i++) {
            char next = s.charAt(i);

            // Break the loop if the character is not a digit
            if (next < '0' || next > '9') {
                break;
            }

            // Accumulate the result
            result = result * 10 + (next - '0');

            // Check if the result exceeds the valid range
            if (sign * result <= Integer.MIN_VALUE) {
                return Integer.MIN_VALUE;
            } else if (sign * result >= Integer.MAX_VALUE) {
                return Integer.MAX_VALUE;
            }
        }

        // Apply the sign to the result and cast it to int
        return sign * (int) result;
    }

    public static void main(String args[]) {
        Leetcode8_stringToInteger solution = new Leetcode8_stringToInteger();
        String test = " ";

        System.out.println(solution.myAtoi(test));
    }
}