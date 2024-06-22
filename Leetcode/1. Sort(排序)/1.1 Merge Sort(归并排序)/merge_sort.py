import sys
import math

def merge_sort(arr,size):
    #TODO: return a sorted arr and num of inversions
    
    #Base case: if there is only one element in the array, return it self,
    # and the number of inversions here is 0
    if size==1:
        return arr, 0

    #seperate the list by index
    mid_point = math.ceil(size/2)
    left = arr[0:mid_point]
    right = arr[mid_point:]
    
    #recursive left half:
    left_sort_arr,left_inver_count = merge_sort(left,len(left))
    #recursive right half
    right_sort_arr,right_inver_count = merge_sort(right,len(right))
    
    #merge two sorted array
    sort_arr, merge_inver_count = merge(left_sort_arr,right_sort_arr)
    
    #calculate total inversions
    total_inver = left_inver_count+right_inver_count+merge_inver_count
    
    return sort_arr, total_inver
  
def merge(left,right):
    #TODO: merge two sorted array, it can be zero
    sorted_arr = []
    inver_count = 0
    
    while (len(left)!=0) or (len(right)!=0):

        #if left is empty, pop right
        if(len(left)==0):
            sorted_arr.append(right.pop(0))
        #if right is empty, pop left
        elif(len(right)==0):
            sorted_arr.append(left.pop(0))
        #both not empty, compare first element
        else:
            if(left[0]<=right[0]):
                sorted_arr.append(left.pop(0))
            else:
                sorted_arr.append(right.pop(0))
                inver_count += len(left)
    return sorted_arr, inver_count



nums_instance = int(sys.stdin.readline())

for i in range(nums_instance):
    
    ##PartA: Initialization
    
    #number of elements we have in this instance
    size_elements = int(sys.stdin.readline())
    
    if size_elements ==0:
        print(str(0))
        continue
   
    #read these elements and store them in an array, as unsort

    str_elements = sys.stdin.readline().split(" ")
    
    unsort_arr = [int(s) for s in str_elements]
    
    sort_arr,inversion_count =merge_sort(unsort_arr, size_elements)
    
    #print("sorted_arr: "+str(sort_arr),end = "")
    #print("  inversion= "+str(inversion_count))
    print(inversion_count)
    
