import os
import matplotlib.pyplot as plt

'''
Usage:

    method = 'insert' or 'remove' or 'search', not supporting any other method.
    init marks the number you want to begin with.
    step means how many times you want the init value to be amplified, e.g step = 2 means you want
    it to be doubled, step = 3 means you want it to be tripled.
    times, means the iteration number.

    e.g plot('insert', 1000000, 2, 5) means I want to generate the scale of:
    1, 2, 4, 8 and 16 millions by insertion
    plot('search', 2000000, 3, 3) means I want to generate the scale of:
    2, 4 and 8 millions by search.
    
'''
def plot(method, init, step, times):
    os.system('g++ -o {} {}.cpp -Og -w'.format(method, method))

    a = init

    for j in range(0, times):
        os.system('./{} {} {}'.format(method, a, 0))
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
        os.system('./{} {} {}'.format(method, a, 1))
        a *= step

    with open("./data.txt", 'r') as f:
        data = f.readlines()  # 将txt中所有字符串读入data
        resRB = []
        for line in data:
            numbers = line.split()        # 将数据分隔
            numbers_float = map(float, numbers) #转化为浮点数
            resRB.append(list(numbers_float))

    os.system("rm ./data.txt")

    a = init

    for j in range(0, times):
        os.system('./{} {} {}'.format(method, a, 2))
        a *= step

    with open("./data.txt", 'r') as f:
        data = f.readlines()  # 将txt中所有字符串读入data
        resSplay = []
        for line in data:
            numbers = line.split()        # 将数据分隔
            numbers_float = map(float, numbers) #转化为浮点数
            resSplay.append(list(numbers_float))

    os.system("rm ./data.txt")

    a = init

    for j in range(0, times):
        os.system('./{} {} {}'.format(method, a, 3))
        a *= step

    with open("./data.txt", 'r') as f:
        data = f.readlines()  # 将txt中所有字符串读入data
        resBTree = []
        for line in data:
            numbers = line.split()        # 将数据分隔
            numbers_float = map(float, numbers) #转化为浮点数
            resBTree.append(list(numbers_float))

    os.system("rm ./data.txt")

    os.system('rm {}'.format(method))

    xstick = []
    a = init
    for i in range(0, times):
        xstick.append("{}".format(a))
        a = a*step

    plt.grid(True, linestyle='--', alpha=0.5)
    plt.xlabel("Scale", fontdict={'size': 16})
    plt.ylabel("Times(s)", fontdict={'size': 16})

    title = ''
    if method == 'insert':
        title = 'Insertion'
    elif method == 'search':
        title = 'Search'
    elif method == 'remove':
        title = 'Removal'
    elif method == 'locality':
        title = 'Locality Search'

    plt.title("The Comparison of {}".format(title), fontdict={'size': 20})

    plt.plot(xstick, resAVL[0], c='blue', label="AVL Tree")
    plt.plot(xstick, resRB[0], c='red', label="RedBlack Tree")
    plt.plot(xstick, resSplay[0], c='green', label="Splay Tree")
    plt.plot(xstick, resBTree[0], c='orange', label="B-Tree")

    plt.scatter(xstick, resAVL[0], c='blue')
    plt.scatter(xstick, resRB[0], c='red')
    plt.scatter(xstick, resSplay[0], c='green')
    plt.scatter(xstick, resBTree[0], c='orange')

    plt.legend()
    plt.savefig("./{}.png".format(title))
    plt.close()

init = 500000
step = 2
times = 7

plot('insert',init, step, times)
plot('search',init, step, times)
plot('remove',init, step, times)
plot('locality', init, step, times)