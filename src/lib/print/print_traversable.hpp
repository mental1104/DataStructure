#pragma once

template <typename T> 
void UniPrint::p ( T& s ) { 
   //printf ( "Type: %s Address: [%p] Size: %d:\n", typeid(s).name(), reinterpret_cast<unsigned*>(&s), s.size()); //fundamental information
   s.traverse ( print ); //traverse
   printf ("\n");
}