import java.util.HashSet;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;

public class Leetcode226_InvertBinaryTree {
    public static void main(String[] args) {

        TreeNode node1 = new TreeNode(1);
        TreeNode node3 = new TreeNode(3);
        TreeNode node6 = new TreeNode(6);
        TreeNode node9 = new TreeNode(9);

        TreeNode node2 = new TreeNode(2, node1, node3);
        TreeNode node7 = new TreeNode(7, node6, node9);

        TreeNode node4 = new TreeNode(4, node2, node7);

        TreeNode empty = new TreeNode();
        Solution solution = new Solution();

        solution.bfs_print(node4);
        solution.invertTree(node4);
        solution.bfs_print(node4);
    }
}

class TreeNode {
    int val;
    TreeNode left;
    TreeNode right;

    TreeNode() {
    }

    TreeNode(int val) {
        this.val = val;
    }

    TreeNode(int val, TreeNode left, TreeNode right) {
        this.val = val;
        this.left = left;
        this.right = right;
    }
}

class Solution {
    public TreeNode invertTree(TreeNode root) {
        if (root != null) {
            bfs(root);
        }
        return root;
    }

    public void bfs_print(TreeNode root) {

        // Initialize the set to include visited elements
        Set<TreeNode> visited = new HashSet<>();

        // Initializaed queue
        Queue<TreeNode> todo = new LinkedList<TreeNode>();
        todo.offer(root);

        // Modify BFS
        while (!todo.isEmpty()) {
            TreeNode next = todo.poll();
            // If this treenode is not visited
            if (!visited.contains(next)) {
                System.out.print(next.val + " ");
                // Mark the employee are visited
                visited.add(next);
                // get the child of treenode
                TreeNode left = next.left;
                TreeNode right = next.right;
                if (left != null) {
                    todo.offer(left);
                }
                if (right != null) {
                    todo.offer(right);
                }
            }
        }
        System.out.println();
    }

    public void bfs(TreeNode root) {

        // Initialize the set to include visited elements
        Set<TreeNode> visited = new HashSet<>();

        // Initializaed queue
        Queue<TreeNode> todo = new LinkedList<TreeNode>();
        todo.offer(root);

        // Modify BFS
        while (!todo.isEmpty()) {
            TreeNode next = todo.poll();
            // If this treenode is not visited
            if (!visited.contains(next)) {

                // Mark the employee are visited
                visited.add(next);
                // get the child of treenode
                TreeNode left = next.left;
                TreeNode right = next.right;
                next.left = right;
                next.right = left;
                if (left != null) {
                    todo.offer(left);
                }
                if (right != null) {
                    todo.offer(right);
                }
            }
        }
    }
}