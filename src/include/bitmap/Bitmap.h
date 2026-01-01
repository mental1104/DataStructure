#ifndef __DSA_BITMAP
#define __DSA_BITMAP

#include <cstdio>
#include <cstring>
#include <fstream>

class Bitmap { 
private:
   char* M; int N; 
protected:
   void init ( int n ) { M = new char[N = ( n + 7 ) / 8]; std::memset ( M, 0, N ); }
public:
   Bitmap ( int n = 8 ) { init ( n ); } 
   Bitmap ( const char* file, int n = 8 ) 
   {  init ( n ); std::ifstream in ( file, std::ios::binary ); if ( in ) in.read ( M, static_cast<std::streamsize> ( N ) );  }
   ~Bitmap() { delete [] M; M = NULL; } 

   void set   ( int k ) { expand ( k );        M[k >> 3] |=   ( 0x80 >> ( k & 0x07 ) ); }
   void clear ( int k ) { expand ( k );        M[k >> 3] &= ~ ( 0x80 >> ( k & 0x07 ) ); }
   bool test  ( int k ) { expand ( k ); return ( M[k >> 3] &    ( 0x80 >> ( k & 0x07 ) ) ) != 0; }

   void dump ( const char* file ) //
   {  std::ofstream out ( file, std::ios::binary ); if ( out ) out.write ( M, static_cast<std::streamsize> ( N ) );  }
   
   char* bits2string ( int n ) { 
      expand ( n - 1 ); 
      char* s = new char[n + 1]; s[n] = '\0'; 
      for ( int i = 0; i < n; i++ ) s[i] = test ( i ) ? '1' : '0';
      return s; 
   }
   void expand ( int k ) { 
      if ( k < 8 * N ) return; 
      int oldN = N; char* oldM = M;
      init ( 2 * k ); 
      std::memcpy( M, oldM, oldN ); delete [] oldM; 
   }
   /*DSA*/
   /*DSA*/   void print ( int n ) 
   /*DSA*/   {  expand ( n ); for ( int i = 0; i < n; i++ ) printf ( test ( i ) ? "1" : "0" );  }
};

#endif
