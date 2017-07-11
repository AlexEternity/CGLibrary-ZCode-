// CGTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CGLib.h"
#include <iostream>
#include<time.h>
using namespace std;


int _tmain(int argc, _TCHAR* argv[])
{
	int n;
	CGRegion reg;								// регион
	CGPointSet *pset;
	CGTIN *tin;
	int pnum;
	cout << "Point total: "; cin >> pnum;
	pset = reg.CreateRandomPoints(pnum);		// создаем набор из pnum случайных точек
	if (!pset) return NO_DATA;

	reg.AddPointSet(pset);						// добавляем набор точек к региону		

	if (reg.Calculate()) return NO_DATA;		// вычисляем границы региона (квадрат на XOY) по заданному набору точек
	clock_t time;
	time = clock();
	tin = CGTIN::CreateTIN(&reg);				// строим триангуляцию в регионе по заданному набору точек
	if (tin) reg.AddTIN(tin);					// добавляем тр-цию в список для рисования
	// чо-нить делаем с триангуляцией
	time = clock() - time;
	cout << "WOrKING TIME " << (double)time / CLOCKS_PER_SEC;
	cin >> n;
	return 0;
}

