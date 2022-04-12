#pragma once

#include <iostream>

#include "../Math/Vector.h"
#include "../Texture/Texture.h"
#include "Grid.h"

namespace Scene3D
{
    namespace TerrainType
    {
        enum Types
        {
            Flat = 0,
            Random = 1,
            Perlin_noise = 2
        };
    }
    class Terrain
    {
        public:
            Terrain();

            Terrain(const unsigned int GRID_SIZE_X, const unsigned int GRID_SIZE_Y, TerrainType::Types TERRAIN_TYPE);
            Terrain(const unsigned int GRID_SIZE_X, const unsigned int GRID_SIZE_Y, Texture HEIGHT_MAP);
            
            Terrain(const Vector2i GRID_SIZE, TerrainType::Types TERRAIN_TYPE);
            Terrain(const Vector2i GRID_SIZE, Texture HEIGHT_MAP);

            ~Terrain();

            void setTexture(Texture texture);
            void setColor(const float R, const float G, const float B);
            void setColor(const Vector3f RGB);


        protected:
        private:
            int m_Max_Size = 100;
            Texture m_Height_Map;

    };
}