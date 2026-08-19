#pragma once
#include "Math3D.hpp"
#include <vector>
#include <string>
#include <map>

namespace mine {

// Mesh with vertices and triangles
struct Mesh {
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;
    Color baseColor = {1, 1, 1, 1};
    float specular = 0.5f;
    float shininess = 32.0f;
    
    void addVertex(const Vertex& v) { vertices.push_back(v); }
    void addTriangle(int v0, int v1, int v2) { triangles.push_back({v0, v1, v2}); }
    
    int getVertexCount() const { return (int)vertices.size(); }
    int getTriangleCount() const { return (int)triangles.size(); }
    
    // Calculate normals from geometry
    void calculateNormals();
    
    // Transform all vertices
    void transform(const Mat4& mat);
    
    // Translate
    void translate(const Vec3& t);
    void scale(const Vec3& s);
};

// Procedural mesh generator
class MeshGenerator {
public:
    // Primitive shapes
    static Mesh createPlane(float width, float depth, int subdivisions = 1);
    static Mesh createBox(float width, float height, float depth);
    static Mesh createSphere(float radius, int segments = 16, int rings = 12);
    static Mesh createCylinder(float radius, float height, int segments = 16);
    static Mesh createCone(float radius, float height, int segments = 16);
    static Mesh createTorus(float majorRadius, float minorRadius, int majorSegments = 16, int minorSegments = 8);
    static Mesh createPyramid(float base, float height);
    static Mesh createDiamond(float radius, float height);
    
    // Terrain
    static Mesh createTerrain(float size, int resolution, float height, unsigned int seed = 42);
    static Mesh createGrid(float size, int divisions);
    
    // Structures
    static Mesh createTower(float baseRadius, float topRadius, float height, int segments = 8);
    static Mesh createWall(float width, float height, float thickness);
    static Mesh createArch(float width, float height, float depth);
    static Mesh createPillar(float radius, float height, int segments = 8);
    static Mesh createStaircase(float width, float height, int steps);
    
    // Character parts
    static Mesh createCharacterTorso(float width, float height, float depth);
    static Mesh createCharacterHead(float radius);
    static Mesh createCharacterArm(float length, float radius);
    static Mesh createCharacterLeg(float length, float radius);
    static Mesh createCharacterHand(float size);
    static Mesh createCharacterFoot(float length, float width);
    
    // Weapons
    static Mesh createSword(float bladeLength, float bladeWidth, float hiltLength);
    static Mesh createGreatsword(float bladeLength, float bladeWidth, float hiltLength);
    static Mesh createStaff(float length, float radius);
    static Mesh createSpear(float bladeLength, float shaftLength);
    static Mesh createShield(float width, float height);
    static Mesh createDagger(float bladeLength, float hiltLength);
    
    // Accessories
    static Mesh createShoulderPad(float radius);
    static Mesh createBelt(float radius, float width);
    static Mesh createRing(float radius, float tubeRadius);
    
    // Combine meshes
    static Mesh combine(const Mesh& a, const Mesh& b);
};

// Mesh manager that loads/generates all game meshes
class MeshManager {
public:
    static MeshManager& instance();
    
    // Initialize all game meshes
    void initialize();
    
    // Get mesh by name
    const Mesh* getMesh(const std::string& name) const;
    
    // World meshes
    const Mesh* getTerrain(const std::string& era) const;  // "R1", "R2", "Link"
    const Mesh* getBuilding(const std::string& type) const; // "tower", "wall", "arch", "pillar"
    const Mesh* getDungeon(const std::string& type) const;  // "cave", "room", "corridor"
    
    // Character meshes
    const Mesh* getCharacter(const std::string& name) const; // "kite", "haseo", "blackrose", etc.
    const Mesh* getWeapon(const std::string& type) const;   // "dual_sword", "heavy_blade", "staff"
    
    // Predefined mesh names
    static constexpr const char* TERRAIN_R1 = "terrain_R1";
    static constexpr const char* TERRAIN_R2 = "terrain_R2";
    static constexpr const char* TERRAIN_LINK = "terrain_Link";
    
private:
    MeshManager() = default;
    std::vector<Mesh> meshes_;
    std::map<std::string, int> meshIndices_;
    
    void addMesh(const std::string& name, const Mesh& mesh);
    void generateWorldMeshes();
    void generateCharacterMeshes();
    void generateWeaponMeshes();
};

} // namespace mine

// Mesh helpers shared across World3D builders
void addQuad(mine::Mesh& m, int i0, int i1, int i2, int i3);
void addTriNorm(mine::Mesh& m, const mine::Vec3& a, const mine::Vec3& b, const mine::Vec3& c);
