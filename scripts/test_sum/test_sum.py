import sys
sys.setrecursionlimit(11000)

#the test case (python)
def test_sum(key: int, val: int):
    def sum(n: int):
        if n < 2:
            return n
        
        return n + sum(n - 1)
    
    result: int = sum(val)
    print(str(key) + ": " + str(result))

for i in range(0, 10):
    test_sum(i, i * 1000)
