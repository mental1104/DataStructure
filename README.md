# DSA

## 简介

本仓库主要涵盖《数据结构-邓俊辉》和 *Algorithm 4th* 中的代码:

使用方法： 
测试操作系统：macOS/Ubuntu 22.04    
测试编译器：g++/clang  
需要安装cmake  

操作方式：`sh entry.sh`  
构建出来的二进制文件在build文件夹下。  

参考资料：
+ Algorithms 4th: [Offical Website](https://algs4.cs.princeton.edu/home/)
+ Algorithms 4th: [Java Code](https://algs4.cs.princeton.edu/code/)
+ Algorithms 4th: [sample data](https://algs4.cs.princeton.edu/code/algs4-data.zip)
+ DSACPP: [Deng JunHui](https://dsa.cs.tsinghua.edu.cn/~deng/ds/dsacpp/)

## 待办事项

+ 更新格式化输出器
+ 更新单测


运行测试需要安装gtest

ubuntu:

```sh
sudo apt install -y libgtest-dev && cd /usr/src/googletest && cmake . && make -j$(nproc) && make install
cd cpp && mkdir build && cd build && cmake .. && make -j8
```

运行测试

```sh
ctest -r
```

