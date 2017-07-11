#pragma once
#include <vector>
#include <string>
#include <list>
#include <bitset>
#include<iostream>
#include "stdafx.h"
using namespace std;

// типы точек
#define SOURCE_POINT 0				// исходная точка
#define BORDER_POINT 1				// вершина границы региона / тр-ции
#define CALC_POINT 2				// расчетная точка (пока не используется)

// коды ошибок
#define NO_DATA -1					// в регионе не задан исходный набор точек
#define WRONG_REGION -2				// неверные данные - невозможно рассчитать регион

#define REL_TOLERANCE 1e-5			// относительная погрешность задания координат точек
#define POINTS_PER_CELL 2			// ср.число точек на ячейку кэша точек (прямой алгоритм)
#define NX_MIN 4					// минимальное/начальное число ячеек любого кэша по оси X			
#define NY_MIN 4					// минимальное/начальное число ячеек любого кэша по оси Y
#define BORDER_SHIFT_PERC 5			// процент расширения региона по X и Y при расчете его границ

class CGRegion;
class CGTIN;

// ****** параллелепипед в пространстве ******
struct CGBox
{
	double Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;
};
// *******************************************

// *********** 3D-точка *************
class CGPoint
{
	double _X, _Y, _Z;
	int _Type;					// SOURCE_POINT | BORDER_POINT | CALC_POINT

public:
	CGPoint(double x, double y, double z, int type = SOURCE_POINT)
	{ _X = x; _Y = y; _Z = z; _Type = type;	}

	bool IsBorderPoint()
	{ return (_Type & BORDER_POINT)? true : false; }

	bool IsCalcPoint()
	{ return (_Type & CALC_POINT)? true : false; }

	double GetX() { return _X; }
	double GetY() { return _Y; }
	double GetZ() { return _Z; }

	__declspec(property(get = GetX)) double X;
	__declspec(property(get = GetY)) double Y;
	__declspec(property(get = GetZ)) double Z;
};
// *************************************

// ****** вектор на плоскости XOY ******
class CGVector2D
{
protected:
	double _dX, _dY;

public:
	CGVector2D(CGPoint *a, CGPoint *b)
	{ _dX = b->X - a->X; _dY = b->Y - a->Y; }

	double Length() { return sqrt(_dX * _dX + _dY * _dY); }

	double operator*(CGVector2D &vec)				// скалярное произведение
	{ return _dX * vec._dX + _dY * vec._dY; }

	double operator^(CGVector2D &vec)				// векторное произведение
	{ return _dX * vec._dY - vec._dX * _dY; }
};
// *************************************

// ****** набор точек / линия **********
class CGPointSet
{
protected:
	vector<CGPoint *> _PntSet;
	bool _Line;
	bool _Closed;

public:
	CGPointSet() { _Line = _Closed = false; }

	CGPointSet(int pnum, bool line = false, bool closed = false)	// зарезервировать память под набор из pnum точек
	{
		_Line = line; _Closed = closed;
		if (pnum > 0) _PntSet.reserve(pnum);
	}

	~CGPointSet()											// удалить все точки и очистить набор
	{
		int n = _PntSet.size();
		for (int i = 0; i < n; i++) delete _PntSet[i];
		_PntSet.clear();
	}

	void Add(CGPoint *pnt) { _PntSet.push_back(pnt); }		// добавить точку в конец набора (единственный способ заполнения набора)

	int Size() { return _PntSet.size(); }

	CGPoint* operator[](int ind)							// только для чтения из набора
	{
		if (ind < 0 || ind >= _PntSet.size()) return NULL;
		return _PntSet[ind]; 
	}

	bool IsLine() { return _Line; }
	bool IsClosed() { return _Closed; }
};
// *************************************

// ******* треугольник в триангуляции ******
class CGTrian
{
	CGPoint *_A, *_B, *_C;				// обход вершин тр-ка - против час. стрелки
	CGTrian *_AB, *_BC, *_CA;			// указатели на соседние тр-ки или NULL (граничное ребро тр-ции)

public:
	CGTrian() { _A = _B = _C = NULL; _AB = _BC = _CA = NULL; }

	CGTrian(CGPoint *a, CGPoint *b, CGPoint *c, CGTrian *ab = NULL, CGTrian *bc = NULL, CGTrian *ca = NULL)
	{ _A = a; _B = b; _C = c; _AB = ab; _BC = bc; _CA = ca; }

	void SetVertices(CGPoint *a, CGPoint *b, CGPoint *c)
	{ _A = a; _B = b; _C = c; }

	void SetNeighbors(CGTrian *ab, CGTrian *bc, CGTrian *ca)
	{ _AB = ab; _BC = bc; _CA = ca; }

	void SetA(CGPoint *a) { _A = a; }
	void SetB(CGPoint *b) { _B = b; }
	void SetC(CGPoint *c) { _C = c; }
	CGPoint *GetA() { return _A; }
	CGPoint *GetB() { return _B; }
	CGPoint *GetC() { return _C; }
	void SetAB(CGTrian *ab) { _AB = ab; }
	void SetBC(CGTrian *bc) { _BC = bc; }
	void SetCA(CGTrian *ca) { _CA = ca; }
	CGTrian *GetAB() { return _AB; }
	CGTrian *GetBC() { return _BC; }
	CGTrian *GetCA() { return _CA; }

	__declspec(property(get = GetA, put = SetA)) CGPoint *A;
	__declspec(property(get = GetB, put = SetB)) CGPoint *B;
	__declspec(property(get = GetC, put = SetC)) CGPoint *C;
	__declspec(property(get = GetAB, put = SetAB)) CGTrian *AB;
	__declspec(property(get = GetBC, put = SetBC)) CGTrian *BC;
	__declspec(property(get = GetCA, put = SetCA)) CGTrian *CA;
	
};
// *************************************

// ***** кэш треугольников (клеточное разбиение) для итеративного алгоритма ******
class CGTrianPartition
{
	CGBox *_RegionBox;						// регион для построения
	int _NX, _NY;							// число ячеек по X и Y
	double _DX, _DY;						// размер ячейки по X и Y
	vector<CGTrian *> _CellTrian;			// массив тр-ков для всех ячеек разбиения

	CGTrianPartition(CGBox *box, int nx = NX_MIN, int ny = NY_MIN);		// резервирование памяти без заполнения кэша
	void GetCellCoords(double x, double y, int &ix, int &iy);			// координаты ячейки по X и Y для точки (x,y)
	void SetTrianByPoint(CGTrian *, CGPoint *);							// сохранить тр-к в ячейке, определяемой точкой
	CGTrian *GetTrianByPoint(CGPoint *);								// получить тр-к в ячейке, определяемой точкой
	void SetTrianByCell(CGTrian *, int, int);							// сохранить тр-к в ячейке, определяемой ее координатами

	CGTrianPartition *Expand();				// CGDevelop.cpp: расширить в 2x2 раза и пересчитать динамический кэш

public:
	friend class CGTIN;

	~CGTrianPartition() { _CellTrian.clear(); }				// треугольники не удаляются!

	int GetNX() { return _NX; }
	int GetNY() { return _NY; }
	double GetDX() { return _DX; }
	double GetDY() { return _DY; }

	// свойства для доступа к параметрам кэша
	__declspec(property(get = GetNX)) int NX;
	__declspec(property(get = GetNY)) int NY;
	__declspec(property(get = GetDX)) double DX;
	__declspec(property(get = GetDY)) double DY;
};
// *************************************

// ****** кэш точек (клеточное разбиение) для прямого алгоритма ******
class CGPointPartition
{
	CGBox *_RegionBox;							// регион для построения
	int _NX, _NY;								// число ячеек по X и Y
	double _DX, _DY;							// размер ячейки по X и Y
	vector<list<CGPoint *> *> _CellPoints;		// массив указателей на списки точек для всех ячеек кэша

	CGPointPartition(CGBox *, int);							// резервирование памяти без заполнения кэша
	void GetCellCoords(double, double, int &, int &);		// координаты ячейки по X и Y для точки (x,y)
	void AddPoint(CGPoint *);								// добавление точки к списку соответствущей ячейки кэша
	list<CGPoint *> *GetCellPoints(int ix, int iy);			// указатель на список точек в ячейке (ix,iy)

public:
	friend class CGTIN;

	~CGPointPartition();						// освобождение списков точек и массива списков для всех ячеек

	int GetNX() { return _NX; }
	int GetNY() { return _NY; }
	double GetDX() { return _DX; }
	double GetDY() { return _DY; }

	// свойства для доступа к параметрам разбиения
	__declspec(property(get = GetNX)) int NX;
	__declspec(property(get = GetNY)) int NY;
	__declspec(property(get = GetDX)) double DX;
	__declspec(property(get = GetDY)) double DY;
};
// *************************************

// ********** триангуляция *************
class CGTIN
{
	CGRegion *_Region;					// регион для построения
	CGPointSet *_Points;				// исходный набор точек
	vector<CGTrian *> _Trians;			// набор треугольников
	CGPointSet *_Border;				// линия границы региона
	CGTrianPartition *_TrianPart;		// кэш тр-ков (для итеративного алгоритма)
	CGPointPartition *_PointPart;		// кэш точек (для алгоритма прямого построения)
	double _Tolerance;					// точность задания координат точек (общая для всех тр-ций региона)

	// методы, вызываемые в статической функции CreateTIN
	CGTIN(CGRegion *);
	int CreateBorder();				// расчет границы (замкнутая линия, 12 точек)
	int InitTIN();					// построение начальной тр-ции по 12 вершинам границы (итеративные алгоритмы)

	int BuildZ();					// CGDevelop.cpp: построение тр-ции по всем точкам исходного набора
	
	// вспомогательные методы BuildTIN
	void FillTrianPartition();						// CGDevelop.cpp: заполнение кэша треугольников (итеративный алгоритм)
	void FillPointPartition();						// CGDevelop.cpp: заполнение кэша точек (прямой алгоритм)

	CGPoint *IncludePoint(CGPoint *);				// CGDevelop.cpp: включение точки в триангуляцию (итеративные алгоритмы)
	bool IsDelaunayPair(CGTrian *);					// CGDevelop.cpp: проверка условия Делоне для тр-ка и его соседа по AB

	void AddPointToTrian(CGPoint *, CGTrian *);							// включение точки в тр-к (разбиение на 3 новых)
	void ChangeNeighbor(CGTrian *tri, CGTrian *told, CGTrian *tnew);	// замена ссылки на соседний с tri тр-к
	void DoFlip(CGTrian *);												// флип тр-ка и его соседа по AB
	double Cosin(CGPoint *A, CGPoint *B, CGPoint *C);					// косинус угла ABC (прямой алгоритм)

public:
	static CGTIN *CreateTIN(CGRegion *);					// создание объекта CGTIN для заданного региона
	~CGTIN();

	// методы для отрисовки тр-ции
	int GetTrianCount() { return _Trians.size(); }

	CGTrian* operator[](int tnum) { return _Trians[tnum]; }

	CGPointSet *GetBorder() { return _Border; }

	CGTrianPartition *GetTrianPartition() { return _TrianPart; }

	CGPointPartition *GetPointPartition() { return _PointPart; }

	
};
// *************************************

// ******* регион - область интереса ******
class CGRegion
{
	CGBox _RegionBox;							// расчетные границы области (квадрат на XOY) с учетом сдвига границы тр-ции
	vector<CGPointSet *> _PointSets;			// наборы точек
	vector<CGTIN *> _TINS;						// триангуляции (наборы треугольников)
	double _Tolerance;							// абс. погрешность задания координат точек

public:

	// создание и расчет региона по набору точек
	CGRegion() {}
	~CGRegion() { Clear(); }

	void Clear();									// удаление всех триангуляций и наборов точек, очистка векторов
	CGPointSet *CreateRandomPoints(int pnum);		// генерация набора pnum случайных точек [-10000, 10000] по X и Y (Z = 0)
	int Calculate();								// расчет региона (квадрат на XOY) с учетом сдвига границы тр-ции

	void AddPointSet(CGPointSet *pset) { _PointSets.push_back(pset); }

	double GetTolerance() { return _Tolerance; }

	void AddTIN(CGTIN *tin) { _TINS.push_back(tin); }

	// получение данных для рисования
	CGBox *GetRegionBox() { return &_RegionBox; }

	int GetPointSetCount() { return _PointSets.size(); }

	CGPointSet *GetPointSet(int num)
	{
		if (num < 0 || num >= _PointSets.size()) return NULL;
		return _PointSets[num];
	}

	int GetTINCount() { return _TINS.size(); }

	CGTIN *GetTIN(int num)
	{
		if (num < 0 || num >= _TINS.size()) return NULL;
		return _TINS[num];
	}
};
class Zcode
{
	string *z;
	int *zind;
public:
	Zcode(int N);
	void BuildZ(CGPointSet *arr);
	void quickSortR(int *zz, int N);
	int GetZind(int j)
	{
		return zind[j];
	}
};
// *************************************


