#  Longest Substring Without Repeating Characters
class Solution(object):
    def findMedianSortedArrays(self, nums1, nums2):
        """
        :type nums1: List[int]
        :type nums2: List[int]
        :rtype: float
        """
        
        # Base Case:
        # if nums1 is empty, return the median of nums2
        if len(nums1) == 0:
            return self.median(nums2)
        # if nums2 is empty, return the median of nums1
        elif len(nums2) == 0:
            return self.median(nums1)
        #Both two 
        else:
            #If we total have even numbers, use helper1
            if (len(nums1)+len(nums2)) % 2 ==0:
                return self.helper1(nums1,nums2)
            else:
                return self.helper2(nums1,nums2)
    
    #Takes advantage of merge sort
    def helper1(self,nums1,nums2):
        index1 = 0
        index2 = 0
        result = []
        count = 0
        target = (len(nums1)+len(nums2))//2+1
        
        while count<target:
            next_nums1 = float('inf')
            next_nums2 = float('inf')
            #Retrive next number in the two array as long as we do not out of index
            if index1 <len(nums1):
                next_nums1 = nums1[index1]
            if index2 <len(nums2):
                next_nums2 = nums2[index2]
            # Compare the two number, add smaller to list, which can be represent by move right the index by 1
            # the numeber from first array is smaller, increase index  
            if next_nums1 <= next_nums2:
                result.append(nums1[index1])
                index1 += 1
            else:
                result.append(nums2[index2]) 
                index2 += 1
            count+=1
        # print(result)
        return float((result[-1]+result[-2]))/2
            
     #Takes advantage of merge sort
    def helper2(self,nums1,nums2):
        index1 = 0
        index2 = 0
        result = []
        count = 0
        target = (len(nums1)+len(nums2))//2+1

        while count<target:
            next_nums1 = float('inf')
            next_nums2 = float('inf')
            #Retrive next number in the two array as long as we do not out of index
            if index1 <len(nums1):
                next_nums1 = nums1[index1]
            if index2 <len(nums2):
                next_nums2 = nums2[index2]
            # Compare the two number, add smaller to list, which can be represent by move right the index by 1
            # the numeber from first array is smaller, increase index  
            if next_nums1 <= next_nums2:
                result.append(nums1[index1])
                index1 += 1
            else:
                result.append(nums2[index2]) 
                index2 += 1
            count+=1
        return result[-1]
            
    def median(self,nums):
        length = len(nums)
        mid_index = length// 2

        if length % 2 == 0:  # If the length is even
            return float((nums[mid_index - 1] + nums[mid_index])) / 2
        else:  # If the length is odd
            return nums[mid_index]
        
        
        
        
    
            
def main():
    nums1 = [1,2]
    nums2 = [3,4]
    solution = Solution()
    print(solution.findMedianSortedArrays(nums1,nums2))
    
if __name__ == "__main__":
    main()