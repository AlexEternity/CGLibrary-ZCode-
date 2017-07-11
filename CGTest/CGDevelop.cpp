#include "StdAfx.h"
#include "CGLib.h"
#include <set>

// ******* кэш треугольников ********
CGTrianPartition *CGTrianPartition::Expand()				// расширить в 2x2 раза и пересчитать динамический кэш
{
	// TODO
	return NULL;	// здесь должен быть указатель на новый объект-разбиение
}

//*******************************************************

// ************ триангул€ци€ ****************************

int CGTIN::BuildZ()										// построение тр-ции по всем точкам исходного набора _Points
{
	Zcode *z=new Zcode(_Points->Size());
	z->BuildZ(_Points);
	for (int i = 0; i < _Points->Size(); i=i+1)
		IncludePoint(_Points->operator[](z->GetZind(i)));
	return 0;		// или код ошибки
}


void CGTIN::FillTrianPartition()							// заполнение кэша треугольников (итеративный алгоритм)
{

}

void CGTIN::FillPointPartition()							// заполнение кэша точек (пр€мой алгоритм)
{
	// TODO
}

CGPoint *CGTIN::IncludePoint(CGPoint *pnt)					// включение точки pnt в триангул€цию (локализаци€ + перестроение окрестности)
{
	vector<CGTrian *> _DQ;
	CGTrian *tri = _Trians[_Trians.size() - 1];
	while (1)
	{
		CGVector2D a(tri->A, tri->B);
		CGVector2D b(tri->A, pnt);
		if (a.operator^(b) >= 0)
		{
			CGVector2D a(tri->B, tri->C);
			CGVector2D b(tri->B, pnt);
			if (a.operator^(b) >= 0)
			{
				CGVector2D a(tri->C, tri->A);
				CGVector2D b(tri->C, pnt);
				if (a.operator^(b) >= 0) break;
				else 
				{
					tri = tri->CA;
				}
			}
			else 
			{
				tri = tri->BC;
			}
		}
		else 
		{
			tri = tri->AB;
		}
		continue;
	}
	AddPointToTrian(pnt, tri);
	_DQ.push_back(tri);
	_DQ.push_back(tri->BC);
	_DQ.push_back(tri->CA);
	for (int i = 0; i < _DQ.size(); i++)
	{
		if (!IsDelaunayPair(_DQ[i]))
		{
			DoFlip(_DQ[i]);
			_DQ.push_back(_DQ[i]);
			_DQ.push_back(_DQ[i]->BC);
		}
	}

	return NULL;	// или указатель на вершину тр-ции, с которой совпадает pnt
}

bool CGTIN::IsDelaunayPair(CGTrian *tri)					// проверка услови€ ƒелоне дл€ тр-ка tri и его соседа по AB - tnei
{
	if (tri->AB == NULL) return true;
	CGPoint *a, *b, *c, *d;
	CGTrian *tnei = tri->AB;
	a = tri->A; b = tri->B; c = tri->C;

	// d - противолежаща€ вершина tnei
	if (tnei->AB == tri) d = tnei->C;
	else if (tnei->BC == tri) d = tnei->A;
	else d = tnei->B;

	// проверка услови€ ƒелоне дл€ тр-ка abc и точки d
	double cosina = (d->X - a->X)*(d->X - b->X) + (d->Y - a->Y)*(d->Y - b->Y);
	double cosinb = (c->X - a->X)*(c->X - b->X) + (c->Y - a->Y)*(c->Y - b->Y);

	if (cosina >= 0 && cosinb >= 0) return true;
	else if (cosina < 0 && cosinb < 0) return false;
	else
	{
		double sinc = abs((c->X - a->X)*(c->Y - b->Y) - (c->X - b->X)*(c->Y - a->Y));
		double sind = abs((d->X - a->X)*(d->Y - b->Y) - (d->X - b->X)*(d->Y - a->Y));
		double sinsum = sind*cosinb + cosina*sinc;
		if (sinsum >= 0) return true;
		else return false;
	}
}
//*******************************************************
