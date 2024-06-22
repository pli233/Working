
import random

def QuickSort(arr, rand = False):
    #Base Case:
    if(len(arr)<=1):
        return arr
    
    #Recursive Call
    else:
        #Here we choose the last number in the array to be pivot
        
        #set the index of pivot
        if(rand == True):
            pivot_index = random.randint(0,len(arr)-1)
        else:
            pivot_index = -1 
        #retrieve the value of pivot
        pivot = arr[pivot_index]
        #delete it from list
        del arr[pivot_index]
        
        left, right = partition(arr, pivot)
        sort_left = QuickSort(left)
        sort_right = QuickSort(right)
        result = []
        for i in sort_left:
            result.append(i)
        result.append(pivot)
        for i in sort_right:
            result.append(i)
        return result
        
        
#put values <= pivot at left, other at right, return new position
def partition(arr,pivot):
    left = []
    right = []
    for i in arr:
        if i<= pivot:
            left.append(i)
        else:
            right.append(i)
    return left,right

def main():

    test1 = [1,23,4,6,5,80,13,9,7,2,3,23,4,3,2,11,1,1,1,1,1,1]
    result = QuickSort(test1,True)
    print(result)
    print(10//3)
    
if __name__ == "__main__":
    main()