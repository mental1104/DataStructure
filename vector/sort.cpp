#include "./Vector.hpp"
#include <cstdio>

int main(int argc, char** argv){
    if(argc!=4){
        printf("Usage: %s <num>\n", argv[0]);
        return 0;
    }

    Vector<int> vec;
    std::string s("./");
    s = s + argv[2] + argv[3] + "ints.txt";
    std::ifstream stream(s);

    std::ofstream output;
    output.open("./data.txt", std::ios::app | std::ios::out);
    int temp;
    while(stream >> temp)
        vec.insert(temp);
    stream.close();

    int num = atoi(argv[1]);

    double val = 0.0;

    switch(num){
        case 1: 
            val = vec.sort(Sort::SelectionSort);
            break;
        case 2:
            val = vec.sort(Sort::InsertionSort);
            break;
        case 3:
            val = vec.sort(Sort::ShellSort);
            break;
        case 4:
            val = vec.sort(Sort::MergeSortA);
            break;
        case 5:
            val = vec.sort(Sort::MergeSortB);
            break;
        case 6:
            val = vec.sort(Sort::QuickSort);
            break;
        case 7:
            val = vec.sort(Sort::Quick3way);
            break;
        default:
            val = vec.sort(Sort::BubbleSort);
            break;
    }
    output << val << " ";
    output.close();
    return 0;
} 