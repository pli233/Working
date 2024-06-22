#  Longest Substring Without Repeating Characters
class Solution(object):

    def lengthOfLongestSubstring(self,s):
        return len(self.longestSubstring(s))

    def longestSubstring(self,s):
        # Base Case: when the length of the String is 1, return itself
        if len(s)<=1:
            return s
        
        # Recursive Case: find out the longestSubstring in each half
        else:
            midpoint = len(s)//2
            upper_half = s[0:midpoint]
            lower_half = s[midpoint:]
            upper_half_longest = self.longestSubstring(upper_half)
            lower_half_longest = self.longestSubstring(lower_half)
            cross_longest = self.crossLongest(upper_half, lower_half)
            longest = max(upper_half_longest, lower_half_longest, cross_longest, key=len)
            return longest
            
    def crossLongest(self,upper_half, lower_half):
        result = ""
        upper_len = len(upper_half)
        lower_len = len(lower_half)
        upper_index = upper_len - 1
        lower_index = 0

        while upper_index >= 0 or lower_index < lower_len:
            if upper_index >= 0:
                next_upper = upper_half[upper_index]
                if next_upper not in result:
                    result = next_upper+result
                    upper_index -= 1
                else:
                    upper_index = -1

            if lower_index < lower_len:
                next_lower = lower_half[lower_index]
                if next_lower not in result:
                    result += next_lower
                    lower_index += 1
                else:
                    lower_index = lower_len
        return result

        
    
            
def main():
    s = "bpfbhmipx"
    solution = Solution()
    print(solution.longestSubstring(s))
    
if __name__ == "__main__":
    main()