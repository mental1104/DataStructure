#include "Stack.h"
#include <iostream>
#include <string>

typedef enum {AVAILABLE, ROUTE, BACKTRACKED, WALL} Status;

typedef enum {UNKNOWN, EAST, SOUTH, WEST, NORTH, NO_WAY} ESWN;

inline ESWN nextESWN(ESWN eswn) {
    return ESWN(eswn + 1);
}

struct Cell {
    int x, y;
    Status status;
    ESWN incoming, outgoing;
};

#define LABY_MAX 24

Cell laby[LABY_MAX][LABY_MAX];

inline Cell* neighbor(Cell* cell){
    switch(cell->outgoing) {
        case EAST : return cell + LABY_MAX;
        case SOUTH : return cell + 1;
        case WEST : return cell - LABY_MAX;
        case NORTH : return cell - 1;
        default : exit(-1);
    }
}

inline Cell* advance(Cell* cell) {
    Cell* next;
    switch(cell->outgoing){
        case EAST:  next = cell + LABY_MAX; next->incoming = WEST;  break;
        case SOUTH: next = cell + 1;        next->incoming = NORTH; break;
        case WEST:  next = cell - LABY_MAX; next->incoming = EAST;  break;
        case NORTH: next = cell - 1;        next->incoming = SOUTH; break;
        default   : exit(-1);
    }
    return next;
}

/******************************************************************************************
 *   输出某一迷宫格的信息
 ******************************************************************************************/
void printLabyCell ( Cell* elem ) {
   printf ( "%d -> (%d, %d) -> %d\n",
            ( ( Cell* ) elem )->incoming,
            ( ( Cell* ) elem )->x,
            ( ( Cell* ) elem )->y,
            ( ( Cell* ) elem )->outgoing );
}

/******************************************************************************************
 * 显示迷宫
 ******************************************************************************************/

int labySize;
Cell* startCell;
Cell* goalCell;

void displayLaby() { //┘└┐┌│─
   static const char*   pattern[5][5] = {
      "┼", "┼", "┼", "┼", "┼",
      "┼", "  ", "┌", "─", "└",
      "┼", "┌", "  ", "┐", "│",
      "┼", "─", "┐", "  ", "┘",
      "┼", "└", "│", "┘", "  "
   };
   system ( "clear" );
   printf ( " " );
   for ( int j = 0; j < labySize; j++ )
      ( j < 10 ) ? printf ( "%X", j ) : printf ( "%c", 'A' - 10 + j );
   printf ( "\n" );
   for ( int j = 0; j < labySize; j++ ) {
      ( j < 10 ) ? printf ( "%X", j ) : printf ( "%c", 'A' - 10 + j );
      for ( int i = 0; i < labySize; i++ )
         if ( goalCell == &laby[i][j] )
            printf ( "$" );
         else
            switch ( laby[i][j].status ) {
               case WALL:  printf ( "█" );   break;
               case BACKTRACKED: printf ( "○" );   break;
               case AVAILABLE: printf ( " " );   break;
               default   : printf ( "%s", pattern[laby[i][j].outgoing][laby[i][j].incoming] );   break;
            }
      printf ( "\n" );
   }//for
}//displayLaby

/******************************************************************************************
 * 读取迷宫文件
 ******************************************************************************************/
void readLaby ( char* labyFile ) { //śÁČëĂÔšŹ
   FILE* fp;
   if ( ! ( fp = fopen ( labyFile, "r" ) ) )
      { std::cout << "can't open " << labyFile << std::endl; exit ( -1 ); }
   fscanf ( fp, "Laby Size = %d\n", &labySize );
   if ( LABY_MAX < labySize )
      { std::cout << "Laby size " << labySize << " > " << LABY_MAX << std::endl; exit ( -1 ); }
   int startX, startY; fscanf ( fp, "Start = (%d, %d)\n", &startX, &startY );
   startCell = &laby[startX][startY];
   int goalX, goalY; fscanf ( fp, "Goal = (%d, %d)\n", &goalX, &goalY );
   goalCell = &laby[goalX][goalY];
   for ( int j = 0; j < labySize; j ++ )
      for ( int i = 0; i < labySize; i ++ ) {
         laby[i][j].x = i;
         laby[i][j].y = j;
         int type; fscanf ( fp, "%d", &type );
         switch ( type ) {
            case 1:   laby[i][j].status = WALL;      break;
            case 0:   laby[i][j].status = AVAILABLE;   break;
            default:   exit ( -1 );
         }
         laby[i][j].incoming =
            laby[i][j].outgoing =   UNKNOWN;
      }
   fclose ( fp );
}

/******************************************************************************************
 * 算法细节
 ******************************************************************************************/
bool labyrinth ( Cell Laby[LABY_MAX][LABY_MAX], Cell* s, Cell* t ) {
   if ( ( AVAILABLE != s->status ) || ( AVAILABLE != t->status ) ) return false; //退化情况
   Stack<Cell*> path; //用栈记录通路（Theseus的线绳）
   s->incoming = UNKNOWN; s->status = ROUTE; path.push ( s ); //起点
   do { //从起点出发不断试探、回溯，直到抵达终点，或者穷尽所有可能
      /*DSA*/displayLaby(); /*path.traverse(printLabyCell); printLabyCell(path.top());*/ getchar();
      Cell* c = path.top(); //检查当前位置（栈顶）
      if ( c == t ) return true; //若已抵达终点，则找到了一条通路；否则，沿尚未试探的方向继续试探
      while ( ( c->outgoing = nextESWN ( c->outgoing ) ) < NO_WAY) //逐一检查所有方向
         if ( AVAILABLE == neighbor ( c )->status ) break; //试图找到尚未试探的方向
      if ( c->outgoing >= NO_WAY) //若所有方向都已尝试过
         { c->status = BACKTRACKED; c = path.pop(); }//则向后回溯一步
      else //否则，向前试探一步
         { path.push ( c = advance ( c ) ); c->outgoing = UNKNOWN; c->status = ROUTE; }
   } while ( !path.empty() );
   return false;
}

int main(){
   srand ( ( unsigned int ) time ( NULL ) ); //设置随机种子
   readLaby (const_cast<char*>("laby/laby_00.txt")) ; //使用指定迷宫文件，或随机生成
   labyrinth ( laby, startCell, goalCell ) ? //启动算法
   printf ( "\nRoute found\a\n" ) :
   printf ( "\nNo route found\a\n" );
   getchar();
   return 0;
}