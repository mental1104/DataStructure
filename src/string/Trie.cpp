#include "Trie.h"

int main(){
    Trie<int> trie;
    if(trie.empty()){
        trie.put("by", 4);
        trie.put("sea", 6);
        trie.put("sells", 1);
        trie.put("she", -1);
        trie.put("shells", 3);
        trie.put("shore", 7);
        trie.put("the", 5);
        //trie.keysWithPrefix("sh");
        //trie.keysThatMatch("s...s");
        String s = trie.longestPrefixOf("shellsort");
        //print(s);
        putchar('\n');
        trie.keys();

        printf("%d\n", trie.size());
        printf("Contains 'shells'? %d\n", trie.contains("shells"));
        trie.remove("shells");
        trie.keys();
        printf("%d\n", trie.size());
        printf("Contains 'shells'? %d\n", trie.contains("shells"));
        
    }
    return 0;
}