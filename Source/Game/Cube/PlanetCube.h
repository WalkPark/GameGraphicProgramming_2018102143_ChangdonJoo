#pragma once

#include "Cube/BaseCube.h"

class PlanetCube final : public BaseCube
{
public:
    PlanetCube(const std::filesystem::path& textureFilePath);
    PlanetCube(const PlanetCube& other) = delete;
    PlanetCube(PlanetCube&& other) = delete;
    PlanetCube& operator=(const PlanetCube& other) = delete;
    PlanetCube& operator=(PlanetCube&& other) = delete;
    ~PlanetCube() = default;

    void Update(_In_ FLOAT deltaTime) override;

private:
    XMMATRIX m_spin;
    XMMATRIX m_orbit;
    XMMATRIX m_translate;
    XMMATRIX m_scale;
};