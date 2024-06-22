def sumOfPower(nums):
        """
        :type nums: List[int]
        :rtype: int
        """
        subsets = permutation(nums)
        print(subsets)
        sum = 0
        for i in subsets:
            temp = power_helper(i)
            sum = sum+temp
        return sum %(10**9+7)

def permutation(nums, subset=[]):
    
    if len(nums) ==1:
        subset.append(nums)
        return subset
    
    else:
        subset.append([nums[0]])
        result = permutation(nums[1:],[])
        for element in result:
            temp = element[:]
            temp.append(nums[0])
            subset.append(temp)
            subset.append(element)
    return subset

def power_helper(nums):
    sorted_nums = sorted(nums)
    return (sorted_nums[-1]**2)*sorted_nums[0]

test = [1,1,1]
print(sumOfPower(test))