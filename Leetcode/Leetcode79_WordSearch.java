

public class Leetcode79_WordSearch {
    public static void main(String[] args){
        char[][] board = {
            {'A', 'B', 'C', 'E'},
            {'S', 'F', 'C', 'S'},
            {'A', 'D', 'E', 'E'}
        };
        String word = "ABCCED";
        Solution solution = new Solution();
        System.out.println(solution.exist(board, word));
    }    
}

class Solution {

    public boolean exist(char[][] board, String word) {
        int num_rows = board.length;
        int num_cols = board[0].length;

        for(int i = 0;i<num_rows; i++){
            for(int j = 0; j<num_cols; j++){
                if(board[i][j] == word.charAt(0)){
                    if(dfs(board,i,j,num_rows,num_cols,0,word)){
                        return true;
                    }
                }
            }
        }
        return false;
    }

    public boolean dfs(char[][] board, int i, int j, int num_rows, int num_cols, int target, String word){
        
        //Base case: Return false if our dfs search is out of range
        if(i<0 || j<0 || i>=num_rows || j>=num_cols){
            return false;
        }
        //Recursive Call: when current element match target, check the next element
        if(board[i][j] == word.charAt(target)){
            if(target==word.length()-1){
                return true;
            }
            char temp = board[i][j];
            board[i][j]='#';
            boolean right = dfs(board,i+1,j,num_rows,num_cols,target+1,word);
            boolean left = dfs(board,i-1,j,num_rows,num_cols,target+1,word);
            boolean up = dfs(board,i,j+1,num_rows,num_cols,target+1,word);
            boolean down = dfs(board,i,j-1,num_rows,num_cols,target+1,word);
            if(right || left || up || down){
                return true;
            }
            board[i][j] = temp;
        } 
        return false;
    }
}






