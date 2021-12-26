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
        resHT = []
        for line in data:
            numbers = line.split()        # 将数据分隔
            numbers_float = map(float, numbers) #转化为浮点数
            resHT.append(list(numbers_float))

    os.system("rm ./data.txt")

    
    a = init

    for j in range(0, times):
        os.system('./{} {} {}'.format(method, a, 1))
        a *= step

    with open("./data.txt", 'r') as f:
        data = f.readlines()  # 将txt中所有字符串读入data
        resQD = []
        for line in data:
            numbers = line.split()        # 将数据分隔
            numbers_float = map(float, numbers) #转化为浮点数
            resQD.append(list(numbers_float))

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

    plt.title("The Comparison of {}".format(title), fontdict={'size': 20})

    plt.plot(xstick, resHT[0], c='blue', label="Linear Probing")
    plt.plot(xstick, resQD[0], c='red', label="bidirectional Quadratic Probing")

    plt.scatter(xstick, resHT[0], c='blue')
    plt.scatter(xstick, resQD[0], c='red')

    plt.legend()
    plt.savefig("./{}.png".format(title))
    plt.close()

def conflict(init, step, times):
    os.system('g++ -o generator generator.cpp -Og -w')
    os.system('g++ -o conflict conflict.cpp -Og -w')

    a = init

    for j in range(0, times):
        os.system('./generator {}'.format(a))
        os.system('./conflict {}'.format(0))
        a *= step


    with open("./data.txt", 'r') as f:
        data = f.readlines()  # 将txt中所有字符串读入data
        resHT = []
        for line in data:
            numbers = line.split()        # 将数据分隔
            numbers_float = map(float, numbers) #转化为浮点数
            resHT.append(list(numbers_float))

    os.system("rm ./data.txt")

    
    a = init

    for j in range(0, times):
        os.system('./generator {}'.format(a))
        os.system('./conflict {}'.format(1))
        a *= step

    os.system('rm generator')

    with open("./data.txt", 'r') as f:
        data = f.readlines()  # 将txt中所有字符串读入data
        resQD = []
        for line in data:
            numbers = line.split()        # 将数据分隔
            numbers_float = map(float, numbers) #转化为浮点数
            resQD.append(list(numbers_float))

    os.system("rm ./data.txt")

    os.system('rm conflict')

    os.system('rm flawedData.txt')

   
        
    xstick = []
    a = init
    for i in range(0, times):
        xstick.append("{}".format(a))
        a = a*step

    plt.grid(True, linestyle='--', alpha=0.5)
    plt.xlabel("Scale", fontdict={'size': 16})
    plt.ylabel("Times(s)", fontdict={'size': 16})

    title = ''
    

    plt.title("The Comparison of Conflict", fontdict={'size': 20})

    plt.plot(xstick, resHT[0], c='blue', label="Linear Probing")
    plt.plot(xstick, resQD[0], c='red', label="bidirectional Quadratic Probing")

    plt.scatter(xstick, resHT[0], c='blue')
    plt.scatter(xstick, resQD[0], c='red')

    plt.legend()
    plt.savefig("./Conflict.png")
    plt.close()

init = 250000
step = 2
times = 5

plot('insert', init, step, times)
conflict(5000, step, times)
