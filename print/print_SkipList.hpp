#pragma once

/******************************************************************************************
 * Skiplist
 ******************************************************************************************/

template <typename K, typename V> 
void UniPrint::p ( Skiplist<K, V>& s ) { 
   printf ( "%s[%p]*%d*%d:\n", typeid ( s ).name(), reinterpret_cast<unsigned*>(&s), s.level(), s.size() ); //������Ϣ
   s.traverse ( print ); 
   printf ( "\n" );
}
