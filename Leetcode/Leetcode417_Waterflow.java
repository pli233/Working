import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Leetcode417_Waterflow {
    public static void main(String[] args) {
        int[][] heights = {
                { 1, 2 },
                { 4, 3 }
        };
        Solution2 solution = new Solution2();
        System.out.println(solution.pacificAtlantic(heights));
    }
}

class Solution {
    public List<List<Integer>> pacificAtlantic(int[][] heights) {
        int num_rows = heights.length;
        int num_cols = heights[0].length;
        // Retrive dp table represent cells can run to pacific ocean
        boolean[][] pacific = pacific(heights, num_rows, num_cols);
        // Retrive dp table represent cells can run to atlantic ocean
        boolean[][] atlantic = atlantic(heights, num_rows, num_cols);
        // Cross Valid the two dp table
        List<List<Integer>> result = new ArrayList<>();
        for (int i = 0; i < num_rows; i++) {
            for (int j = 0; j < num_cols; j++) {
                if (pacific[i][j] && atlantic[i][j]) {
                    List<Integer> temp = new ArrayList<>();
                    temp.add(i);
                    temp.add(j);
                    result.add(temp);
                }
            }
        }
        return result;
    }

    public boolean[][] pacific(int[][] heights, Integer num_rows, Integer num_cols) {
        boolean[][] dp = new boolean[num_rows][num_cols];
        // Initialize the left side to be able to run into pacific ocean
        for (int i = 0; i < num_rows; i++) {
            dp[i][0] = true;
        }
        // Initialize the top side to be able to run into pacific ocean
        for (int j = 0; j < num_cols; j++) {
            dp[0][j] = true;
        }
        // Pop the dp table
        for (int i = 1; i < num_rows; i++) {
            for (int j = 1; j < num_cols; j++) {
                if (dp[i][j] == false) {
                    recur_fill(heights, i, j, dp, num_rows, num_cols);
                }
            }
        }
        return dp;
    }

    public boolean[][] atlantic(int[][] heights, Integer num_rows, Integer num_cols) {
        boolean[][] dp = new boolean[num_rows][num_cols];
        // Initialize the right side to be able to run into pacific ocean
        for (int i = 0; i < num_rows; i++) {
            dp[i][num_cols - 1] = true;
        }
        // Initialize the bottom side to be able to run into pacific ocean
        for (int j = 0; j < num_cols; j++) {
            dp[num_rows - 1][j] = true;
        }
        // Pop the dp table
        for (int i = num_rows - 2; i >= 0; i--) {
            for (int j = num_cols - 2; j >= 0; j--) {
                if (dp[i][j] == false) {
                    recur_fill(heights, i, j, dp, num_rows, num_cols);
                }
            }
        }
        // for(int i = 0; i< num_rows; i++){
        // for(int j = 0; j<num_cols; j++){
        // System.out.print(dp[i][j]+" ");
        // }
        // System.out.println();
        // }
        return dp;
    }

    public boolean recur_fill(int[][] heights, Integer i, Integer j, boolean dp[][], Integer num_rows,
            Integer num_cols) {
        // Base case1, check the range
        if (i < 0 || j < 0 || i >= num_rows || j >= num_cols) {
            return false;
        }
        // Base case2: if the dp said yes, than yes
        else if (dp[i][j] == true) {
            // System.out.println("i:"+i+" j:"+j+" value:"+heights[i][j]);
            return true;
        } else {
            // for i, index range should be between 0-heigts.len-1
            // check the top neighbor
            int temp = heights[i][j];
            heights[i][j] = -1;
            if (i > 0 && heights[i - 1][j] != -1 && temp >= heights[i - 1][j]
                    && recur_fill(heights, i - 1, j, dp, num_rows, num_cols)) {
                dp[i][j] = true;
                heights[i][j] = temp;
                return true;
            }
            // check the down neighbor
            else if (i < num_rows - 1 && heights[i + 1][j] != -1 && temp >= heights[i + 1][j]
                    && recur_fill(heights, i + 1, j, dp, num_rows, num_cols)) {
                dp[i][j] = true;
                heights[i][j] = temp;
                return true;
            }
            // check the left neighbor
            else if (j > 0 && heights[i][j - 1] != -1 && temp >= heights[i][j - 1]
                    && recur_fill(heights, i, j - 1, dp, num_rows, num_cols)) {
                dp[i][j] = true;
                heights[i][j] = temp;
                return true;
            }
            // check the right neighbor
            else if (j < num_cols - 1 && heights[i][j + 1] != -1 && temp >= heights[i][j + 1]
                    && recur_fill(heights, i, j + 1, dp, num_rows, num_cols)) {
                dp[i][j] = true;
                heights[i][j] = temp;
                return true;
            } else {
                heights[i][j] = temp;
                return false;
            }
        }
    }
}

class Solution2{
    private static final int[][] DIRECTIONS = new int[][] {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    public List<List<Integer>> pacificAtlantic(int[][] heights) {
        
        int num_rows = heights.length;
        int num_cols = heights[0].length;
        // Retrive dp table represent cells can run to pacific ocean
        boolean[][] pacific = new boolean[num_rows][num_cols];
        // Retrive dp table represent cells can run to atlantic ocean
        boolean[][] atlantic = new boolean[num_rows][num_cols];
        // Initialize the left side to be able to run into pacific ocean
        for (int i = 0; i < num_rows; i++) {
            dfs(heights, i, 0, num_rows, num_cols, pacific);
            dfs(heights, i, num_cols-1, num_rows, num_cols, atlantic);
        }
        // Initialize the top side to be able to run into pacific ocean
        for (int j = 0; j < num_cols; j++) {
            dfs(heights, 0, j, num_rows, num_cols, pacific);
            dfs(heights, num_rows-1, j, num_rows, num_cols, atlantic);
        }
        // Collect the cells that are reachable by both oceans
        List<List<Integer>> result = new ArrayList<>();
        for (int i = 0; i < num_rows; i++) {
            for (int j = 0; j < num_cols; j++) {
                if (pacific[i][j] && atlantic[i][j]) {
                    result.add(Arrays.asList(i, j));
                }
            }
        }
        return result;
    }

    public void dfs(int[][] heights, int i, int j, int num_rows, int num_cols, boolean[][] dp) {
        //Mark the coordinate as visited
        dp[i][j] = true;
        for (int[] dir : DIRECTIONS) {
            int newRow = i + dir[0];
            int newCol = j + dir[1];
            boolean condition = newRow < 0 || newRow >= num_rows || newCol < 0 || newCol >= num_cols
                || dp[newRow][newCol] || heights[newRow][newCol] < heights[i][j];
            // Base case: Do nothing if our dfs search meet any condition above
            if (condition) {
                continue;
            }
            //recursive call
            dfs(heights, newRow, newCol, num_rows, num_cols, dp);
        }
    }
}