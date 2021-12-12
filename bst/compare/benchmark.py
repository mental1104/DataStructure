import os
import matplotlib.pyplot as plt

# Weighing the perfomance of insertion
os.system('g++ -o insert insert.cpp -Og')

init = 1000000
step = 2
times = 8

a = init

for j in range(0, times):
    os.system('./insert {} {}'.format(a, 0))
    a *= step

with open("./data.txt", 'r') as f:
    data = f.readlines()  # 将txt中所有字符串读入data
    resAVL = []
    for line in data:
        numbers = line.split()        # 将数据分隔
        numbers_float = map(float, numbers) #转化为浮点数
        resAVL.append(list(numbers_float))

os.system("rm ./data.txt")


a = init

for j in range(0, times):
    os.system('./insert {} {}'.format(a, 1))
    a *= step

with open("./data.txt", 'r') as f:
    data = f.readlines()  # 将txt中所有字符串读入data
    resRB = []
    for line in data:
        numbers = line.split()        # 将数据分隔
        numbers_float = map(float, numbers) #转化为浮点数
        resRB.append(list(numbers_float))

os.system("rm ./data.txt")

xstick = []
a = 1
for i in range(0, times):
    xstick.append("{}M".format(a))
    a = a*step

plt.grid(True, linestyle='--', alpha=0.5)
plt.xlabel("Scale", fontdict={'size': 16})
plt.ylabel("Times(s)", fontdict={'size': 16})
plt.title("The Comparison of Insertion", fontdict={'size': 20})

plt.plot(xstick, resAVL[0], c='blue', label="AVL Tree")
plt.plot(xstick, resRB[0], c='red', label="RedBlack Tree")

plt.scatter(xstick, resAVL[0], c='blue')
plt.scatter(xstick, resRB[0], c='red')

plt.legend()
plt.savefig("./insertion.png")
plt.close()



# Weighing the perfomance of searching
os.system('g++ -o search search.cpp -Og')

a = init 

for j in range(0, times):
    os.system('./search {} {}'.format(a, 0))
    a *= step

with open("./data.txt", 'r') as f:
    data = f.readlines()  # 将txt中所有字符串读入data
    resAVL = []
    for line in data:
        numbers = line.split()        # 将数据分隔
        numbers_float = map(float, numbers) #转化为浮点数
        resAVL.append(list(numbers_float))

os.system("rm ./data.txt")

a = init

for j in range(0, times):
    os.system('./search {} {}'.format(a, 1))
    a *= step

with open("./data.txt", 'r') as f:
    data = f.readlines()  # 将txt中所有字符串读入data
    container = []
    for line in data:
        numbers = line.split()        # 将数据分隔
        numbers_float = map(float, numbers) #转化为浮点数
        container.append(list(numbers_float))

os.system("rm ./data.txt")

xstick = []
a = 1
for i in range(0, times):
    xstick.append("{}M".format(a))
    a = a*step

plt.grid(True, linestyle='--', alpha=0.5)
plt.xlabel("Scale", fontdict={'size': 16})
plt.ylabel("Times(s)", fontdict={'size': 16})
plt.title("The Comparison of Search", fontdict={'size': 20})

plt.plot(xstick, resAVL[0], c='blue', label="AVL Tree")
plt.plot(xstick, resRB[0], c='red', label="RedBlack Tree")

plt.scatter(xstick, resAVL[0], c='blue')
plt.scatter(xstick, resRB[0], c='red')

plt.legend()
plt.savefig("./search.png")
plt.close()

os.system("rm ./insert")
os.system("rm ./search")