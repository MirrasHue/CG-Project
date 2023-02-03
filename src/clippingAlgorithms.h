#include <vector>

#include "vec2f.h"
#include "objects.h"

namespace mirras
{

struct LineSeg
{
    Vec2f p0, p1;
};

struct Vertex
{
    Vec2f pos;
    int id{-1};
    bool isIntersection{};
    bool isEntering{};
};

enum Orientation : uint8_t
{
    Clockwise,
    CounterClockwise,
    Collinear
};

inline Orientation getOrientation(Vec2f p0, Vec2f p1, Vec2f p2)
{
    float determinant = (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);

    if(determinant < 0.f) return Clockwise; // p2 is on the right side of vector p0->p1
    if(determinant > 0.f) return CounterClockwise; // p2 is on the left side of vector p0->p1
    return Collinear; // All 3 points are collinear
}

// Given a segment and a point that are colinear, check if p lies on the segment
inline bool liesOnSegment(LineSeg l, Vec2f p)
{
    if (p.x <= std::max(l.p0.x, l.p1.x) && p.x >= std::min(l.p0.x, l.p1.x) &&
        p.y <= std::max(l.p0.y, l.p1.y) && p.y >= std::min(l.p0.y, l.p1.y))
        return true;
  
    return false;
}

inline bool checkIntersection(LineSeg l0, LineSeg l1)
{
    auto o1 = getOrientation(l0.p0, l0.p1, l1.p0);
    auto o2 = getOrientation(l0.p0, l0.p1, l1.p1);
    auto o3 = getOrientation(l1.p0, l1.p1, l0.p0);
    auto o4 = getOrientation(l1.p0, l1.p1, l0.p1);

    if (o1 != o2 && o3 != o4)
        return true;

    if(o1 == Collinear && liesOnSegment(l0, l1.p0)) return true;

    if(o2 == Collinear && liesOnSegment(l0, l1.p1)) return true;

    if(o3 == Collinear && liesOnSegment(l1, l0.p0)) return true;

    if(o4 == Collinear && liesOnSegment(l1, l0.p1)) return true;

    return false;
}

inline std::optional<Vec2f> getIntersectionPos(LineSeg l0, LineSeg l1)
{
    if(!checkIntersection(l0, l1))
        return{};
    
    float a1 = l0.p1.y - l0.p0.y;
    float b1 = l0.p0.x - l0.p1.x;
    float c1 = a1 * (l0.p0.x) + b1 * (l0.p0.y);
 
    float a2 = l1.p1.y - l1.p0.y;
    float b2 = l1.p0.x - l1.p1.x;
    float c2 = a2 * (l1.p0.x) + b2 * (l1.p0.y);
 
    float determinant = a1 * b2 - a2 * b1;
 
    if (determinant == 0)
        return {};

    float x = (b2 * c1 - b1 * c2) / determinant;
    float y = (a1 * c2 - a2 * c1) / determinant;
    
    return Vec2f{x, y};
}

// Init and return the clipped polygon list, without intersections
inline std::vector<Vertex> getClippedPolyList(const Polygon& poly)
{
    std::vector<Vertex> clippedPoly;
    clippedPoly.reserve(poly.vertices.size() + 4);

    // Use the viewport coordinates in this case, because we are clipping against the Viewport
    for(const auto& v :  poly.vertices)
        clippedPoly.emplace_back(Vertex{v});

    return clippedPoly;
}

// Init and return the clipping polygon list, without intersections
inline std::vector<Vertex> getClippingPolyList(const std::vector<Vec2f>& winPoints)
{
    std::vector<Vertex> clippingPoly;
    clippingPoly.reserve(winPoints.size() + 4);
    
    for(const auto& p : winPoints)
        clippingPoly.emplace_back(Vertex{p});

    return clippingPoly;
}

// Sort the intersections between each clipping poly segment with respect to the segment's starting point
inline void sortIntersectionsForEachSegmentOf(std::vector<Vertex>& clippingPoly)
{
    auto begin = clippingPoly.begin();
    
    for(size_t i = 0; i < clippingPoly.size() - 1; ++i)
    {
        if(clippingPoly[i].isIntersection)
            continue;
        
        for(size_t j = i + 1; j < clippingPoly.size() + 1; ++j)
        {
            size_t idx = j % clippingPoly.size();

            if(clippingPoly[idx].isIntersection)
                continue;

            std::sort((begin + i + 1), (begin + j), [&clippingPoly, i](const Vertex& v1, const Vertex& v2)
            {
                float dist1 = std::pow(v1.pos.x - clippingPoly[i].pos.x, 2) + std::pow(v1.pos.y - clippingPoly[i].pos.y, 2);
                float dist2 = std::pow(v2.pos.x - clippingPoly[i].pos.x, 2) + std::pow(v2.pos.y - clippingPoly[i].pos.y, 2);

                return dist1 < dist2;
            });
            
            i = j - 1;
            break;
        }
    }
}


// In this step I tried to insert the intersections in the right place as I found them, in the hope of increasing
// the performance, maybe not worth it, but not sure if it would make things simpler by inserting them later.
// There are many edge cases, I couldn't account for them all.
inline void placeIntersectionsInto(std::vector<Vertex>& clippedPoly, std::vector<Vertex>& clippingPoly, const Polygon& poly, const std::vector<Vec2f>& winPoints)
{
    auto it = clippedPoly.begin();
    uint32_t streakNoIntersect{0};
    int vertexId{};

    for(size_t i = 0; i < poly.vertices.size(); ++i)
    {
        bool didIntersect{};
        uint32_t numInterSamePolySeg{};
        bool vertAlreadyAdded{};
        bool addInterOnNextIter{};
        
        for(size_t j = 0; j < winPoints.size(); ++j)
        {
            size_t polyNextVertexIdx = (i + 1) % poly.vertices.size();
            size_t winNextPointIdx = (j + 1) % winPoints.size();
            
            auto intersectPos = getIntersectionPos(LineSeg{poly.vertices[i], poly.vertices[polyNextVertexIdx]},
                                                   LineSeg{winPoints[j], winPoints[winNextPointIdx]});
                                                   
            if(!intersectPos)
                continue;

            bool isEntering{};

            auto orient1 = getOrientation(winPoints[j], winPoints[winNextPointIdx], poly.vertices[i]);
            auto orient2 = getOrientation(winPoints[j], winPoints[winNextPointIdx], poly.vertices[polyNextVertexIdx]);
            
            // Some edge cases

            if(orient1 == Collinear && orient2 == Collinear)
                continue;
                
            if(orient1 == CounterClockwise && orient2 == Collinear)
                isEntering = true;
                
            if(orient1 == Clockwise && orient2 == Collinear)
            {
                size_t thirdVertexIdx = (i + 2) % poly.vertices.size();
                
                auto orient = getOrientation(winPoints[j], winPoints[winNextPointIdx], poly.vertices[thirdVertexIdx]);
                
                if(orient == Clockwise || orient == Collinear)
                    continue;

                if(orient == CounterClockwise)
                    isEntering = false;
            }
                
            if(orient1 == Collinear && orient2 == Clockwise)
                continue;

            if(orient1 == Collinear && orient2 == CounterClockwise)
                isEntering = false;
            
            if(orient1 == CounterClockwise && orient2 == Clockwise && (*intersectPos == winPoints[j] || *intersectPos == winPoints[winNextPointIdx]))
            {
                // Polygon segment is tangent to one corner of the window
                if(*intersectPos == winPoints[0])
                {
                    auto orient = getOrientation(winPoints[winPoints.size() - 1], winPoints[0], poly.vertices[polyNextVertexIdx]);
                    
                    if(orient == CounterClockwise)
                        continue;
                }
                else
                {
                    size_t thirdWinPointIdx = (j + 2) % winPoints.size();
                    auto orient = getOrientation(winPoints[winNextPointIdx], winPoints[thirdWinPointIdx], poly.vertices[polyNextVertexIdx]);
                    
                    if(orient == CounterClockwise)
                        continue;
                }
            }

            if(orient1 == Clockwise && orient2 == CounterClockwise && (*intersectPos == winPoints[j] || *intersectPos == winPoints[winNextPointIdx]))
            {
                // Polygon segment is tangent to one corner of the window
                if(*intersectPos == winPoints[0])
                {
                    auto orient = getOrientation(winPoints[winPoints.size() - 1], winPoints[0], poly.vertices[i]);
                    
                    if(orient == CounterClockwise)
                        continue;
                }
                else
                {
                    size_t thirdWinPointIdx = (j + 2) % winPoints.size();
                    auto orient = getOrientation(winPoints[winNextPointIdx], winPoints[thirdWinPointIdx], poly.vertices[i]);
                    
                    if(orient == CounterClockwise)
                        continue;
                }
            }
                
            ++numInterSamePolySeg;

            if(orient2 == Clockwise)
                isEntering = true;

            auto itTemp = it + streakNoIntersect + 1;
            
            if(itTemp > clippedPoly.end())
                itTemp = clippedPoly.end();

            /*
                Place the intersections within the clipped poly
            */

            bool addIntersection = addInterOnNextIter;
            bool sameVertex{};
            
            // Treat the case when a single polygon segment intersects the window more than once
            if(numInterSamePolySeg == 2)
            {
                Vertex previous = *it;

                float dist1 = std::pow(previous.pos.x - poly.vertices[i].x, 2) + std::pow(previous.pos.y - poly.vertices[i].y, 2);
                float dist2 = std::pow(intersectPos->x - poly.vertices[i].x, 2) + std::pow(intersectPos->y - poly.vertices[i].y, 2);

                if(dist1 > dist2)
                {
                    Vec2f prevPos = previous.pos;
                    int id = previous.id;
                    
                    previous.pos.x = intersectPos->x;
                    previous.pos.y = intersectPos->y;
                    previous.isEntering = true;
                    previous.id = vertexId;

                    *it = previous;
                    
                    if(itTemp != clippedPoly.end())
                        --itTemp;

                    it = clippedPoly.emplace(itTemp, Vertex{prevPos, id, true, false});
                }
                else
                if(dist1 < dist2)
                {
                    if(i != poly.vertices.size() - 1 || itTemp != clippedPoly.end())
                        --itTemp;
                    
                    it = clippedPoly.emplace(itTemp, Vertex{*intersectPos, vertexId, true, isEntering});
                }
                else
                {
                    if(itTemp == clippedPoly.end() && i == poly.vertices.size() - 1 && j == winPoints.size() - 1)
                    {                            
                        auto it1 = clippedPoly.end() - 1;
                        auto it2 = clippedPoly.end() - 2;

                        std::swap(*it1, *it2);
                    }
                    
                    numInterSamePolySeg = 0;
                    addInterOnNextIter = true;
                    sameVertex = true;
                }

                vertAlreadyAdded = true;
            }
                
            if(!vertAlreadyAdded || addIntersection)
            {
                it = clippedPoly.emplace(itTemp, Vertex{*intersectPos, vertexId, true, isEntering});
                addInterOnNextIter = false;
            }
            
            /*
                Place the intersections within the clipping poly
            */

            uint32_t count{0};
            bool addInterLater{};

            for(size_t k = 0; k < clippingPoly.size(); ++k)
            {
                if(numInterSamePolySeg > 2 || sameVertex)
                    break;
                
                if(!clippingPoly[k].isIntersection)
                    count++;
                    
                if(*intersectPos == winPoints[winNextPointIdx] && !addInterLater)
                {
                    if(winNextPointIdx == 0)
                    {
                        clippingPoly.emplace(clippingPoly.begin() + 1, Vertex{*intersectPos, vertexId, true, isEntering});
                        break;
                    }

                    --count;
                    addInterLater = true;
                }
                    
                if(count - 1 == j)
                {
                    clippingPoly.emplace(clippingPoly.begin() + k + 1, Vertex{*intersectPos, vertexId, true, isEntering});
                    break;
                }
            }

            streakNoIntersect = 1;

            didIntersect = true;
            ++vertexId;
        }
        
        if(!didIntersect)
            ++streakNoIntersect;
    }
}

inline auto getSubPolygons(const std::vector<Vertex>& clippedPoly, const std::vector<Vertex>& clippingPoly, size_t initialSize = 2)
{
    std::vector<std::vector<Vertex>> subPolygons;
    subPolygons.reserve(initialSize); 

    bool addVertices{};
    bool allVertsAdded{};
    int enteringVertId{-1};
    int firstEnteringVertId{-2};

    std::vector<Vertex> subPoly;
    subPoly.reserve(4);

    for(size_t i = 0; i < clippedPoly.size() * 2; ++i)
    {
        size_t idx = i % clippedPoly.size();
        size_t nextIdx  = (i + 1) % clippedPoly.size();

        if(firstEnteringVertId == clippedPoly[idx].id || allVertsAdded)
            break;
        
        if(clippedPoly[idx].isIntersection && clippedPoly[idx].isEntering)
        {
            addVertices = true;
            enteringVertId = clippedPoly[idx].id;

            if(firstEnteringVertId == -2)
                firstEnteringVertId = enteringVertId;
        }
        
        if((addVertices && clippedPoly[idx].isIntersection && !clippedPoly[idx].isEntering))
        {
            bool addVerts{};
            int countInterInARow{};
            bool skipVertNextIter{};
            bool anyVertexSkipped{};

            for(size_t j = 0; j < clippingPoly.size() * 2; ++j)
            {
                size_t jdx = j % clippingPoly.size();
              
                if(clippingPoly[jdx].id == clippedPoly[idx].id)
                    addVerts = true;

                if(!addVerts)
                    continue;
                    
                bool skipVertex = skipVertNextIter;
                    
                if(clippingPoly[jdx].isIntersection)
                {
                    ++countInterInARow;
                    
                    if(anyVertexSkipped && j == clippingPoly.size() - 1)
                        skipVertNextIter = true;
                }
                else
                {
                    if(countInterInARow < 2)
                        countInterInARow = 0;
                    else
                        skipVertex = true;
                }

                if(clippingPoly[jdx].id == firstEnteringVertId && j > clippingPoly.size())
                    allVertsAdded = true;
                
                if(clippingPoly[jdx].id == enteringVertId)
                {
                    subPolygons.emplace_back(subPoly);
                    subPoly.clear();
                    addVertices = false;
                    break;
                }
                else
                {
                    if(skipVertex) // Don't add the window corner
                    {
                        if(j != clippingPoly.size() - 1)
                            countInterInARow = 0;
                            
                        anyVertexSkipped = true;
                        skipVertNextIter = false;
                        continue;
                    }

                    subPoly.emplace_back(clippingPoly[jdx]);
                }
            }
        }
        else
        if(addVertices)
            subPoly.emplace_back(clippedPoly[idx]);
    }

    return subPolygons;
}

// This is probably not the best approach to implement Weiler Atherton.
// I didn't find any good implementation online, to compare mine against.
// I used the steps mentioned at 'https://www.geeksforgeeks.org/weiler-atherton-polygon-clipping-algorithm/' as a guiding reference
// The algorithm is not complete, there are edge cases that are not treated
inline auto weilerAtherton(const Polygon& poly, const Window& win)
{
    // We could use a std::array here, as we are only dealing with a rectangular window
    const std::vector<Vec2f> winPoints = {win.wmin, {win.wmin.x, win.wmax.y}, win.wmax, {win.wmax.x, win.wmin.y}};

    auto clippedPoly = getClippedPolyList(poly);
    auto clippingPoly = getClippingPolyList(winPoints);

    placeIntersectionsInto(clippedPoly, clippingPoly, poly, winPoints);

    sortIntersectionsForEachSegmentOf(clippingPoly);

    return getSubPolygons(clippedPoly, clippingPoly, poly.vertices.size() / 2);
}

//////////////////////////////////////////////////////////////////////////////////

enum RegionCode
{
    In     = 0,
    Top    = 0b1000,
    Bottom = 0b0100,
    Right  = 0b0010,
    Left   = 0b0001,
};

void operator|= (RegionCode& code1, RegionCode code2)
{
    int c1 = static_cast<int>(code1);
    int c2 = static_cast<int>(code2);

    c1 |= c2;
    
    code1 = static_cast<RegionCode>(c1);
}

inline RegionCode getCode(Window win, Vec2f p)
{
    RegionCode code = In;
 
    if(p.x < win.wmin.x)
        code |= Left;
    else
    if(p.x > win.wmax.x)
        code |= Right;
        
    if(p.y < win.wmin.y)
        code |= Bottom;
    else
    if(p.y > win.wmax.y)
        code |= Top;
        
    return code;
}

inline float angularCoefficient(LineSeg line)
{
    auto[p0, p1] = line;
    return (p1.y - p0.y) / (p1.x - p0.x);
}

inline std::optional<LineSeg> cohenSutherland(const Window& win, LineSeg line)
{
    auto[p0, p1] = line;
    
    float m = angularCoefficient(line);
    
    RegionCode code1 = getCode(win, p0);
    RegionCode code2 = getCode(win, p1);
 
    for (int i = 0; i < 2; ++i)
    {
        if((code1 == In) && (code2 == In))
            return LineSeg{p0, p1};
        else
        if(code1 & code2)
            return {};
        else
        {
            RegionCode code_out;
            float x{}, y{};
 
            code_out = (code1 != In) ? code1 : code2;
 
            if(code_out & Top)
            {
                x = p0.x + 1 / m * (win.wmax.y - p0.y);
                y = win.wmax.y;
            }
            else if(code_out & Bottom)
            {
                x = p0.x + 1 / m * (win.wmin.y - p0.y);
                y = win.wmin.y;
            }
            else if(code_out & Right)
            {
                y = p0.y + m * (win.wmax.x - p0.x);
                x = win.wmax.x;
            }
            else if(code_out & Left)
            {
                y = p0.y + m * (win.wmin.x - p0.x);
                x = win.wmin.x;
            }
 
            if(code_out == code1)
            {
                p0.x = x;
                p0.y = y;
                code1 = getCode(win, p0);
            }
            else
            {
                p1.x = x;
                p1.y = y;
                code2 = getCode(win, p1);
            }
        }
    }
    
    return LineSeg{p0, p1};
}

////////////////////////////////////////////////////////////////////////////////

inline std::optional<LineSeg> liangBarsky(const Window& win, LineSeg line)
{
    auto[p0, p1] = line;
    
    float dx = p1.x - p0.x;
	float dy = p1.y - p0.y;
	
    float p[4], q[4];
	
	p[0] = -dx;
	p[1] = dx;
	p[2] = -dy;
	p[3] = dy;
	
    q[0] = p0.x - win.wmin.x;
    q[1] = win.wmax.x - p0.x;
    q[2] = p0.y - win.wmin.y;
    q[3] = win.wmax.y - p0.y;
	
	float t[4];
	
	float t1{},
	      t2{1.f};
	
	for(int i = 0; i < 4; ++i)
	{
	    if(p[i] > 0)
	    {
            t[i] = (float) q[i] / p[i];
            t2 = std::min(t2, t[i]);
	    }
        else
        if(p[i] < 0)
        {
            t[i] = (float) q[i] / p[i];
            t1 = std::max(t1, t[i]);
        }
        else
        if(q[i] < 0) // No intersection with the window
            return {};
	}

    if(t1 > t2) // LineSeg completely out
        return {};

    float x1 = p0.x + t1 * dx;
    float y1 = p0.y + t1 * dy;
    
    float x2 = p0.x + t2 * dx;
    float y2 = p0.y + t2 * dy;
    
    return LineSeg{{x1, y1}, {x2, y2}};
}

} // namespace mirras