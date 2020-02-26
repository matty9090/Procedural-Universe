#include "TerrainNode.hpp"
#include "Services/Log.hpp"

#include "Render/Planet/Planet.hpp"
#include "Render/Planet/Components/TerrainComponent.hpp"

template <class HeightFunc>
CTerrainNode<HeightFunc>::CTerrainNode(CPlanet* planet, CTerrainNode* parent, EQuad quad)
    : Quadtree(quad, parent),
      Planet(planet),
      Buffer(planet->GetDevice()),
      PSBuffer(planet->GetDevice()),
      Diameter(Bounds.size * Planet->Radius)
{
    for (size_t child = 0; child < 4; ++child) {
        ChildNodes[child] = nullptr;
    }

    if (Parent != nullptr)
    {
        Depth = Parent->GetDepth() + 1;
        World = Parent->World;
        Orientation = Parent->Orientation;
    }
}

template <class HeightFunc>
void CTerrainNode<HeightFunc>::Generate()
{
    UINT gridsize = CTerrainComponent<HeightFunc>::GridSize, gh = CTerrainComponent<HeightFunc>::GridSize / 2;

    Indices.clear();
    Vertices.clear();
    Vertices.reserve(gridsize * gridsize);

    float step = Bounds.size / (gridsize - 1);
    int k = 0, sx = 0, sy = 0;

    switch (Quad)
    {
        case NW: sx = 0, sy = 0; break;
        case NE: sx = gh, sy = 0; break;
        case SE: sx = gh, sy = gh; break;
        case SW: sx = 0, sy = gh; break;
    }

    for (UINT y = 0; y < gridsize; ++y)
    {
        float yy = Bounds.y + y * step;

        for (UINT x = 0; x < gridsize; ++x, ++k)
        {
            TerrainVertex vertex;

            if (Parent && (x % 2 == 0) && (y % 2 == 0))
            {
                int xh = sx + x / 2;
                int yh = sy + y / 2;

                vertex = Parent->GetVertex(xh + yh * gridsize);
            }
            else
            {
                float xx = Bounds.x + x * step;

                Vector3 pos = Vector3(xx, yy, 1.0f);
                pos = Vector3::Transform(PointToSphere(pos), Orientation);

                Vector3 normal = pos;
                normal.Normalize();

                float height = GetHeight(normal, Depth);
                Vector3 finalPos = pos * Planet->Radius + normal * height;

                vertex.Position = finalPos;
                vertex.Normal = Vector3::Zero;
                vertex.UV = Vector2(xx, yy);
            }

            if (x == 0)             Edges[West].push_back(k);
            if (x >= gridsize - 1)  Edges[East].push_back(k);
            if (y == 0)             Edges[North].push_back(k);
            if (y >= gridsize - 1)  Edges[South].push_back(k);

            Vertices.push_back(vertex);
        }
    }

    Indices = CTerrainComponent<HeightFunc>::IndexPerm[0];

    for (size_t i = 0; i < Indices.size(); i += 3)
    {
        Vector3 p1 = Vertices[Indices[i + 0]].Position;
        Vector3 p2 = Vertices[Indices[i + 1]].Position;
        Vector3 p3 = Vertices[Indices[i + 2]].Position;

        Vector3 n = (p3 - p1).Cross(p2 - p1);

        Vertices[Indices[i + 0]].Normal += n;
        Vertices[Indices[i + 1]].Normal += n;
        Vertices[Indices[i + 2]].Normal += n;
    }

    D3D11_BUFFER_DESC desc;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = static_cast<UINT>(Vertices.size() * sizeof(TerrainVertex));
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = Vertices.data();

    Planet->GetDevice()->CreateBuffer(&desc, &data, VertexBuffer.ReleaseAndGetAddressOf());

    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.ByteWidth = static_cast<UINT>(Indices.size() * sizeof(UINT));
    data.pSysMem = Indices.data();

    Planet->GetDevice()->CreateBuffer(&desc, &data, IndexBuffer.ReleaseAndGetAddressOf());
}

template <class HeightFunc>
void CTerrainNode<HeightFunc>::NotifyNeighbours()
{
    std::vector<CTerrainNode*> neighbours;

    for (int i = 0; i < 4; ++i)
    {
        auto sn = GetSmallerNeighbours(GetGreaterThanOrEqualNeighbour(i), i);
        neighbours.insert(neighbours.end(), sn.begin(), sn.end());
    }

    for (auto n : neighbours)
        n->FixEdges();
}

template <class HeightFunc>
void CTerrainNode<HeightFunc>::FixEdges()
{
    std::vector<CTerrainNode*> neighbours(4);
    std::vector<bool> depths(4);

    for (int i = 0; i < 4; ++i) neighbours[i] = GetGreaterThanOrEqualNeighbour(i);
    for (int i = 0; i < 4; ++i) depths[i] = neighbours[i] && (Depth - neighbours[i]->GetDepth() >= 1);

    int d0 = depths[North] ? CTerrainComponent<HeightFunc>::Top : 0;
    int d1 = depths[East] ? CTerrainComponent<HeightFunc>::Right : 0;
    int d2 = depths[South] ? CTerrainComponent<HeightFunc>::Bottom : 0;
    int d3 = depths[West] ? CTerrainComponent<HeightFunc>::Left : 0;

    int perm = d0 | d1 | d2 | d3;
    Indices = CTerrainComponent<HeightFunc>::IndexPerm[perm];

    FixEdge(North, neighbours[North], neighbours[North]->GetEdge(South), d0);
    FixEdge(East,  neighbours[East],  neighbours[East]->GetEdge(West),   d1);
    FixEdge(South, neighbours[South], neighbours[South]->GetEdge(North), d2);
    FixEdge(West,  neighbours[West],  neighbours[West]->GetEdge(East),   d3);

    D3D11_BUFFER_DESC desc;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = static_cast<UINT>(sizeof(UINT) * Indices.size());
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = Indices.data();

    Planet->GetDevice()->CreateBuffer(&desc, &data, IndexBuffer.ReleaseAndGetAddressOf());
}

template<class HeightFunc>
void CTerrainNode<HeightFunc>::FixEdge(EDir dir, CTerrainNode* neighbour, std::vector<UINT> edge, int depth)
{
    if (Depth - depth == 0)
    {
        for (UINT i = 0; i < CTerrainComponent<HeightFunc>::GridSize; ++i)
        {
            //Vertices[Edges[dir][i]].Normal = neighbour->GetVertex(edge[i]).Normal;
        }
    }
}

template <class HeightFunc>
Vector3 CTerrainNode<HeightFunc>::PointToSphere(Vector3 p)
{
    float x2 = p.x * p.x, y2 = p.y * p.y, z2 = p.z * p.z;

    return Vector3(p.x * sqrtf(1.0f - y2 * 0.5f - z2 * 0.5f + (y2 * z2) * 0.33333333f),
        p.y * sqrtf(1.0f - z2 * 0.5f - x2 * 0.5f + (z2 * x2) * 0.33333333f),
        p.z * sqrtf(1.0f - x2 * 0.5f - y2 * 0.5f + (x2 * y2) * 0.33333333f));
}

template <class HeightFunc>
void CTerrainNode<HeightFunc>::Update(float dt)
{
    auto cam = Planet->Camera.GetPosition();
    float distance = Vector3::Distance(cam, GetCenterWorld());
    float height = Vector3::Distance(cam, Planet->GetPosition()) - Planet->Radius;
    float horizon = sqrtf(height * (2 * Planet->Radius + height));

    Visible = distance - Diameter / 2.0f < horizon;

    if (Visible)
    {
        Quadtree::Update(dt);
    }
}

template <class HeightFunc>
void CTerrainNode<HeightFunc>::Render(Matrix viewProj)
{
    if (!Visible)
        return;

    if (IsLeaf())
    {
        auto context = Planet->GetContext();
        auto w = World * Planet->World;

        Buffer.SetData(context, { w * viewProj, w });
        PSBuffer.SetData(context, { Planet->LightSource });

        UINT offset = 0;
        UINT stride = sizeof(TerrainVertex);

        context->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &stride, &offset);
        context->IASetIndexBuffer(IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->VSSetConstantBuffers(0, 1, Buffer.GetBuffer());
        context->PSSetConstantBuffers(0, 1, PSBuffer.GetBuffer());
        context->DrawIndexed(static_cast<UINT>(Indices.size()), 0, 0);
    }
    else
    {
        for (int i = 0; i < 4; ++i)
            ChildNodes[i]->Render(viewProj);
    }
}

template <class HeightFunc>
Vector3 CTerrainNode<HeightFunc>::GetCenterWorld()
{
    Vector3 midpoint = Vertices[(CTerrainComponent<HeightFunc>::GridSize * CTerrainComponent<HeightFunc>::GridSize) / 2].Position;
    return Vector3::Transform(midpoint, Planet->World * World);
}

template <class HeightFunc>
void CTerrainNode<HeightFunc>::SplitFunction()
{
    float x = Bounds.x, y = Bounds.y;
    float d = Bounds.size / 2;

    for (int i = 0; i < 4; ++i)
    {
        ChildNodes[i] = new CTerrainNode(Planet, this, static_cast<EQuad>(i));
    }

    ChildNodes[NW]->SetBounds(Square { x    , y    , d });
    ChildNodes[NE]->SetBounds(Square { x + d, y    , d });
    ChildNodes[SE]->SetBounds(Square { x + d, y + d, d });
    ChildNodes[SW]->SetBounds(Square { x    , y + d, d });

    for (int i = 0; i < 4; ++i)
        ChildNodes[i]->Generate();

    for (int i = 0; i < 4; ++i)
        ChildNodes[i]->FixEdges();

    NotifyNeighbours();
}

template <class HeightFunc>
void CTerrainNode<HeightFunc>::MergeFunction()
{
    for (int i = 0; i < 4; ++i)
    {
        ChildNodes[i] = nullptr;
    }

    NotifyNeighbours();
}

template <class HeightFunc>
bool CTerrainNode<HeightFunc>::DistanceFunction()
{
    Vector3 cam = Planet->Camera.GetPosition();
    float distance = Vector3::Distance(cam, GetCenterWorld());

    return Depth < 8 && distance < Bounds.size * Planet->SplitDistance;
}
