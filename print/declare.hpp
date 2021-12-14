#pragma once

static void print ( char* x );
static void print ( const char* x );
template <typename T> static void print ( T& x );
template <typename T> static void print ( const T& x );
template <typename T> static void print ( T* x );