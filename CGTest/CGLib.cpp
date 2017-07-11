#include "stdafx.h"
#include "CGLib.h"
#include <time.h>

Zcode::Zcode(int N)
{
	z = new string[N];
	zind = new int[N];
	for (int i = 0; i < N; i++)
	{
		zind[i] = i;
	}
}

void Zcode::BuildZ(CGPointSet *arr)
{
	for (int j = 0; j <arr->Size(); j++)
	{
		/*Ѕитовое представление и перевод в строку*/
		bitset < sizeof(arr->operator[](j)->GetX())*CHAR_BIT> bx(arr->operator[](j)->GetX());
		bitset < sizeof(arr->operator[](j)->GetY())*CHAR_BIT> by(arr->operator[](j)->GetY());
		bitset <128> zx;
		//‘ормирование z-кода
		int k = 0;
		for (int j = 0; j < 128; j += 2)
		{
			if (bx[k] == 1)
				zx.set(j);
			if (by[k] == 1)
				zx.set(j + 1);
			k++;
		}
		z[j] = zx.to_string();
	}
	quickSortR(zind,arr->Size() - 1);
}

void Zcode::quickSortR(int *zz, int N)
{
	int i = 0, j = N;
	//double temp,temp1,temp2, p;
	string temp1, p;
	double temp;
	//p = z1[(N+i)/2];
	p = z[zz[(N + i) / 2]];
	do {
		while (z[zz[i]] < p)
				i++;
		while (z[zz[j]] > p)
				j--;
		if (i <= j) {
			temp = zz[i];
			zz[i] = zz[j];
			zz[j] = temp;
			/*temp1 = z1[i];
			z1[i] = z1[j];
			z1[j] = temp1;
			temp2 = z2[i];
			z2[i] = z2[j];
			z2[j] = temp2;*/
			i++; j--;
		}
	} while (i <= j);
	if (j > 0) quickSortR(zz, j);
	if (N > i) quickSortR(zz + i, N - i);
}

// ******* кэш треугольников ********
CGTrianPartition::CGTrianPartition(CGBox *box, int nx, int ny)				// резервирование пам€ти без заполнени€ кэша
{
	int n;
	_RegionBox = box;
	if (nx < NX_MIN) _NX = NX_MIN;
	if (ny < NY_MIN) _NY = NY_MIN;
	_DX = (box->Xmax - box->Xmin) / _NX;
	_DY = (box->Ymax - box->Ymin) / _NY;
	n = _NX * _NY;
	_CellTrian.assign(n, NULL);
}

void CGTrianPartition::GetCellCoords(double x, double y, int &ix, int &iy)	// номера €чейки по X и Y дл€ точки (x,y)
{
	ix = (int)((x - _RegionBox->Xmin) / _DX);
	iy = (int)((y - _RegionBox->Ymin) / _DX);
	if (ix < 0) ix = 0; else if (ix >= _NX) ix = _NX - 1;
	if (iy < 0) iy = 0; else if (iy >= _NY) iy = _NY - 1;
}

void CGTrianPartition::SetTrianByPoint(CGTrian *tri, CGPoint *pnt)			// сохранить тр-к в €чейке, определ€емой точкой
{
	int ix, iy;
	GetCellCoords(pnt->X, pnt->Y, ix, iy);
	_CellTrian[_NX * iy + ix] = tri;
}

CGTrian* CGTrianPartition::GetTrianByPoint(CGPoint *pnt)					// получить тр-к из €чейки, определ€емой точкой
{
	int ix, iy;
	GetCellCoords(pnt->X, pnt->Y, ix, iy);
	return _CellTrian[_NX * iy + ix];
}

void CGTrianPartition::SetTrianByCell(CGTrian *tri, int ix, int iy)		// сохранить тр-к в €чейке, заданной ее координатами
{ 
	if (ix < 0 || ix >= _NX || iy < 0 || iy >= _NY) return;
	_CellTrian[_NX * iy + ix] = tri; 
}

//*******************************************************

// ***************** кэш точек *********************
CGPointPartition::CGPointPartition(CGBox *box, int pnum)				// резервирование пам€ти без заполнени€ кэша
{
	int i, n;
	_RegionBox = box;

	// расчет числа €чеек разбиени€ по X и Y дл€ pnum точек
	_NX = _NY = (int)sqrt((double)pnum / POINTS_PER_CELL) + 1;
	if (_NX < NX_MIN) _NX = NX_MIN;
	if (_NY < NY_MIN) _NY = NY_MIN;
	_DX = (box->Xmax - box->Xmin) / _NX;
	_DY = (box->Ymax - box->Ymin) / _NY;
	n = _NX * _NY;
	_CellPoints.reserve(n);
	for (i = 0; i < n; i++) _CellPoints.push_back(new list<CGPoint *>);
}

CGPointPartition::~CGPointPartition()									// освобождение списков точек и массива списков
{
	int n = _CellPoints.size();
	for (int i = 0; i < n; i++)
	{
		_CellPoints[i]->clear(); delete _CellPoints[i];
	}
	_CellPoints.clear();
}

void CGPointPartition::GetCellCoords(double x, double y, int &ix, int &iy)		// координаты €чейки по X и Y дл€ точки (x,y)
{
	ix = (int)((x - _RegionBox->Xmin) / _DX);
	iy = (int)((y - _RegionBox->Ymin) / _DX);
	if (ix < 0) ix = 0; else if (ix >= _NX) ix = _NX - 1;
	if (iy < 0) iy = 0; else if (iy >= _NY) iy = _NY - 1;
}

void CGPointPartition::AddPoint(CGPoint *pnt)							// добавление точки к списку соот-щей €чейки кэша
{
	int ix, iy;
	GetCellCoords(pnt->X, pnt->Y, ix, iy);
	_CellPoints[_NX * iy + ix]->push_back(pnt);
}

list<CGPoint *> *CGPointPartition::GetCellPoints(int ix, int iy)		// указатель на список точек в €чейке (ix,iy)
{
	return _CellPoints[_NX * iy + ix];
}
//*******************************************************

// ********* триангул€ци€********************************
CGTIN *CGTIN::CreateTIN(CGRegion *reg)						// создание объекта CGTIN и построение триангул€ции в заданном регионе
{
	CGTIN *tin;
	if (reg == NULL || reg->GetPointSetCount() < 1) return NULL;
	tin = new CGTIN(reg);
	if (tin->CreateBorder()) { delete tin; return NULL; }	// расчет граничной линии (12 вершин)
	if (tin->InitTIN()) { delete tin; return NULL; }		// построение начальной тр-ции (10 тр-ков) по вершинам границы (итеративные алгоритмы)
		if (tin->BuildZ()) { delete tin; return NULL; }
	return tin;
}

CGTIN::CGTIN(CGRegion *reg)									// создание объекта CGTIN дл€ заданного региона
{
	_Region = reg; _Points = reg->GetPointSet(0);
	_Border = NULL;
	_TrianPart = NULL; _PointPart = NULL;
	_Tolerance = reg->GetTolerance();
}

CGTIN::~CGTIN()												// удаление всех объектов тр-ции
{
	if (_TrianPart != NULL) delete _TrianPart;
	if (_PointPart != NULL) delete _PointPart;
	if (_Border != NULL) delete _Border;
	int n = _Trians.size();
	for (int i = 0; i < n; i++) delete _Trians[i];
	_Trians.clear();
}

int CGTIN::CreateBorder()									// расчет граничной линии (замкнута€, 12 точек)
{
	CGPoint *pnt;
	double dx, dy;
	if (_Region == NULL || _Points == NULL) return NO_DATA;
	CGBox *rbox = _Region->GetRegionBox();
	if (rbox->Xmax <= rbox->Xmin || rbox->Ymax <= rbox->Ymin) return WRONG_REGION;

	_Tolerance = _Region->GetTolerance();
	dx = (rbox->Xmax - rbox->Xmin) / 3;
	dy = (rbox->Ymax - rbox->Ymin) / 3;
	_Border = new CGPointSet(12, true, true);
	pnt = new CGPoint(rbox->Xmin, rbox->Ymin, 0, BORDER_POINT);			// 0
	_Border->Add(pnt);
	pnt = new CGPoint(rbox->Xmin + dx, rbox->Ymin, 0, BORDER_POINT);	// 1
	_Border->Add(pnt);
	pnt = new CGPoint(rbox->Xmax - dx, rbox->Ymin, 0, BORDER_POINT);	// 2
	_Border->Add(pnt);
	pnt = new CGPoint(rbox->Xmax, rbox->Ymin, 0, BORDER_POINT);			// 3
	_Border->Add(pnt);
	pnt = new CGPoint(rbox->Xmax, rbox->Ymin + dy, 0, BORDER_POINT);	// 4
	_Border->Add(pnt);
	pnt = new CGPoint(rbox->Xmax, rbox->Ymax - dy, 0, BORDER_POINT);	// 5
	_Border->Add(pnt);
	pnt = new CGPoint(rbox->Xmax, rbox->Ymax, 0, BORDER_POINT);			// 6
	_Border->Add(pnt);
	pnt = new CGPoint(rbox->Xmax - dx, rbox->Ymax, 0, BORDER_POINT);	// 7
	_Border->Add(pnt);
	pnt = new CGPoint(rbox->Xmin + dx, rbox->Ymax, 0, BORDER_POINT);	// 8
	_Border->Add(pnt);
	pnt = new CGPoint(rbox->Xmin, rbox->Ymax, 0, BORDER_POINT);			// 9
	_Border->Add(pnt);
	pnt = new CGPoint(rbox->Xmin, rbox->Ymax - dy, 0, BORDER_POINT);	// 10
	_Border->Add(pnt);
	pnt = new CGPoint(rbox->Xmin, rbox->Ymin + dy, 0, BORDER_POINT);	// 11
	_Border->Add(pnt);
	return 0;
}

int CGTIN::InitTIN()									// построение начальной тр-ции (10 тр-ков) по вершинам границы (итеративные алгоритмы)
{
	CGTrian *tri;
	if (_Border == NULL || _Border->Size() != 12) return WRONG_REGION;

	// вершины 10 тр-ков
	_Trians.clear();
	tri = new CGTrian((*_Border)[0], (*_Border)[1], (*_Border)[11]);	// 0
	_Trians.push_back(tri);
	tri = new CGTrian((*_Border)[1], (*_Border)[4], (*_Border)[11]);	// 1
	_Trians.push_back(tri);
	tri = new CGTrian((*_Border)[1], (*_Border)[2], (*_Border)[4]);		// 2
	_Trians.push_back(tri);
	tri = new CGTrian((*_Border)[2], (*_Border)[3], (*_Border)[4]);		// 3
	_Trians.push_back(tri);
	tri = new CGTrian((*_Border)[4], (*_Border)[5], (*_Border)[11]);	// 4
	_Trians.push_back(tri);
	tri = new CGTrian((*_Border)[5], (*_Border)[10], (*_Border)[11]);	// 5
	_Trians.push_back(tri);
	tri = new CGTrian((*_Border)[5], (*_Border)[6], (*_Border)[7]);		// 6
	_Trians.push_back(tri);
	tri = new CGTrian((*_Border)[5], (*_Border)[7], (*_Border)[10]);	// 7
	_Trians.push_back(tri);
	tri = new CGTrian((*_Border)[7], (*_Border)[8], (*_Border)[10]);	// 8
	_Trians.push_back(tri);
	tri = new CGTrian((*_Border)[8], (*_Border)[9], (*_Border)[10]);	// 9
	_Trians.push_back(tri);

	// ссылки на соседние тр-ки
	_Trians[0]->SetNeighbors(NULL, _Trians[1], NULL);
	_Trians[1]->SetNeighbors(_Trians[2], _Trians[4], _Trians[0]);
	_Trians[2]->SetNeighbors(NULL, _Trians[3], _Trians[1]);
	_Trians[3]->SetNeighbors(NULL, NULL, _Trians[2]);
	_Trians[4]->SetNeighbors(NULL, _Trians[5], _Trians[1]);
	_Trians[5]->SetNeighbors(_Trians[7], NULL, _Trians[4]);
	_Trians[6]->SetNeighbors(NULL, NULL, _Trians[7]);
	_Trians[7]->SetNeighbors(_Trians[6], _Trians[8], _Trians[5]);
	_Trians[8]->SetNeighbors(NULL, _Trians[9], _Trians[7]);
	_Trians[9]->SetNeighbors(NULL, NULL, _Trians[8]);
	return 0;
}

void CGTIN::AddPointToTrian(CGPoint *pnt, CGTrian *tri)						// включение точки в тр-к (разбиение на 3 новых - итеративный алгоритм)
{
	CGTrian *tr1, *tr2;
	tr1 = new CGTrian(tri->B, tri->C, pnt);
	tr2 = new CGTrian(tri->C, tri->A, pnt);
	tri->C = pnt;
	tr1->AB = tri->BC; tr1->BC = tr2; tr1->CA = tri;
	tr2->AB = tri->CA; tr2->BC = tri; tr2->CA = tr1;
	tri->BC = tr1; tri->CA = tr2;
	_Trians.push_back(tr1);
	_Trians.push_back(tr2);
	ChangeNeighbor(tr1->AB, tri, tr1);
	ChangeNeighbor(tr2->AB, tri, tr2);
}

void CGTIN::ChangeNeighbor(CGTrian *tri, CGTrian *told, CGTrian *tnew)		// замена ссылки на соседний с tri тр-к
{
	if (tri == NULL) return;
	if (tri->AB == told) tri->AB = tnew;
	else if (tri->BC == told) tri->BC = tnew;
	else tri->CA = tnew;
}

void CGTIN::DoFlip(CGTrian *tri)											// флип тр-ка tri и его соседа по AB
{
	CGPoint *a, *b, *c, *d;
	CGTrian *tab, *tbc, *tca, *tad, *tdb;

	a = tri->A; b = tri->B; c = tri->C;
	tab = tri->AB; tbc = tri->BC; tca = tri->CA;

	// d - противолежаща€ вершина tab, tad и tdb - соседи tab
	if (tab->AB == tri) 
	{ 
		d = tab->C; tad = tab->BC; tdb = tab->CA;
	}
	else if (tab->BC == tri) 
	{ 
		d = tab->A; tad = tab->CA; tdb = tab->AB;
	}
	else 
	{ 
		d = tab->B; tad = tab->AB; tdb = tab->BC;
	}

	// замена вершин и соседей tri и tab
	tri->B = d; 
	tri->AB = tad; tri->BC = tab;
	tab->A = d; tab->B = b; tab->C = c;
	tab->AB = tdb; tab->BC = tbc; tab->CA = tri;

	// замена ссылок соседних тр-ков
	ChangeNeighbor(tad, tab, tri);
	ChangeNeighbor(tbc, tri, tab);
}

double CGTIN::Cosin(CGPoint *A, CGPoint *B, CGPoint *C)						// косинус угла ABC
{
	CGVector2D ba(B, A), bc(B, C);
	double lenBA = ba.Length(), lenBC = bc.Length();
	if (lenBA <= _Tolerance || lenBC <= _Tolerance) return -1;
	return (ba * bc) / lenBA / lenBC;
}

//*******************************************************

// ************* регион - область интереса **************
void CGRegion::Clear()														// очистка всех наборов точек и триангул€ций
{
	for (int i = 0; i < _TINS.size(); i++)
		if (_TINS[i]) delete _TINS[i];
	_TINS.clear();

	for (int i = 0; i < _PointSets.size(); i++)
		if (_PointSets[i]) delete _PointSets[i];
	_PointSets.clear();
}

CGPointSet *CGRegion::CreateRandomPoints(int pnum)				// генераци€ набора pnum случайных точек [-10000, 10000] по X и Y (Z = 0)
{
	srand(time(0));
	CGPointSet *pset;
	pset = new CGPointSet(pnum);
	if (!pset) return NULL;
	for (int i = 0; i < pnum; i++)
		pset->Add(new CGPoint(rand() % 20000-10000, rand() % 20000-10000, 0));
	return pset;
}

int CGRegion::Calculate()										// расчет региона (квадрат на XOY) с учетом сдвига границы тр-ции
{
	CGPointSet *pset;
	CGPoint *pnt;
	double xmi, xma, ymi, yma, db;
	int i, n;

	if (_PointSets.size() < 1) return NO_DATA;
	pset = _PointSets[0];
	n = pset->Size();
	if (n < 3) return NO_DATA;

	// границы оболочки точек
	xmi = ymi = 1e20; xma = yma = -xmi;
	for (i = 0; i < n; i++)
	{
		pnt = (*pset)[i];
		if (pnt->X < xmi) xmi = pnt->X;
		if (pnt->X > xma) xma = pnt->X;
		if (pnt->Y < ymi) ymi = pnt->Y;
		if (pnt->Y > yma) yma = pnt->Y;
	}
	if (xma - xmi <= REL_TOLERANCE && yma - ymi <= REL_TOLERANCE) return NO_DATA;

	// получаем квадратную область
	if (xma - xmi >= yma - ymi)
	{
		db = (xma - xmi - yma + ymi) / 2;
		ymi -= db; yma += db;
	}
	else
	{
		db = (yma - ymi - xma + xmi) / 2;
		xmi -= db; xma += db;
	}

	// сдвигаем условную границу
	db = (xma - xmi) * BORDER_SHIFT_PERC / 100;
	_RegionBox.Xmin = xmi - db;
	_RegionBox.Xmax = xma + db;
	_RegionBox.Ymin = ymi - db;
	_RegionBox.Ymax = yma + db;

	return 0;
}
//*******************************************************

