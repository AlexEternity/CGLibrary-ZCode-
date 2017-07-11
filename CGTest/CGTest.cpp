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
	CGRegion reg;								// ������
	CGPointSet *pset;
	CGTIN *tin;
	int pnum;
	cout << "Point total: "; cin >> pnum;
	pset = reg.CreateRandomPoints(pnum);		// ������� ����� �� pnum ��������� �����
	if (!pset) return NO_DATA;

	reg.AddPointSet(pset);						// ��������� ����� ����� � �������		

	if (reg.Calculate()) return NO_DATA;		// ��������� ������� ������� (������� �� XOY) �� ��������� ������ �����
	clock_t time;
	time = clock();
	tin = CGTIN::CreateTIN(&reg);				// ������ ������������ � ������� �� ��������� ������ �����
	if (tin) reg.AddTIN(tin);					// ��������� ��-��� � ������ ��� ���������
	// ��-���� ������ � �������������
	time = clock() - time;
	cout << "WOrKING TIME " << (double)time / CLOCKS_PER_SEC;
	cin >> n;
	return 0;
}

