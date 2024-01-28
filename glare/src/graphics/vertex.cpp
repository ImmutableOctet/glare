#include "vertex.hpp"

namespace graphics
{
    std::optional<BoneIndexVector::length_type> get_next_weight_channel(const StandardAnimationVertex& vertex)
    {
        using index_t = BoneIndexVector::length_type;

        for (auto channel = index_t {}; channel < VERTEX_MAX_BONE_INFLUENCE; channel++)
        {
            if (vertex.bone_indices[channel] == VERTEX_BONE_CHANNEL_OPEN)
            {
                return channel;
            }
        }

        return std::nullopt;
    }
}