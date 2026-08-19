#pragma once
#include <string>
#include <vector>
#include <memory>

namespace mine {

// 3D model loaded from original game data
struct Model3D {
    std::string name;
    std::string path;
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> uvs;
    std::vector<unsigned int> indices;
    int texture_id = -1;
};

// Shader for rendering
struct Shader {
    std::string name;
    std::string vertex_path;
    std::string fragment_path;
    int program_id = 0;
};

// Texture
struct Texture {
    std::string name;
    std::string path;
    int width = 0, height = 0;
    int texture_id = 0;
};

// Character model with animations
struct CharacterModel {
    std::string name;
    Model3D mesh;
    struct Animation {
        std::string name;
        int start_frame;
        int end_frame;
        bool loop;
    };
    std::vector<Animation> animations;
    int current_anim = 0;
    float anim_time = 0.0f;
};

// World renderer that loads original game assets
class WorldRenderer {
public:
    WorldRenderer();
    ~WorldRenderer();

    bool initialize();
    void shutdown();

    // Load disk-specific world
    bool loadWorld(const std::string& disc_id);
    void unloadWorld();

    // Load original game assets
    bool loadModel(const std::string& path, Model3D& model);
    bool loadTexture(const std::string& path, Texture& texture);
    bool loadShader(const std::string& vertex_path, const std::string& fragment_path, Shader& shader);

    // Render
    void beginFrame();
    void endFrame();
    void renderWorld();
    void renderCharacter(const CharacterModel& character);
    void renderSky();
    void renderGround();
    void renderStructures();

    // Camera
    void setCameraPosition(float x, float y, float z);
    void setCameraRotation(float yaw, float pitch);

    // World info
    std::string getCurrentWorldName() const { return current_world_name_; }
    std::string getCurrentEra() const { return current_era_; }

private:
    bool loadWorldMesh(const std::string& disc_id);
    bool loadCharacterMeshes(const std::string& disc_id);
    bool loadWorldShaders(const std::string& disc_id);
    bool loadWorldTextures(const std::string& disc_id);

    std::string current_world_name_;
    std::string current_era_;
    std::string current_disc_;

    Model3D world_mesh_;
    std::vector<CharacterModel> characters_;
    std::vector<Texture> textures_;
    std::vector<Shader> shaders_;

    float cam_x_ = 0.0f, cam_y_ = 0.0f, cam_z_ = 0.0f;
    float cam_yaw_ = 0.0f, cam_pitch_ = 0.0f;
};

} // namespace mine
