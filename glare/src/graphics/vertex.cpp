#include "vertex.hpp"

namespace graphics
{
    int get_next_weight_channel(const StandardAnimationVertex& vertex)
    {
        for (int i = 0; i < VERTEX_MAX_BONE_INFLUENCE; i++)
        {
            if (vertex.bone_indices[i] == -1) // && (vertex.bone_weights[i] == 0.0f)
            {
                return i;
            }
        }

        return -1;
    }
}