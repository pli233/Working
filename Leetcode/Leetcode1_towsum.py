def twoSum(nums, target):
        """
        :type nums: List[int]
        :type target: int
        :rtype: List[int]
        """
        for i in range(len(nums)):
            i_item = nums[i]
            for j in range(i+1,len(nums)):
                if i_item+nums[j] == target:
                    return([i,j]) 
                
def main():
    
    nums = [3,2,4]
    target = 6
    print(twoSum(nums,target))
    
if __name__ == "__main__":
    main()