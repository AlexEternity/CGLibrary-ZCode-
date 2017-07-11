#pragma once
#include <vector>
#include <string>
#include <list>
#include <bitset>
#include<iostream>
#include "stdafx.h"
using namespace std;

// ���� �����
#define SOURCE_POINT 0				// �������� �����
#define BORDER_POINT 1				// ������� ������� ������� / ��-���
#define CALC_POINT 2				// ��������� ����� (���� �� ������������)

// ���� ������
#define NO_DATA -1					// � ������� �� ����� �������� ����� �����
#define WRONG_REGION -2				// �������� ������ - ���������� ���������� ������

#define REL_TOLERANCE 1e-5			// ������������� ����������� ������� ��������� �����
#define POINTS_PER_CELL 2			// ��.����� ����� �� ������ ���� ����� (������ ��������)
#define NX_MIN 4					// �����������/��������� ����� ����� ������ ���� �� ��� X			
#define NY_MIN 4					// �����������/��������� ����� ����� ������ ���� �� ��� Y
#define BORDER_SHIFT_PERC 5			// ������� ���������� ������� �� X � Y ��� ������� ��� ������

class CGRegion;
class CGTIN;

// ****** �������������� � ������������ ******
struct CGBox
{
	double Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;
};
// *******************************************

// *********** 3D-����� *************
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

// ****** ������ �� ��������� XOY ******
class CGVector2D
{
protected:
	double _dX, _dY;

public:
	CGVector2D(CGPoint *a, CGPoint *b)
	{ _dX = b->X - a->X; _dY = b->Y - a->Y; }

	double Length() { return sqrt(_dX * _dX + _dY * _dY); }

	double operator*(CGVector2D &vec)				// ��������� ������������
	{ return _dX * vec._dX + _dY * vec._dY; }

	double operator^(CGVector2D &vec)				// ��������� ������������
	{ return _dX * vec._dY - vec._dX * _dY; }
};
// *************************************

// ****** ����� ����� / ����� **********
class CGPointSet
{
protected:
	vector<CGPoint *> _PntSet;
	bool _Line;
	bool _Closed;

public:
	CGPointSet() { _Line = _Closed = false; }

	CGPointSet(int pnum, bool line = false, bool closed = false)	// ��������������� ������ ��� ����� �� pnum �����
	{
		_Line = line; _Closed = closed;
		if (pnum > 0) _PntSet.reserve(pnum);
	}

	~CGPointSet()											// ������� ��� ����� � �������� �����
	{
		int n = _PntSet.size();
		for (int i = 0; i < n; i++) delete _PntSet[i];
		_PntSet.clear();
	}

	void Add(CGPoint *pnt) { _PntSet.push_back(pnt); }		// �������� ����� � ����� ������ (������������ ������ ���������� ������)

	int Size() { return _PntSet.size(); }

	CGPoint* operator[](int ind)							// ������ ��� ������ �� ������
	{
		if (ind < 0 || ind >= _PntSet.size()) return NULL;
		return _PntSet[ind]; 
	}

	bool IsLine() { return _Line; }
	bool IsClosed() { return _Closed; }
};
// *************************************

// ******* ����������� � ������������ ******
class CGTrian
{
	CGPoint *_A, *_B, *_C;				// ����� ������ ��-�� - ������ ���. �������
	CGTrian *_AB, *_BC, *_CA;			// ��������� �� �������� ��-�� ��� NULL (��������� ����� ��-���)

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

// ***** ��� ������������� (��������� ���������) ��� ������������ ��������� ******
class CGTrianPartition
{
	CGBox *_RegionBox;						// ������ ��� ����������
	int _NX, _NY;							// ����� ����� �� X � Y
	double _DX, _DY;						// ������ ������ �� X � Y
	vector<CGTrian *> _CellTrian;			// ������ ��-��� ��� ���� ����� ���������

	CGTrianPartition(CGBox *box, int nx = NX_MIN, int ny = NY_MIN);		// �������������� ������ ��� ���������� ����
	void GetCellCoords(double x, double y, int &ix, int &iy);			// ���������� ������ �� X � Y ��� ����� (x,y)
	void SetTrianByPoint(CGTrian *, CGPoint *);							// ��������� ��-� � ������, ������������ ������
	CGTrian *GetTrianByPoint(CGPoint *);								// �������� ��-� � ������, ������������ ������
	void SetTrianByCell(CGTrian *, int, int);							// ��������� ��-� � ������, ������������ �� ������������

	CGTrianPartition *Expand();				// CGDevelop.cpp: ��������� � 2x2 ���� � ����������� ������������ ���

public:
	friend class CGTIN;

	~CGTrianPartition() { _CellTrian.clear(); }				// ������������ �� ���������!

	int GetNX() { return _NX; }
	int GetNY() { return _NY; }
	double GetDX() { return _DX; }
	double GetDY() { return _DY; }

	// �������� ��� ������� � ���������� ����
	__declspec(property(get = GetNX)) int NX;
	__declspec(property(get = GetNY)) int NY;
	__declspec(property(get = GetDX)) double DX;
	__declspec(property(get = GetDY)) double DY;
};
// *************************************

// ****** ��� ����� (��������� ���������) ��� ������� ��������� ******
class CGPointPartition
{
	CGBox *_RegionBox;							// ������ ��� ����������
	int _NX, _NY;								// ����� ����� �� X � Y
	double _DX, _DY;							// ������ ������ �� X � Y
	vector<list<CGPoint *> *> _CellPoints;		// ������ ���������� �� ������ ����� ��� ���� ����� ����

	CGPointPartition(CGBox *, int);							// �������������� ������ ��� ���������� ����
	void GetCellCoords(double, double, int &, int &);		// ���������� ������ �� X � Y ��� ����� (x,y)
	void AddPoint(CGPoint *);								// ���������� ����� � ������ �������������� ������ ����
	list<CGPoint *> *GetCellPoints(int ix, int iy);			// ��������� �� ������ ����� � ������ (ix,iy)

public:
	friend class CGTIN;

	~CGPointPartition();						// ������������ ������� ����� � ������� ������� ��� ���� �����

	int GetNX() { return _NX; }
	int GetNY() { return _NY; }
	double GetDX() { return _DX; }
	double GetDY() { return _DY; }

	// �������� ��� ������� � ���������� ���������
	__declspec(property(get = GetNX)) int NX;
	__declspec(property(get = GetNY)) int NY;
	__declspec(property(get = GetDX)) double DX;
	__declspec(property(get = GetDY)) double DY;
};
// *************************************

// ********** ������������ *************
class CGTIN
{
	CGRegion *_Region;					// ������ ��� ����������
	CGPointSet *_Points;				// �������� ����� �����
	vector<CGTrian *> _Trians;			// ����� �������������
	CGPointSet *_Border;				// ����� ������� �������
	CGTrianPartition *_TrianPart;		// ��� ��-��� (��� ������������ ���������)
	CGPointPartition *_PointPart;		// ��� ����� (��� ��������� ������� ����������)
	double _Tolerance;					// �������� ������� ��������� ����� (����� ��� ���� ��-��� �������)

	// ������, ���������� � ����������� ������� CreateTIN
	CGTIN(CGRegion *);
	int CreateBorder();				// ������ ������� (��������� �����, 12 �����)
	int InitTIN();					// ���������� ��������� ��-��� �� 12 �������� ������� (����������� ���������)

	int BuildZ();					// CGDevelop.cpp: ���������� ��-��� �� ���� ������ ��������� ������
	
	// ��������������� ������ BuildTIN
	void FillTrianPartition();						// CGDevelop.cpp: ���������� ���� ������������� (����������� ��������)
	void FillPointPartition();						// CGDevelop.cpp: ���������� ���� ����� (������ ��������)

	CGPoint *IncludePoint(CGPoint *);				// CGDevelop.cpp: ��������� ����� � ������������ (����������� ���������)
	bool IsDelaunayPair(CGTrian *);					// CGDevelop.cpp: �������� ������� ������ ��� ��-�� � ��� ������ �� AB

	void AddPointToTrian(CGPoint *, CGTrian *);							// ��������� ����� � ��-� (��������� �� 3 �����)
	void ChangeNeighbor(CGTrian *tri, CGTrian *told, CGTrian *tnew);	// ������ ������ �� �������� � tri ��-�
	void DoFlip(CGTrian *);												// ���� ��-�� � ��� ������ �� AB
	double Cosin(CGPoint *A, CGPoint *B, CGPoint *C);					// ������� ���� ABC (������ ��������)

public:
	static CGTIN *CreateTIN(CGRegion *);					// �������� ������� CGTIN ��� ��������� �������
	~CGTIN();

	// ������ ��� ��������� ��-���
	int GetTrianCount() { return _Trians.size(); }

	CGTrian* operator[](int tnum) { return _Trians[tnum]; }

	CGPointSet *GetBorder() { return _Border; }

	CGTrianPartition *GetTrianPartition() { return _TrianPart; }

	CGPointPartition *GetPointPartition() { return _PointPart; }

	
};
// *************************************

// ******* ������ - ������� �������� ******
class CGRegion
{
	CGBox _RegionBox;							// ��������� ������� ������� (������� �� XOY) � ������ ������ ������� ��-���
	vector<CGPointSet *> _PointSets;			// ������ �����
	vector<CGTIN *> _TINS;						// ������������ (������ �������������)
	double _Tolerance;							// ���. ����������� ������� ��������� �����

public:

	// �������� � ������ ������� �� ������ �����
	CGRegion() {}
	~CGRegion() { Clear(); }

	void Clear();									// �������� ���� ������������ � ������� �����, ������� ��������
	CGPointSet *CreateRandomPoints(int pnum);		// ��������� ������ pnum ��������� ����� [-10000, 10000] �� X � Y (Z = 0)
	int Calculate();								// ������ ������� (������� �� XOY) � ������ ������ ������� ��-���

	void AddPointSet(CGPointSet *pset) { _PointSets.push_back(pset); }

	double GetTolerance() { return _Tolerance; }

	void AddTIN(CGTIN *tin) { _TINS.push_back(tin); }

	// ��������� ������ ��� ���������
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


