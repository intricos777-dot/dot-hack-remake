#include "MeshBuilder.hpp"
#include "Math3D.hpp"
#include <cmath>
#include <numbers>

namespace mine {

// Character mesh creation
static Mesh createHumanoidBody(const std::string& name, const Color& bodyColor, const Color& accentColor) {
    Mesh m;
    m.name = name;
    m.baseColor = bodyColor;
    
    // Torso (capsule-like)
    float torsoW = 0.8f, torsoH = 1.2f, torsoD = 0.4f;
    int segments = 12;
    
    // Create torso as stacked circles
    for (int y = 0; y <= 4; y++) {
        float fy = (float)y / 4;
        float py = torsoH * 0.5f - fy * torsoH;
        float radius = torsoW * (0.8f + 0.2f * std::sin(fy * 3.14159f));
        for (int s = 0; s <= segments; s++) {
            float theta = (float)s / segments * 2.0f * (float)std::numbers::pi;
            float x = std::cos(theta) * radius;
            float z = std::sin(theta) * torsoD * 0.5f;
            Vec3 n(std::cos(theta), 0, std::sin(theta));
            m.addVertex(Vertex({x, py, z}, n, {(float)s / segments, fy}));
        }
    }
    
    // Head
    float headR = 0.35f;
    Vec3 headPos(0, torsoH * 0.5f + headR, 0);
    int headStart = m.getVertexCount();
    for (int y = 0; y <= 3; y++) {
        float fy = (float)y / 3;
        float py = headPos.y + headR * (0.5f - fy);
        float r = headR * std::sqrt(1.0f - (fy - 0.5f) * (fy - 0.5f) * 4.0f);
        for (int s = 0; s <= segments; s++) {
            float theta = (float)s / segments * 2.0f * (float)std::numbers::pi;
            float x = std::cos(theta) * r;
            float z = std::sin(theta) * r;
            Vec3 n(x, (fy - 0.5f) * 2, z);
            n.normalize();
            m.addVertex(Vertex({x, py, z}, n, {(float)s / segments, fy}));
        }
    }
    
    m.calculateNormals();
    return m;
}

// Create a weapon mesh
static Mesh createBlade(float length, float width) {
    Mesh m;
    m.name = "blade";
    
    float halfW = width * 0.5f;
    float halfL = length * 0.5f;
    
    // Blade as a thin diamond
    m.addVertex(Vertex({0, halfL, 0}, {0, 1, 0}));  // tip
    m.addVertex(Vertex({-halfW, 0, 0}, {-1, 0, 0}));
    m.addVertex(Vertex({0, 0, -halfW}, {0, 0, -1}));
    m.addVertex(Vertex({halfW, 0, 0}, {1, 0, 0}));
    m.addVertex(Vertex({0, 0, halfW}, {0, 0, 1}));
    m.addVertex(Vertex({0, -halfL * 0.3f, 0}, {0, -1, 0}));  // base
    
    m.addTriangle(0, 1, 2);
    m.addTriangle(0, 2, 3);
    m.addTriangle(0, 3, 4);
    m.addTriangle(0, 4, 1);
    m.addTriangle(5, 2, 1);
    m.addTriangle(5, 3, 2);
    m.addTriangle(5, 4, 3);
    m.addTriangle(5, 1, 4);
    
    return m;
}

Mesh createKiteMesh() {
    return createHumanoidBody("char_kite", Color(0.2f, 0.4f, 0.8f, 1.0f), Color(0.4f, 0.7f, 1.0f, 1.0f));
}

Mesh createHaseoMesh() {
    return createHumanoidBody("char_haseo", Color(0.1f, 0.05f, 0.05f, 1.0f), Color(0.8f, 0.2f, 0.2f, 1.0f));
}

Mesh createBlackRoseMesh() {
    return createHumanoidBody("char_blackrose", Color(0.4f, 0.15f, 0.5f, 1.0f), Color(0.8f, 0.4f, 0.9f, 1.0f));
}

Mesh createTsukasaMesh() {
    return createHumanoidBody("char_tsukasa", Color(0.3f, 0.3f, 0.35f, 1.0f), Color(0.6f, 0.6f, 0.7f, 1.0f));
}

Mesh createAIDAMesh() {
    return createHumanoidBody("char_aida", Color(0.1f, 0.05f, 0.15f, 1.0f), Color(0.9f, 0.2f, 0.5f, 1.0f));
}

Mesh createShugoMesh() {
    return createHumanoidBody("char_shugo", Color(0.2f, 0.5f, 0.2f, 1.0f), Color(0.4f, 0.8f, 0.4f, 1.0f));
}

Mesh createRoseMesh() {
    return createHumanoidBody("char_rose", Color(0.3f, 0.15f, 0.35f, 1.0f), Color(0.7f, 0.3f, 0.8f, 1.0f));
}

Mesh createShinoMesh() {
    return createHumanoidBody("char_shino", Color(0.2f, 0.1f, 0.3f, 1.0f), Color(0.6f, 0.3f, 0.9f, 1.0f));
}

Mesh createDualSwordMesh() {
    return createBlade(2.0f, 0.15f);
}

Mesh createHeavyBladeMesh() {
    return createBlade(2.5f, 0.4f);
}

Mesh createStaffMesh() {
    Mesh m;
    m.name = "staff";
    m.baseColor = Color(0.6f, 0.4f, 0.2f, 1.0f);
    
    float length = 2.5f;
    float radius = 0.05f;
    int segments = 8;
    
    for (int s = 0; s <= segments; s++) {
        float theta = (float)s / segments * 2.0f * (float)std::numbers::pi;
        float x = std::cos(theta) * radius;
        float z = std::sin(theta) * radius;
        Vec3 n(std::cos(theta), 0, std::sin(theta));
        m.addVertex(Vertex({x, 0, z}, n));
        m.addVertex(Vertex({x, length, z}, n));
    }
    
    for (int s = 0; s < segments; s++) {
        int i0 = s * 2;
        int i1 = i0 + 1;
        int i2 = i0 + 2;
        int i3 = i0 + 3;
        addQuad(m, i0, i2, i3, i1);
    }
    
    // Staff orb
    int orbStart = m.getVertexCount();
    float orbR = 0.2f;
    for (int y = 0; y <= 3; y++) {
        float fy = (float)y / 3;
        float py = length + orbR * (0.5f - fy);
        float r = orbR * std::sqrt(1.0f - (fy - 0.5f) * (fy - 0.5f) * 4.0f);
        for (int s = 0; s <= segments; s++) {
            float theta = (float)s / segments * 2.0f * (float)std::numbers::pi;
            float x = std::cos(theta) * r;
            float z = std::sin(theta) * r;
            Vec3 n(x, (fy - 0.5f) * 2, z);
            n.normalize();
            m.addVertex(Vertex({x, py, z}, n));
        }
    }
    
    return m;
}

Mesh createShieldMesh() {
    Mesh m;
    m.name = "shield";
    m.baseColor = Color(0.4f, 0.3f, 0.5f, 1.0f);
    
    float width = 0.8f, height = 1.2f;
    float hw = width * 0.5f, hh = height * 0.5f;
    
    // Shield as a curved quad
    float curve = 0.1f;
    m.addVertex(Vertex({-hw, -hh, curve}, {0, 0, 1}));
    m.addVertex(Vertex({hw, -hh, curve}, {0, 0, 1}));
    m.addVertex(Vertex({hw, hh, curve * 0.5f}, {0, 0, 1}));
    m.addVertex(Vertex({-hw, hh, curve * 0.5f}, {0, 0, 1}));
    
    m.addTriangle(0, 1, 2);
    m.addTriangle(0, 2, 3);
    
    return m;
}

Mesh createTerrainR1() {
    return MeshGenerator::createTerrain(200, 24, 12, 42);
}

Mesh createTerrainR2() {
    return MeshGenerator::createTerrain(200, 24, 18, 123);
}

Mesh createTerrainLink() {
    return MeshGenerator::createTerrain(200, 24, 8, 777);
}

} // namespace mine
