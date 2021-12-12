#pragma once

#include <random>

std::random_device rd;
std::mt19937 eng(rd());
std::uniform_int_distribution<int> dis(0, 99999);

/******************************************************************************************
 * 在[0, range)内随机生成一个数
 ******************************************************************************************/
static int dice ( int range ) { return dis(eng) % range; } //取[0, range)中的随机整数
static int dice ( int lo, int hi ) { return lo + dis(eng) % ( hi - lo ); } //取[lo, hi)中的随机整数
static float dice ( float range ) { return dis(eng) % ( 1000 * ( int ) range ) / ( float ) 1000.; }
static double dice ( double range ) { return dis(eng) % ( 1000 * ( int ) range ) / ( double ) 1000.; }
static char dice ( ) { return ( char ) ( 32 + dis(eng) % 96 ); }