#include "dsa_string.h"
#include "print.h"

int main(){
    String s("Necrozma");
    String str(s);
    char r = 'r';
    print(s+r);
    
    printf("%c %c\n", s.front(), s.back());//测试front与back
    printf("%c %c %c\n", s[0],s[1],s[2]);//测试下标运算符

    str = String(str);//测试拷贝赋值
    String t;
    t = "come on";//测试拷贝构造赋值运算符
    printf("%c %c %c\n", t.charAt(0), t.charAt(1), t.charAt(2));
    printf("%u\n", str.size());

    if(s == str)//测试判等符
        printf("s and str are equal.\n");
    if(s != t)//测试不等符
        printf("s and t are not equal.\n");

    print(str.substr(7, 5));//子串
    print(str.substr(7));
    putchar('\n');
    print(str);
    putchar('\n');
    print(t);
    putchar('\n');

    print(str.prefix(3));
    putchar('\n');

    print(str.suffix(3));
    putchar('\n');

    str.concat(t);
    str = str+t;
    print(str);
    putchar('\n');
    printf("%u", str.size());
    putchar('\n');
    printf("%c %c\n", str.front(), str.back());

    str = str + " This one";//Test for C-Stype String
    print(str);
    putchar('\n');
    printf("%u", str.size());
    putchar('\n');
    printf("%c %c\n", str.front(), str.back());

    printf("========================================\n");  

    String S("data structures");
    printf("length()                                     %u\n", S.size());
    printf("charAt(5)                                    %c\n", S.charAt(5));
    
    printf("prefix(4)                                    ");
    print(S.prefix(4));
    putchar('\n');

    printf("suffix(10)                                   ");
    print(S.suffix(10));
    putchar('\n');  

    S.concat(" and algorithms");
    printf("concat(\" and algorithms\")                    ");

    print(S);
    putchar('\n');

    printf("equal(\"data structures\")                     %d\n", S.equal("data structures"));
    printf("equal(\"data structures and algorithms\")      %d\n", S.equal("data structures and algorithms"));

    return 0;
}
