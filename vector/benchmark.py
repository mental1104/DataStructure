import os
import matplotlib.pyplot as plt

init = 2
n = 5 #iteraton times
times = 2 #Next scale


os.system('g++ -o sort sort.cpp -Og')
os.system('g++ -o random randomGenerate.cpp -Og')
a = init #The beginning is 2K random numbers.
for i in range(0,n):
    os.system('./random {} {}'.format(a, 'K'))
    a = a*times # 2, 4, 8, 16, 32, 64, 128

for method in range(0,8):
    a = init
    for scale in range(0,n):
        os.system('./sort {} {} {}'.format(method,a,'K'))
        a = a*times
    with open('./data.txt', 'a') as f:
        f.write('\n')    
    


plt.figure(figsize=(10, 5), dpi=100)


with open("./data.txt", 'r') as f:
    data = f.readlines()  # 将txt中所有字符串读入data
    container = []
    for line in data:
        numbers = line.split()        # 将数据分隔
        numbers_float = map(float, numbers) #转化为浮点数
        container.append(list(numbers_float))

a = init
for i in range(0, n):
    os.system("rm ./{}{}ints.txt".format(a, 'K'))
    a = a*times

os.system("rm ./data.txt")
os.system("rm ./random")
os.system("rm ./sort")

xstick = []
a = 2
for i in range(0, n):
    xstick.append("{}".format(a))
    a = a*2

plt.grid(True, linestyle='--', alpha=0.5)
plt.xlabel("Scale", fontdict={'size': 16})
plt.ylabel("Times(s)", fontdict={'size': 16})
plt.title("The Comparison between Different Sorting Method", fontdict={'size': 20})
plt.plot(xstick, container[0], c='red', label="BubbleSort")
plt.plot(xstick, container[1], c='orange', label="SelectionSort")
plt.plot(xstick, container[2], c='yellow', label="InsertionSort")
plt.plot(xstick, container[3], c='green', linestyle='--', label="ShellSort")
plt.plot(xstick, container[4], c='blue', label="MergeSort(Top-down)")
plt.plot(xstick, container[5], c='blue', linestyle='--', label="MergeSort(Bottom-up)")
plt.plot(xstick, container[6], c='purple', label="QuickSort")
plt.plot(xstick, container[7], c='purple', linestyle='--', label="QuickSort(3way)")

#散点图
plt.scatter(xstick, container[0], c='red')
plt.scatter(xstick, container[1], c='orange')
plt.scatter(xstick, container[2], c='yellow')
plt.scatter(xstick, container[3], c='green')
plt.scatter(xstick, container[4], c='blue')
plt.scatter(xstick, container[5], c='blue')
plt.scatter(xstick, container[6], c='purple')
plt.scatter(xstick, container[7], c='purple')

plt.legend()
plt.savefig("./tiny.png")
plt.close()
