extern int s_lo, s_hi;

int gs_LS( int A[], int n ) { //扫描策略：O(n)
   int gs = A[0], s = 0, i = n, j = n;
   while ( 0 < i-- ) { //在当前区间[i,j)内
      s += A[i]; //递增地累计总和
      if ( gs < s ) { gs = s; s_lo = i; s_hi = j; } //择优、更新
      if ( s <= 0 ) { s = 0; j = i; } //剪除负和后缀
   }
   return gs;
}
