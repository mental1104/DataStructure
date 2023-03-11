#pragma once

/******************************************************************************************
 * Entry
 ******************************************************************************************/
template <typename K, typename V>
void UniPrint::p ( Entry<K, V>& e ) 
{  printf ( "-<" ); print ( e.key ); printf ( ":" ); print ( e.value ); printf ( ">-" );  }
