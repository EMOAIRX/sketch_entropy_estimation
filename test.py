import numpy


def f(x):#x log x
    return x*numpy.log(x)

def entropy(x):# []
    ans = 0
    sum = 0
    for i in x:
        sum += i
    for i in x:
        ans += f(i)
    return numpy.log(sum) - ans / sum

C = [26,26]

A = [25,1]
B = [1,25]

print(entropy(A))  
print(entropy(B))
print('E(A)+E(B)',entropy(A)/2.0+entropy(B)/2.0)
print('E(A+B)',entropy(C))



