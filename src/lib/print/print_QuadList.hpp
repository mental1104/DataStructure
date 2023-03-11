#pragma once

template <typename T>
void UniPrint::p ( Quadlist<T>& q ) { //����
   printf ( "%s[%p]*%03d: ", typeid(q).name(), reinterpret_cast<unsigned*>(&q), q.size() ); //������Ϣ
   if ( q.size() <= 0 ) {  printf ( "\n" ); return;  }
   QuadlistNode<T>* curr = q.first()->pred; 
   QuadlistNode<T>* base = q.first(); 
   while ( base->below ) base = base->below; 
   while ( base->pred ) base = base->pred; 
   for ( int i = 0; i < q.size(); i++ ) { 
      curr = curr->succ; //curr
      QuadlistNode<T>* proj = curr; 
      while ( proj->below ) proj = proj->below; 
      while ( ( base = base->succ ) != proj ) 
         printf ( "---------------" ); 
      print ( curr->entry ); 
   }
   printf ( "\n" );
}
