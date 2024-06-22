import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Queue;
import java.util.Set;

class Employee {
    public int id;
    public int importance;
    public List<Integer> subordinates;

    public Employee(int id, int importance, List<Integer> subordinates) {
        this.id = id;
        this.importance = importance;
        this.subordinates = subordinates;
    }
};

class Solution {
    public int getImportance(List<Employee> employees, int id) {
        int length = employees.size();
        // Base case: if there is no Employee given, return 0
        if (length == 0) {
            return 0;
        }
        Map<Integer, Employee> emlopyeeMap = new HashMap<>();
        // Construct HashMap as getting the employee from id is difficult in a list
        for (Employee e : employees) {
            emlopyeeMap.put(e.id, e);
        }

        // Intialize the sum of Importance to zero
        int totalImportance = 0;

        // Initialize the set to include visited elements
        Set<Integer> visited = new HashSet<>();

        // Initializaed queue
        Queue<Integer> todo = new LinkedList<Integer>();
        todo.offer(id);
        // Modify BFS
        while (!todo.isEmpty()) {
            int curId = todo.poll();
            // If this employee is not visited
            if (!visited.contains(curId)) {
                // Mark the employee are visited
                visited.add(curId);
                //get the employee from hashmap
                Employee curEmployee = emlopyeeMap.get(curId);
                // After the employee is found , process their importance and subordinates
                totalImportance += curEmployee.importance;
                // Add suborinates into the queue
                for (int subordinateId : curEmployee.subordinates) {
                    todo.offer(subordinateId);
                }
            }
        }
        return totalImportance;
    }
}

public class Leetcode690_EmployeeImportance {
    public static void main(String args[]) {
        Employee employee1 = new Employee(1, 5, Arrays.asList(2, 3));
        Employee employee2 = new Employee(2, 3, Arrays.asList());
        Employee employee3 = new Employee(3, 3, Arrays.asList());
        List<Employee> employees = Arrays.asList(employee1, employee2, employee3);
        Solution solution = new Solution();
        System.out.println(solution.getImportance(employees, 1));
    }
}
