//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// jump point search grid navigation with navpoint refinement
// (c) 2018 Martin Sedlak
//
//=============================================================================

#include <queue>
#include <vector>
#include <algorithm>
#include <functional>
#include <assert.h>
#include <stddef.h>
#include <math.h>

// note: change this to thread_local if C++11 allowed
#define NAV_THREAD_LOCAL static

// TODO: this could be cleaned up/simplified ...

class Navigation
{
public:
	Navigation();

	void Resize(int width, int height);

	// ncpath = navpoint-compressed path
	// opath = path composed of individual grid elements
	bool NavigateRefined(int sx, int sy, int ex, int ey, std::vector<int> &opath,
		std::vector<int> &ncpath);

	bool Navigate(int sx, int sy, int ex, int ey, std::vector<int> &opath);

	bool TraceLine(int srcx, int srcy, int targx, int targy, std::vector<int> &rpath) const;

	inline void SetMapRow(int y, const unsigned char *row) {map[y] = row;}

	inline static int PackSquare(int x, int y);
	inline static void UnpackSquare(int sq, int &x, int &y);

private:
	// priority queue entry
	struct Entry
	{
		float cost;
		int index;

		inline Entry() {}

		inline Entry(float ncost, int nindex)
			: cost(ncost)
			, index(nindex)
		{
		}

		inline bool operator <(const Entry &b) const
		{
			return cost < b.cost;
		}

		inline bool operator >(const Entry &b) const
		{
			return cost > b.cost;
		}
	};

	int mapWidth;
	int mapHeight;
	std::vector<const unsigned char *> map;

	typedef unsigned char tFrameId;
	typedef char tPrev;

	struct NodeInfo
	{
		// quantized min distance from origin
		unsigned short dist;
		// frame id (counter to detect new search)
		tFrameId frameId;
		// previous node index (packed, relative to current node)
		tPrev prev;

		inline NodeInfo()
			: dist(0)
			, frameId(0)
			, prev(-1)
		{
		}
	};

	std::vector<NodeInfo> mapNodes;
	tFrameId frameId;

	std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry> > pq;
	std::vector<int> path, npath, cpath;

	// temps for routing towards unreachable areas
	int cnode;
	int closest;

	// orthogonal only (this should correspond to what AGS is doing)
	bool nodiag;

	bool navLock;

	void IncFrameId();

	inline static tPrev PackPrev(int px, int py, int x, int y);
	inline int UnpackPrev(int x, int y, tPrev prev) const;

	// stronger inside test
	bool Passable(int x, int y) const;
	// plain access, unchecked
	bool Walkable(int x, int y) const;

	void AddPruned(int *buf, int &bcount, int x, int y) const;
	bool HasForcedNeighbor(int x, int y, int dx, int dy) const;
	int FindJump(int x, int y, int dx, int dy, int ex, int ey);
	int FindOrthoJump(int x, int y, int dx, int dy, int ex, int ey);

	// neighbor reachable (nodiag only)
	bool Reachable(int x0, int y0, int x1, int y1) const;

	static inline int sign(int n)
	{
		return n < 0 ? -1 : (n > 0 ? 1 : 0);
	}

	static inline int iabs(int n)
	{
		return n < 0 ? -n : n;
	}

	static inline int iclamp(int v, int min, int max)
	{
		return v < min ? min : (v > max ? max : v);
	}

	static inline int ClosestDist(int dx, int dy)
	{
		return dx*dx + dy*dy;
		// Manhattan?
		//return iabs(dx) + iabs(dy);
	}
};

// Navigation

Navigation::Navigation()
	: mapWidth(0)
	, mapHeight(0)
	, frameId(1)
	, cnode(0)
	, closest(0)
	// no diagonal route - this should correspond to what AGS does
	, nodiag(true)
	, navLock(false)
{
}

void Navigation::Resize(int width, int height)
{
	mapWidth = width;
	mapHeight = height;

	int size = mapWidth*mapHeight;

	map.resize(mapHeight);
	mapNodes.resize(size);
}

void Navigation::IncFrameId()
{
	if (++frameId == 0)
	{
		for (int i=0; i<(int)mapNodes.size(); i++)
			mapNodes[i].frameId = 0;

		frameId = 1;
	}
}

inline Navigation::tPrev Navigation::PackPrev(int px, int py, int x, int y)
{
	int dx = px - x;
	int dy = py - y;

	dx = iclamp(dx, -1, 1);
	dy = iclamp(dy, -1, 1);

	return (dy+1)*4 + (dx+1);
}

inline int Navigation::UnpackPrev(int x, int y, tPrev prev) const
{
	if (prev < 0)
		return -1;

	int dx = (prev & 3) - 1;
	int dy = (prev >> 2) - 1;

	// we need to scan back to get previous node
	// we trade efficiency (O(1) lookup) for 4x memory savings

	// scan in reverse direction because we used jumps
	for (;;)
	{
		x += dx;
		y += dy;

		int sq = y*mapWidth + x;

		const NodeInfo &node = mapNodes[sq];

		if (node.frameId == frameId)
			return PackSquare(x, y);
	}
}

inline int Navigation::PackSquare(int x, int y)
{
	return (y << 15) + x;
}

inline void Navigation::UnpackSquare(int sq, int &x, int &y)
{
	y = sq >> 15;
	x = sq & ((1 << 15)-1);
}

bool Navigation::Walkable(int x, int y) const
{
	// invert condition because of AGS
	return map[y][x] != 0;
}

bool Navigation::Passable(int x, int y) const
{
	if ((unsigned)x >= (unsigned)mapWidth ||
		(unsigned)y >= (unsigned)mapHeight)
		return false;

	return Walkable(x, y);
}

bool Navigation::Reachable(int x0, int y0, int x1, int y1) const
{
	assert(nodiag);

	return Passable(x1, y1) &&
		(Passable(x1, y0) || Passable(x0, y1));
}

// A* using jump point search (JPS)
// reference: http://users.cecs.anu.edu.au/~dharabor/data/papers/harabor-grastien-aaai11.pdf
void Navigation::AddPruned(int *buf, int &bcount, int x, int y) const
{
	assert(buf && bcount < 8);

	if ((unsigned)x >= (unsigned)mapWidth ||
		(unsigned)y >= (unsigned)mapHeight || !Walkable(x, y))
		return;

	buf[bcount++] = PackSquare(x, y);
}

bool Navigation::HasForcedNeighbor(int x, int y, int dx, int dy) const
{
	if (!dy)
	{
		return  (!Passable(x, y-1) && Passable(x+dx, y-1)) ||
				(!Passable(x, y+1) && Passable(x+dx, y+1));
	}

	if (!dx)
	{
		return  (!Passable(x-1, y) && Passable(x-1, y+dy)) ||
				(!Passable(x+1, y) && Passable(x+1, y+dy));
	}

	return
		(!Passable(x - dx, y) && Passable(x - dx, y + dy)) ||
		(!Passable(x, y - dy) && Passable(x + dx, y - dy));
}

int Navigation::FindOrthoJump(int x, int y, int dx, int dy, int ex, int ey)
{
	assert((!dx || !dy) && (dx || dy));

	for (;;)
	{
		x += dx;
		y += dy;

		if ((unsigned)x >= (unsigned)mapWidth ||
			(unsigned)y >= (unsigned)mapHeight || !Walkable(x, y))
			break;

		int edx = x - ex;
		int edy = y - ey;
		int edist = ClosestDist(edx, edy);

		if (edist < closest)
		{
			closest = edist;
			cnode = PackSquare(x, y);
		}

		if ((x == ex && y == ey) || HasForcedNeighbor(x, y, dx, dy))
			return PackSquare(x, y);
	}

	return -1;
}

int Navigation::FindJump(int x, int y, int dx, int dy, int ex, int ey)
{
	if (!(dx && dy))
		return FindOrthoJump(x, y, dx, dy, ex, ey);

	if (nodiag && !Reachable(x, y, x+dx, y+dy))
		return -1;

	x += dx;
	y += dy;

	if ((unsigned)x >= (unsigned)mapWidth ||
		(unsigned)y >= (unsigned)mapHeight || !Walkable(x, y))
		return -1;

	int edx = x - ex;
	int edy = y - ey;
	int edist = ClosestDist(edx, edy);

	if (edist < closest)
	{
		closest = edist;
		cnode = PackSquare(x, y);
	}

	if ((x == ex && y == ey) || HasForcedNeighbor(x, y, dx, dy))
		return PackSquare(x, y);

	if (dx && dy)
	{
		if (FindOrthoJump(x, y, dx, 0, ex, ey) ||
			FindOrthoJump(x, y, 0, dy, ex, ey))
			return PackSquare(x, y);
	}

	return nodiag ? -1 : FindJump(x, y, dx, dy, ex, ey);
}

bool Navigation::Navigate(int sx, int sy, int ex, int ey, std::vector<int> &opath)
{
	IncFrameId();

	if (!Walkable(sx, sy))
	{
		opath.clear();
		return false;
	}

	// try ray first, if reachable, no need for A* at all
	if (!TraceLine(sx, sy, ex, ey, opath))
		return true;

	NodeInfo &ni = mapNodes[sy*mapWidth+sx];
	ni.dist = 0;
	ni.frameId = frameId;
	ni.prev = -1;

	closest = 0x7fffffff;
	cnode = PackSquare(sx, sy);

	// no clear for priority queue, like, really?!
	while (!pq.empty())
			pq.pop();

	pq.push(Entry(0.0, cnode));

	while (!pq.empty())
	{
		Entry e = pq.top();
		pq.pop();

		int x, y;
		UnpackSquare(e.index, x, y);

		int dx = x - ex;
		int dy = y - ey;
		int edist = ClosestDist(dx, dy);

		if (edist < closest)
		{
			closest = edist;
			cnode = e.index;
		}

		if (x == ex && y == ey)
		{
			// done
			break;
		}

		const NodeInfo &node = mapNodes[y*mapWidth+x];

		float dist = (float)node.dist;

		int pneig[8];
		int ncount = 0;

		int prev = UnpackPrev(x, y, node.prev);

		if (prev < 0)
		{
			for (int ny = y-1; ny <= y+1; ny++)
			{
				if ((unsigned)ny >= (unsigned)mapHeight)
					continue;

				for (int nx = x-1; nx <= x+1; nx++)
				{
					if (nx == x && ny == y)
						continue;

					if ((unsigned)nx >= (unsigned)mapWidth)
						continue;

					if (!Walkable(nx, ny))
						continue;

					if (nodiag && !Reachable(x, y, nx, ny))
						continue;

					pneig[ncount++] = PackSquare(nx, ny);
				}
			}
		}
		else
		{
			// filter
			int px, py;
			UnpackSquare(prev, px, py);
			int dx = sign(x - px);
			int dy = sign(y - py);
			assert(dx || dy);

			if (!dy)
			{
				AddPruned(pneig, ncount, x+dx, y);

				// add corners
				if (!nodiag || Passable(x+dx, y))
				{
					if (!Passable(x, y+1))
						AddPruned(pneig, ncount, x+dx, y+1);

					if (!Passable(x, y-1))
						AddPruned(pneig, ncount, x+dx, y-1);
				}
			}
			else if (!dx)
			{
				// same as above but transposed
				AddPruned(pneig, ncount, x, y+dy);

				// add corners
				if (!nodiag || Passable(x, y+dy))
				{
					if (!Passable(x+1, y))
						AddPruned(pneig, ncount, x+1, y+dy);

					if (!Passable(x-1, y))
						AddPruned(pneig, ncount, x-1, y+dy);
				}
			}
			else
			{
				// diagonal case
				AddPruned(pneig, ncount, x, y+dy);
				AddPruned(pneig, ncount, x+dx, y);

				if (!nodiag || Reachable(x, y, x+dx, y+dy))
					AddPruned(pneig, ncount, x+dx, y+dy);

				if (!Passable(x - dx, y) &&
					(nodiag || Reachable(x, y, x-dx, y+dy)))
					AddPruned(pneig, ncount, x-dx, y+dy);

				if (!Passable(x, y-dy) &&
					(nodiag || Reachable(x, y, x+dx, y-dy)))
					AddPruned(pneig, ncount, x+dx, y-dy);
			}
		}

		// sort by heuristics
		Entry sort[8];

		for (int ni = 0; ni < ncount; ni++)
		{
			int nx, ny;
			UnpackSquare(pneig[ni], nx, ny);
			float edx = (float)(nx - ex);
			float edy = (float)(ny - ey);
			sort[ni].cost = sqrt(edx*edx + edy*edy);
			sort[ni].index = pneig[ni];
		}

		std::sort(sort, sort+ncount);

		int succ[8];
		int nsucc = 0;

		for (int ni=0; ni<ncount; ni++)
			pneig[ni] = sort[ni].index;

		for (int ni = 0; ni < ncount; ni ++)
		{
			int nx, ny;
			UnpackSquare(pneig[ni], nx, ny);

			int dx = nx - x;
			int dy = ny - y;
			int j = FindJump(x, y, dx, dy, ex, ey);

			if (j < 0)
				continue;

			succ[nsucc++] = j;
		}

		for (int ni = 0; ni < nsucc; ni ++)
		{
			int nx, ny;
			UnpackSquare(succ[ni], nx, ny);
			assert(Walkable(nx, ny));

			NodeInfo &node = mapNodes[ny*mapWidth+nx];

			float ndist = node.frameId != frameId ? INFINITY : (float)node.dist;

			float dx = (float)(nx - x);
			float dy = (float)(ny - y);
			// FIXME: can do better here
			float cost = sqrt(dx*dx + dy*dy);
			float ecost = dist + cost;

			float edx = (float)(nx - ex);
			float edy = (float)(ny - ey);
			float heur = sqrt(edx*edx + edy*edy);

			if (ecost < ndist)
			{
				// assert because we use 16-bit quantized min distance from start to save memory
				assert(ecost <= 65535.0f);

				if (ecost > 65535.0f)
					continue;

				node.dist = (unsigned short)(ecost + 0.5f);
				node.frameId = frameId;
				node.prev = PackPrev(x, y, nx, ny);
				pq.push(Entry(ecost + heur, PackSquare(nx, ny)));
			}
		}
	}

	opath.clear();

	// now since we allow approx routing even if dst
	// isn't directly reachable
	// note: not sure if this provides optimal results even if we update
	// cnode during jump search
	int nex, ney;
	UnpackSquare(cnode, nex, ney);

	if ((nex != sx || ney != sy) && (nex != ex || ney != ey))
	{
		// target not directly reachable => move closer to target
		TraceLine(nex, ney, ex, ey, opath);
		UnpackSquare(opath.back(), nex, ney);

		bool res = true;

		// note: navLock => better safe than sorry
		// infinite recursion should never happen but... better safe than sorry
		assert(!navLock);

		if (!navLock)
		{
			// and re-route
			opath.clear();

			navLock = true;
			res = Navigate(sx, sy, nex, ney, opath);
			navLock = false;
		}

		// refine this a bit further; find path point closest
		// to original target and truncate

		int best = 0x7fffffff;
		int bestSize = (int)opath.size();

		for (int i=0; i<(int)opath.size(); i++)
		{
			int x, y;
			UnpackSquare(opath[i], x, y);
			int dx = x-ex, dy = y-ey;
			int cost = ClosestDist(dx, dy);

			if (cost < best)
			{
				best = cost;
				bestSize = i+1;
			}
		}

		opath.resize(bestSize);

		return res;
	}

	if (mapNodes[ey*mapWidth+ex].frameId != frameId)
	{
		// path not found
		return false;
	}

	int tx = ex;
	int ty = ey;
	// add end
	opath.push_back(PackSquare(tx, ty));

	for (;;)
	{
		int prev = UnpackPrev(tx, ty, mapNodes[ty*mapWidth+tx].prev);

		if (prev < 0)
			break;

		// unpack because we use JPS
		int px, py;
		UnpackSquare(prev, px, py);
		int dx = sign(px - tx);
		int dy = sign(py - ty);

		while (tx != px || ty != py)
		{
			tx += dx;
			ty += dy;
			opath.push_back(PackSquare(tx, ty));
		}
	}

	std::reverse(opath.begin(), opath.end());
	return true;
}

bool Navigation::NavigateRefined(int sx, int sy, int ex, int ey,
	std::vector<int> &opath, std::vector<int> &ncpath)
{
	ncpath.clear();

	if (!Navigate(sx, sy, ex, ey, opath))
		return false;

	NAV_THREAD_LOCAL std::vector<int> tmp;
	tmp.clear();

	tmp.reserve(opath.size());
	int fx = sx;
	int fy = sy;

	NAV_THREAD_LOCAL std::vector<int> fpath;
	NAV_THREAD_LOCAL std::vector<int> ncpathIndex;
	fpath.clear();
	ncpathIndex.clear();

	fpath.reserve(opath.size());
	fpath.push_back(opath[0]);
	ncpath.push_back(opath[0]);
	ncpathIndex.push_back(0);

	NAV_THREAD_LOCAL std::vector<int> rayPath, orayPath;

	rayPath.clear();
	orayPath.clear();

	rayPath.reserve(opath.size());
	orayPath.reserve(opath.size());

	for (int i=1; i<(int)opath.size(); i++)
	{
		// trying to optimize path
		int tx, ty;
		UnpackSquare(opath[i], tx, ty);

		bool last = i == (int)opath.size()-1;

		if (!TraceLine(fx, fy, tx, ty, rayPath))
		{
			assert(rayPath.back() == opath[i]);
			std::swap(rayPath, orayPath);

			if (!last)
				continue;
		}

		// copy orayPath
		for (int j=1; j<(int)orayPath.size(); j++)
			fpath.push_back(orayPath[j]);

		if (!orayPath.empty())
		{
			assert(ncpath.back() == orayPath[0]);
			ncpath.push_back(orayPath.back());
			ncpathIndex.push_back((int)fpath.size()-1);

			if (!last)
			{
				UnpackSquare(orayPath.back(), fx, fy);
				orayPath.clear();
				i--;
				continue;
			}
		}

		if (fpath.back() != opath[i])
			fpath.push_back(opath[i]);

		if (ncpath.back() != opath[i])
		{
			ncpath.push_back(opath[i]);
			ncpathIndex.push_back((int)fpath.size()-1);
		}

		fx = tx;
		fy = ty;
	}

	std::swap(opath, fpath);

	// validate cpath
	for (int i=0; i<(int)ncpath.size()-1; i++)
	{
		int tx, ty;
		UnpackSquare(ncpath[i], fx, fy);
		UnpackSquare(ncpath[i+1], tx, ty);
		assert(!TraceLine(fx, fy, tx, ty, rayPath));
	}

	assert(ncpath.size() == ncpathIndex.size());

	// so now we have opath, ncpath and ncpathIndex
	// we want to gradually move ncpath node towards previous to see
	// if we can raycast from prev ncpath node to moved and from moved
	// to the end

	bool adjusted = false;

	for (int i=(int)ncpath.size()-2; i>0; i--)
	{
		int px, py;
		int nx, ny;

		int pidx = ncpathIndex[i-1];
		int idx = ncpathIndex[i];

		UnpackSquare(ncpath[i-1], px, py);
		UnpackSquare(ncpath[i+1], nx, ny);

		for (int j=idx-1; j >= pidx; j--)
		{
			int x, y;
			UnpackSquare(opath[j], x, y);

			// if we can raycast px,py => x,y and x,y => nx,ny,
			// we can move ncPath node!
			if (TraceLine(px, py, x, y, rayPath))
				continue;

			if (TraceLine(x, y, nx, ny, rayPath))
				continue;

			ncpath[i] = opath[j];
			ncpathIndex[i] = j;
			adjusted = true;
		}

		if (ncpath[i] == ncpath[i-1])
		{
			// if we get here, we need to remove ncpath[i]
			// because we reached the previous node
			ncpath.erase(ncpath.begin()+i);
			ncpathIndex.erase(ncpathIndex.begin()+i);
			adjusted = true;
		}
	}

	if (!adjusted)
		return true;

	// final step (if necessary) is to reconstruct path from compressed path

	opath.clear();
	opath.push_back(ncpath[0]);

	for (int i=1; i<(int)ncpath.size(); i++)
	{
		int fx, fy;
		int tx, ty;

		UnpackSquare(ncpath[i-1], fx, fy);
		UnpackSquare(ncpath[i], tx, ty);

		TraceLine(fx, fy, tx, ty, rayPath);

		for (int j=1; j<(int)rayPath.size(); j++)
			opath.push_back(rayPath[j]);
	}

	return true;
}

bool Navigation::TraceLine(int srcx, int srcy, int targx, int targy,
	std::vector<int> &rpath) const
{
	rpath.clear();

	// DDA
	int x0 = (srcx << 16) + 0x8000;
	int y0 = (srcy << 16) + 0x8000;
	int x1 = (targx << 16) + 0x8000;
	int y1 = (targy << 16) + 0x8000;

	int dx = x1 - x0;
	int dy = y1 - y0;

	if (!dx && !dy)
	{
		if (!Passable(srcx, srcy))
			return true;

		rpath.push_back(PackSquare(srcx, srcy));
		return false;
	}

	int xinc, yinc;

	if (iabs(dx) >= iabs(dy))
	{
		// step along x
		xinc = sign(dx) * 65536;
		yinc = (int)((double)dy * 65536 / iabs(dx));
	}
	else
	{
		// step along y
		yinc = sign(dy) * 65536;
		xinc = (int)((double)dx * 65536 / iabs(dy));
	}

	int fx = x0;
	int fy = y0;
	int x = x0 >> 16;
	int y = y0 >> 16;
	int ex = x1 >> 16;
	int ey = y1 >> 16;

	while (x != ex || y != ey)
	{
		if (!Passable(x, y))
			return true;

		rpath.push_back(PackSquare(x, y));

		fx += xinc;
		fy += yinc;
		int ox = x;
		int oy = y;
		x = fx >> 16;
		y = fy >> 16;

		if (nodiag && !Reachable(ox, oy, x, y))
			return true;
	}

	assert(iabs(x - ex) <= 1 && iabs(y - ey) <= 1);

	if (nodiag && !Reachable(x, y, ex, ey))
		return false;

	if (!Passable(ex, ey))
		return true;

	int sq = PackSquare(ex, ey);

	if (rpath.empty() || rpath.back() != sq)
		rpath.push_back(sq);

	return false;
}
