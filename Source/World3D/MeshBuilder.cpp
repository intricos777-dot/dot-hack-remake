#include "MeshBuilder.hpp"
#include <cmath>
#include <numbers>
#include <algorithm>

namespace mine {

// Helper: create a mesh with a single vertex
static Mesh makeMesh(const std::string& name) {
    Mesh m;
    m.name = name;
    return m;
}

// Helper: add a quad (two triangles) to a mesh
void addQuad(Mesh& m, int i0, int i1, int i2, int i3) {
    m.addTriangle(i0, i1, i2);
    m.addTriangle(i0, i2, i3);
}

// Helper: add a triangle to a mesh with computed normal
void addTriNorm(Mesh& m, const Vec3& a, const Vec3& b, const Vec3& c) {
    Vec3 n = Vec3::cross(b - a, c - a).normalized();
    int base = m.getVertexCount();
    m.addVertex(Vertex(a, n));
    m.addVertex(Vertex(b, n));
    m.addVertex(Vertex(c, n));
    m.addTriangle(base, base+1, base+2);
}

Mesh MeshGenerator::createPlane(float width, float depth, int subdivisions) {
    Mesh m = makeMesh("plane");
    float halfW = width * 0.5f;
    float halfD = depth * 0.5f;
    
    for (int z = 0; z <= subdivisions; z++) {
        for (int x = 0; x <= subdivisions; x++) {
            float fx = (float)x / subdivisions;
            float fz = (float)z / subdivisions;
            Vec3 pos(-halfW + fx * width, 0, -halfD + fz * depth);
            Vec2 uv(fx, fz);
            m.addVertex(Vertex(pos, {0, 1, 0}, uv));
        }
    }
    
    for (int z = 0; z < subdivisions; z++) {
        for (int x = 0; x < subdivisions; x++) {
            int i0 = z * (subdivisions + 1) + x;
            int i1 = i0 + 1;
            int i2 = i0 + (subdivisions + 1);
            int i3 = i2 + 1;
            addQuad(m, i0, i1, i3, i2);
        }
    }
    
    return m;
}

Mesh MeshGenerator::createBox(float width, float height, float depth) {
    Mesh m = makeMesh("box");
    float hw = width * 0.5f, hh = height * 0.5f, dd = depth * 0.5f;
    
    // 6 faces, each with 4 vertices and 2 triangles
    Vec3 corners[6][4] = {
        // Front
        {{-hw, -hh, dd}, {hw, -hh, dd}, {hw, hh, dd}, {-hw, hh, dd}},
        // Back
        {{hw, -hh, -dd}, {-hw, -hh, -dd}, {-hw, hh, -dd}, {hw, hh, -dd}},
        // Top
        {{-hw, hh, dd}, {hw, hh, dd}, {hw, hh, -dd}, {-hw, hh, -dd}},
        // Bottom
        {{-hw, -hh, -dd}, {hw, -hh, -dd}, {hw, -hh, dd}, {-hw, -hh, dd}},
        // Right
        {{hw, -hh, dd}, {hw, -hh, -dd}, {hw, hh, -dd}, {hw, hh, dd}},
        // Left
        {{-hw, -hh, -dd}, {-hw, -hh, dd}, {-hw, hh, dd}, {-hw, hh, -dd}}
    };
    
    Vec3 normals[6] = {
        {0, 0, 1}, {0, 0, -1}, {0, 1, 0}, {0, -1, 0}, {1, 0, 0}, {-1, 0, 0}
    };
    
    for (int f = 0; f < 6; f++) {
        int base = m.getVertexCount();
        for (int i = 0; i < 4; i++) {
            Vec2 uv((i == 1 || i == 2) ? 1.0f : 0.0f, (i >= 2) ? 1.0f : 0.0f);
            m.addVertex(Vertex(corners[f][i], normals[f], uv));
        }
        addQuad(m, base, base+1, base+2, base+3);
    }
    
    return m;
}

Mesh MeshGenerator::createSphere(float radius, int segments, int rings) {
    Mesh m = makeMesh("sphere");
    
    for (int r = 0; r <= rings; r++) {
        float phi = (float)r / rings * (float)std::numbers::pi;
        for (int s = 0; s <= segments; s++) {
            float theta = (float)s / segments * 2.0f * (float)std::numbers::pi;
            float x = std::sin(phi) * std::cos(theta);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(theta);
            Vec3 pos(x * radius, y * radius, z * radius);
            Vec3 n(x, y, z);
            Vec2 uv((float)s / segments, (float)r / rings);
            m.addVertex(Vertex(pos, n, uv));
        }
    }
    
    for (int r = 0; r < rings; r++) {
        for (int s = 0; s < segments; s++) {
            int i0 = r * (segments + 1) + s;
            int i1 = i0 + 1;
            int i2 = i0 + (segments + 1);
            int i3 = i2 + 1;
            addQuad(m, i0, i1, i3, i2);
        }
    }
    
    return m;
}

Mesh MeshGenerator::createCylinder(float radius, float height, int segments) {
    Mesh m = makeMesh("cylinder");
    float halfH = height * 0.5f;
    
    // Side vertices
    for (int s = 0; s <= segments; s++) {
        float theta = (float)s / segments * 2.0f * (float)std::numbers::pi;
        float x = std::cos(theta) * radius;
        float z = std::sin(theta) * radius;
        Vec3 n(std::cos(theta), 0, std::sin(theta));
        Vec2 uv((float)s / segments, 0);
        m.addVertex(Vertex({x, -halfH, z}, n, uv));
        uv.y = 1;
        m.addVertex(Vertex({x, halfH, z}, n, uv));
    }
    
    // Side triangles
    for (int s = 0; s < segments; s++) {
        int i0 = s * 2;
        int i1 = i0 + 1;
        int i2 = i0 + 2;
        int i3 = i0 + 3;
        addQuad(m, i0, i2, i3, i1);
    }
    
    // Top cap
    int topStart = m.getVertexCount();
    m.addVertex(Vertex({0, halfH, 0}, {0, 1, 0}, {0.5f, 0.5f}));
    for (int s = 0; s <= segments; s++) {
        float theta = (float)s / segments * 2.0f * (float)std::numbers::pi;
        float x = std::cos(theta) * radius;
        float z = std::sin(theta) * radius;
        m.addVertex(Vertex({x, halfH, z}, {0, 1, 0}, {x / radius * 0.5f + 0.5f, z / radius * 0.5f + 0.5f}));
    }
    for (int s = 0; s < segments; s++) {
        m.addTriangle(topStart, topStart + s + 1, topStart + s + 2);
    }
    
    // Bottom cap
    int botStart = m.getVertexCount();
    m.addVertex(Vertex({0, -halfH, 0}, {0, -1, 0}, {0.5f, 0.5f}));
    for (int s = 0; s <= segments; s++) {
        float theta = (float)s / segments * 2.0f * (float)std::numbers::pi;
        float x = std::cos(theta) * radius;
        float z = std::sin(theta) * radius;
        m.addVertex(Vertex({x, -halfH, z}, {0, -1, 0}, {x / radius * 0.5f + 0.5f, z / radius * 0.5f + 0.5f}));
    }
    for (int s = 0; s < segments; s++) {
        m.addTriangle(botStart, botStart + s + 2, botStart + s + 1);
    }
    
    return m;
}

Mesh MeshGenerator::createCone(float radius, float height, int segments) {
    Mesh m = makeMesh("cone");
    float halfH = height * 0.5f;
    
    // Side
    for (int s = 0; s <= segments; s++) {
        float theta = (float)s / segments * 2.0f * (float)std::numbers::pi;
        float x = std::cos(theta) * radius;
        float z = std::sin(theta) * radius;
        Vec3 n(std::cos(theta), radius / height, std::sin(theta));
        n.normalize();
        m.addVertex(Vertex({x, -halfH, z}, n, {(float)s / segments, 0}));
    }
    
    // Apex
    int apex = m.getVertexCount();
    m.addVertex(Vertex({0, halfH, 0}, {0, 1, 0}, {0.5f, 1}));
    
    // Base center
    int baseCenter = m.getVertexCount();
    m.addVertex(Vertex({0, -halfH, 0}, {0, -1, 0}, {0.5f, 0.5f}));
    
    // Base rim
    int rimStart = m.getVertexCount();
    for (int s = 0; s <= segments; s++) {
        float theta = (float)s / segments * 2.0f * (float)std::numbers::pi;
        float x = std::cos(theta) * radius;
        float z = std::sin(theta) * radius;
        m.addVertex(Vertex({x, -halfH, z}, {0, -1, 0}, {x / radius * 0.5f + 0.5f, z / radius * 0.5f + 0.5f}));
    }
    
    for (int s = 0; s < segments; s++) {
        m.addTriangle(s, apex, s + 1);  // side
        m.addTriangle(baseCenter, rimStart + s + 1, rimStart + s);  // base
    }
    
    return m;
}

Mesh MeshGenerator::createTorus(float majorRadius, float minorRadius, int majorSegments, int minorSegments) {
    Mesh m = makeMesh("torus");
    
    for (int i = 0; i <= majorSegments; i++) {
        float theta = (float)i / majorSegments * 2.0f * (float)std::numbers::pi;
        for (int j = 0; j <= minorSegments; j++) {
            float phi = (float)j / minorSegments * 2.0f * (float)std::numbers::pi;
            float x = (majorRadius + minorRadius * std::cos(phi)) * std::cos(theta);
            float y = minorRadius * std::sin(phi);
            float z = (majorRadius + minorRadius * std::cos(phi)) * std::sin(theta);
            float nx = std::cos(phi) * std::cos(theta);
            float ny = std::sin(phi);
            float nz = std::cos(phi) * std::sin(theta);
            m.addVertex(Vertex({x, y, z}, {nx, ny, nz}, {(float)i / majorSegments, (float)j / minorSegments}));
        }
    }
    
    for (int i = 0; i < majorSegments; i++) {
        for (int j = 0; j < minorSegments; j++) {
            int i0 = i * (minorSegments + 1) + j;
            int i1 = i0 + 1;
            int i2 = i0 + (minorSegments + 1);
            int i3 = i2 + 1;
            addQuad(m, i0, i2, i3, i1);
        }
    }
    
    return m;
}

Mesh MeshGenerator::createPyramid(float base, float height) {
    Mesh m = makeMesh("pyramid");
    float half = base * 0.5f;
    float halfH = height * 0.5f;
    
    // Base vertices (y-down)
    int b0 = m.getVertexCount();
    m.addVertex(Vertex({-half, -halfH, -half}, {0, -1, 0}));
    m.addVertex(Vertex({half, -halfH, -half}, {0, -1, 0}));
    m.addVertex(Vertex({half, -halfH, half}, {0, -1, 0}));
    m.addVertex(Vertex({-half, -halfH, half}, {0, -1, 0}));
    int b1 = b0 + 1, b2 = b0 + 2, b3 = b0 + 3;
    
    // Apex
    int apex = m.getVertexCount();
    m.addVertex(Vertex({0, halfH, 0}, {0, 1, 0}));
    
    // Front face
    addTriNorm(m, {-half, -halfH, half}, {half, -halfH, half}, {0, halfH, 0});
    // Right face
    addTriNorm(m, {half, -halfH, half}, {half, -halfH, -half}, {0, halfH, 0});
    // Back face
    addTriNorm(m, {half, -halfH, -half}, {-half, -halfH, -half}, {0, halfH, 0});
    // Left face
    addTriNorm(m, {-half, -halfH, -half}, {-half, -halfH, half}, {0, halfH, 0});
    // Base
    m.addTriangle(b0, b2, b1);
    m.addTriangle(b0, b3, b2);
    
    return m;
}

Mesh MeshGenerator::createTower(float baseRadius, float topRadius, float height, int segments) {
    Mesh m = makeMesh("tower");
    float halfH = height * 0.5f;
    
    // Side
    for (int s = 0; s <= segments; s++) {
        float theta = (float)s / segments * 2.0f * (float)std::numbers::pi;
        float x = std::cos(theta);
        float z = std::sin(theta);
        Vec3 n(x, 0, z);
        m.addVertex(Vertex({x * baseRadius, -halfH, z * baseRadius}, n, {(float)s / segments, 0}));
        m.addVertex(Vertex({x * topRadius, halfH, z * topRadius}, n, {(float)s / segments, 1}));
    }
    
    for (int s = 0; s < segments; s++) {
        int i0 = s * 2;
        int i1 = i0 + 1;
        int i2 = i0 + 2;
        int i3 = i0 + 3;
        addQuad(m, i0, i2, i3, i1);
    }
    
    return m;
}

Mesh MeshGenerator::createWall(float width, float height, float thickness) {
    Mesh m = makeMesh("wall");
    float hw = width * 0.5f, hh = height * 0.5f, ht = thickness * 0.5f;
    
    // Front face
    Vec3 corners[4] = {{-hw, -hh, ht}, {hw, -hh, ht}, {hw, hh, ht}, {-hw, hh, ht}};
    int base = m.getVertexCount();
    for (int i = 0; i < 4; i++) {
        Vec2 uv((i == 1 || i == 2) ? 1.0f : 0.0f, (i >= 2) ? 1.0f : 0.0f);
        m.addVertex(Vertex(corners[i], {0, 0, 1}, uv));
    }
    addQuad(m, base, base+1, base+2, base+3);
    
    return m;
}

Mesh MeshGenerator::createPillar(float radius, float height, int segments) {
    return createCylinder(radius, height, segments);
}

Mesh MeshGenerator::createStaircase(float width, float height, int steps) {
    Mesh m = makeMesh("staircase");
    float stepH = height / steps;
    float stepD = 2.0f / steps;
    float hw = width * 0.5f;
    
    for (int i = 0; i < steps; i++) {
        float y = i * stepH;
        float z = -1.0f + i * stepD;
        int base = m.getVertexCount();
        // Step top
        m.addVertex(Vertex({-hw, y, z}, {0, 1, 0}));
        m.addVertex(Vertex({hw, y, z}, {0, 1, 0}));
        m.addVertex(Vertex({hw, y, z + stepD}, {0, 1, 0}));
        m.addVertex(Vertex({-hw, y, z + stepD}, {0, 1, 0}));
        // Step front
        m.addVertex(Vertex({hw, y, z + stepD}, {0, 0, 1}));
        m.addVertex(Vertex({-hw, y, z + stepD}, {0, 0, 1}));
        m.addVertex(Vertex({-hw, y + stepH, z + stepD}, {0, 0, 1}));
        m.addVertex(Vertex({hw, y + stepH, z + stepD}, {0, 0, 1}));
        addQuad(m, base, base+1, base+2, base+3);
        addQuad(m, base+4, base+5, base+6, base+7);
    }
    
    return m;
}

Mesh MeshGenerator::createTerrain(float size, int resolution, float height, unsigned int seed) {
    Mesh m = makeMesh("terrain");
    float half = size * 0.5f;
    
    // Simple noise
    auto noise = [&](float x, float z) -> float {
        return std::sin(x * 0.1f + seed) * std::cos(z * 0.1f + seed * 0.7f) * height;
    };
    
    for (int z = 0; z <= resolution; z++) {
        for (int x = 0; x <= resolution; x++) {
            float fx = (float)x / resolution;
            float fz = (float)z / resolution;
            float px = -half + fx * size;
            float pz = -half + fz * size;
            float py = noise(px, pz);
            m.addVertex(Vertex({px, py, pz}, {0, 1, 0}, {fx, fz}));
        }
    }
    
    for (int z = 0; z < resolution; z++) {
        for (int x = 0; x < resolution; x++) {
            int i0 = z * (resolution + 1) + x;
            int i1 = i0 + 1;
            int i2 = i0 + (resolution + 1);
            int i3 = i2 + 1;
            addQuad(m, i0, i1, i3, i2);
        }
    }
    
    m.calculateNormals();
    return m;
}

Mesh MeshGenerator::createGrid(float size, int divisions) {
    Mesh m = makeMesh("grid");
    float half = size * 0.5f;
    
    for (int i = 0; i <= divisions; i++) {
        float t = (float)i / divisions * size - half;
        // X-aligned lines
        m.addVertex(Vertex({t, 0, -half}, {0, 1, 0}));
        m.addVertex(Vertex({t, 0, half}, {0, 1, 0}));
        // Z-aligned lines
        m.addVertex(Vertex({-half, 0, t}, {0, 1, 0}));
        m.addVertex(Vertex({half, 0, t}, {0, 1, 0}));
    }
    
    // Grid as line segments (we'll render as thin quads)
    for (int i = 0; i < divisions; i++) {
        int base = i * 4;
        m.addTriangle(base, base + 1, base + 2);
        m.addTriangle(base + 2, base + 3, base);
    }
    
    return m;
}

void Mesh::calculateNormals() {
    // Reset normals
    for (auto& v : vertices) {
        v.normal = {0, 0, 0};
    }
    
    // Accumulate face normals
    for (const auto& t : triangles) {
        if (t.v0 >= (int)vertices.size() || t.v1 >= (int)vertices.size() || t.v2 >= (int)vertices.size()) continue;
        Vec3 a = vertices[t.v1].position - vertices[t.v0].position;
        Vec3 b = vertices[t.v2].position - vertices[t.v0].position;
        Vec3 n = a.cross(b);
        vertices[t.v0].normal = vertices[t.v0].normal + n;
        vertices[t.v1].normal = vertices[t.v1].normal + n;
        vertices[t.v2].normal = vertices[t.v2].normal + n;
    }
    
    // Normalize
    for (auto& v : vertices) {
        v.normal.normalize();
    }
}

void MeshManager::initialize() {
    generateWorldMeshes();
    generateCharacterMeshes();
    generateWeaponMeshes();
}

void MeshManager::addMesh(const std::string& name, const Mesh& mesh) {
    meshIndices_[name] = (int)meshes_.size();
    meshes_.push_back(mesh);
}

const Mesh* MeshManager::getMesh(const std::string& name) const {
    auto it = meshIndices_.find(name);
    if (it != meshIndices_.end()) return &meshes_[it->second];
    return nullptr;
}

void MeshManager::generateWorldMeshes() {
    // Terrain meshes
    addMesh("terrain_R1", MeshGenerator::createTerrain(200, 32, 15, 42));
    addMesh("terrain_R2", MeshGenerator::createTerrain(200, 32, 20, 123));
    addMesh("terrain_Link", MeshGenerator::createTerrain(200, 32, 10, 777));
    
    // Building meshes
    addMesh("tower", MeshGenerator::createTower(8, 5, 40, 8));
    addMesh("wall", MeshGenerator::createWall(10, 6, 1));
    addMesh("pillar", MeshGenerator::createPillar(1.5f, 12, 12));
    addMesh("monolith", MeshGenerator::createTower(3, 1, 50, 6));
    addMesh("dungeon_pillar", MeshGenerator::createPillar(2, 8, 6));
    addMesh("dungeon_arch", MeshGenerator::createTower(4, 3, 8, 8));
}

void MeshManager::generateCharacterMeshes() {
    // Character meshes will be generated in CharacterMeshes.cpp
}

void MeshManager::generateWeaponMeshes() {
    // Weapon meshes will be generated in CharacterMeshes.cpp
}

const Mesh* MeshManager::getTerrain(const std::string& era) const {
    if (era == "R1") return getMesh("terrain_R1");
    if (era == "R2") return getMesh("terrain_R2");
    if (era == "Link") return getMesh("terrain_Link");
    return getMesh("terrain_R1");
}

const Mesh* MeshManager::getBuilding(const std::string& type) const {
    return getMesh(type);
}

const Mesh* MeshManager::getCharacter(const std::string& name) const {
    return getMesh("char_" + name);
}

const Mesh* MeshManager::getWeapon(const std::string& type) const {
    return getMesh("weapon_" + type);
}

MeshManager& MeshManager::instance() {
    static MeshManager inst;
    return inst;
}

} // namespace mine
