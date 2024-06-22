import java.util.Scanner;
import java.util.Arrays;
import java.util.Random;

public class QuickSort{


    

    /**
     * 
     * @param arr, an array
     * @param start, the start index of an array, 0
     * @param end, the end index of an array, length-1
     * @return return a sorted int array
     *
     */ 

    public static void quick_sort(int[] arr, int start, int end, String method){
        
        if (method.equals("random")){
            quick_sort_random(arr,start,end);
        }
        else{
            quick_sort_classic(arr,start,end);
        }
    }

     /**
     * 
     * @param arr, an array
     * @param start, the start index of an array, 0
     * @param end, the end index of an array, length-1
     * @return return a sorted int array
     */

     public static void quick_sort_random(int[] arr, int start, int end){

        //Base Case1: When the lenght of arr is 1, do nothing
        if (end == start){
            ;
        } 
        // Base Case2: the input arr is sorted, do nothing
        else if(end < start){
            ;
        }
        //Recursive Case: 
        else{
            // Here we use random method to pick pivot 

            // Create an instance of Random class
            Random random = new Random();

            // Set the pivot index 
            int pivot_index = random.nextInt(end - start + 1) + start; 
            // Partition and retrieve the index of pivot after partition
            int pivot_pos = partition(arr, start, end, pivot_index);
            
            //Recursive call left
            quick_sort_random(arr, start, pivot_pos-1);
            quick_sort_random(arr, pivot_pos+1, end);
            //Recursive call right
        }      
    } 







    /**
     * 
     * @param arr, an array
     * @param start, the start index of an array, 0
     * @param end, the end index of an array, length-1
     * @return return a sorted int array
     */

    public static void quick_sort_classic(int[] arr, int start, int end){

        //Base Case1: When the lenght of arr is 1, do nothing
        if (end == start){
            ;
        } 
        // Base Case2: the input arr is sorted, do nothing
        else if(end < start){
            ;
        }
        //Recursive Case: 
        else{
            // set the last number in the array to be pivot, here we use index of it
            int pivot_index = end; 
            int pivot_pos = partition(arr, start, end, pivot_index);
            
            //Recursive call left
            quick_sort_classic(arr, start, pivot_pos-1);
            quick_sort_classic(arr, pivot_pos+1, end);
            //Recursive call right
        }
            
    } 

    /**
     * 
     * @param input, an array
     * @param left, the left index that we are allowed to do operation
     * @param right, the right bound that we are allow to do operation, if the array start at 0, it is the length 
     * @param pivot_index, the index of pivot we select
     * @return the index of pivot after partition, we will make values smaller or equal to it at left, 
     *          values larger than the pivot at right
     */
    public static int partition(int[] input, int left, int right, int pivot_index){
        // Retrieve the value of the pivot from array
        int pivot_val = input[pivot_index];
        
        // Set up a pointer, point to the left of our array
        int p = left;

        // Set up the end cursor of the <= region
        int small_equal_cur = left;

        while(p<=right){
            // Case1: if the element <= pivot value, exchage the value with the last element in the region
            if(input[p]<=pivot_val){
                swap(input, small_equal_cur, p);
                small_equal_cur++;
            }
            // Case2: the element is > pivot value: do nothing, keep while loop
            p++;
        }
        return small_equal_cur-1;

    }


    /**
     * 
     * @param arr, an array
     * @param a, the first index
     * @param b, the second index
     * 
     * Given an array, we will exchange the position in it
     */
    public static void swap(int[] arr, int a, int b){
        int num = arr[a];
        arr[a] = arr[b];
        arr[b] = num;
    }

    public static void print_arr (int[] arr){
        for (int i =0; i< arr.length; i++){
            System.out.print(arr[i]+" ");
        }
        System.out.println();
    }





    public static void main(String[] args){
        int[] test = {10,2,3,7,23,12,2,3,10,34,5};
        quick_sort(test, 0, test.length-1,"random");
        print_arr(test);
    }   
}