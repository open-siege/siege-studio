#ifndef DARKSTARDTSCONVERTER_STRUCTURES_HPP
#define DARKSTARDTSCONVERTER_STRUCTURES_HPP

#include "json_boost.hpp"
#include <array>

namespace darkstar::dts
{
    namespace endian = boost::endian;
    using json = nlohmann::json;
    using file_tag = std::array<std::byte, 4>;

    constexpr file_tag to_tag(const std::array<std::uint8_t, 4> values)
    {
        file_tag result{};

        for (int i = 0; i < values.size(); i++)
        {
            result[i] = std::byte{values[i]};
        }
        return result;
    }

    constexpr file_tag pers_tag = to_tag({'P', 'E', 'R', 'S'});

    using version = endian::little_int32_t;

    struct file_info
    {
        endian::little_int32_t file_length;
        endian::little_int16_t class_name_length;
    };

    struct tag_header
    {
        dts::file_tag tag;
        dts::file_info file_info;
        std::vector<std::byte> class_name;
        dts::version version;
    };

    struct vector3f
    {
        float x;
        float y;
        float z;
    };

    static_assert(sizeof(vector3f) == sizeof(std::array<float, 3>));
    NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(vector3f, x, y, z);

    struct quaternion4s
    {
        endian::little_int16_t x;
        endian::little_int16_t y;
        endian::little_int16_t z;
        endian::little_int16_t w;
    };

    NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(quaternion4s, x, y, z, w);

    struct quaternion4f
    {
        float x;
        float y;
        float z;
        float w;
    };

    static_assert(sizeof(quaternion4s) == sizeof(std::array<endian::little_int16_t, 4>));
    NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(quaternion4f, x, y, z, w);


    namespace shape::v7
    {
        struct header
        {
            endian::little_int32_t num_nodes;
            endian::little_int32_t num_sequences;
            endian::little_int32_t num_sub_sequences;
            endian::little_int32_t num_key_frames;
            endian::little_int32_t num_transforms;
            endian::little_int32_t num_names;
            endian::little_int32_t num_objects;
            endian::little_int32_t num_details;
            endian::little_int32_t num_meshes;
            endian::little_int32_t num_transitions;
            endian::little_int32_t num_frame_triggers;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(header, num_nodes, num_sequences, num_sub_sequences, num_key_frames,
                num_transforms, num_names, num_objects, num_details, num_meshes, num_transitions, num_frame_triggers)

        struct data
        {
            float radius;
            vector3f centre;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(data, radius, centre);

        static_assert(sizeof(data) == sizeof(std::array<float, 4>));

        struct node
        {
            endian::little_int32_t name;
            endian::little_int32_t parent;
            endian::little_int32_t num_sub_sequences;
            endian::little_int32_t first_sub_sequence;
            endian::little_int32_t default_transform;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(node, name, parent, num_sub_sequences, first_sub_sequence, default_transform)

        struct sequence
        {
            endian::little_int32_t name_index;
            endian::little_int32_t cyclic;
            float duration;
            endian::little_int32_t priority;
            endian::little_int32_t first_frame_trigger;
            endian::little_int32_t num_frame_triggers;
            endian::little_int32_t num_ifl_sub_sequences;
            endian::little_int32_t first_ifl_sub_sequence;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(sequence, name_index, cyclic, duration, priority, first_frame_trigger,
                num_frame_triggers, num_ifl_sub_sequences, first_ifl_sub_sequence)

        struct sub_sequence
        {
            endian::little_int32_t sequence_index;
            endian::little_int32_t num_key_frames;
            endian::little_int32_t first_key_frame;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(sub_sequence, sequence_index, num_key_frames, first_key_frame)

        struct keyframe
        {
            float position;
            endian::little_uint32_t key_value;
            endian::little_uint32_t mat_index;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(keyframe, position, key_value, mat_index)

        struct transform
        {
            quaternion4s rotation;
            vector3f translation;
            vector3f scale;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(transform, rotation, translation, scale)

        static_assert(sizeof(transform) == sizeof(std::array<std::int32_t, 8>));

        using name = std::array<char, 24>;

        struct object
        {
            endian::little_int16_t name_index;
            endian::little_int16_t flags;
            endian::little_int32_t mesh_index;
            endian::little_int32_t node_index;
            endian::little_int32_t dep_flags;
            std::array<vector3f,3> dep;
            vector3f object_offset;
            endian::little_int32_t num_sub_sequences;
            endian::little_int32_t first_sub_sequences;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(object, name_index, flags, mesh_index, node_index, dep_flags, dep, object_offset, num_sub_sequences, first_sub_sequences)

        struct detail
        {
            endian::little_int32_t name_index;
            float size;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(detail, name_index, size)

        struct transition
        {
            endian::little_int32_t start_sequence;
            endian::little_int32_t end_sequence;
            float start_position;
            float end_position;
            float duration;
            quaternion4s rotation;
            vector3f translation;
            vector3f scale;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(transition, start_sequence, end_sequence, start_position, end_position, duration, rotation, translation, scale)

        struct frame_trigger
        {
            float position;
            float value;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(frame_trigger, position, value)

        struct footer
        {
            endian::little_int32_t num_default_materials;
            endian::little_int32_t always_node;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(footer, num_default_materials, always_node)

        using has_material_list_flag = endian::little_int32_t;
    }

    namespace mesh::v3
    {
        struct header
        {
            endian::little_int32_t num_verts;
            endian::little_int32_t verts_per_frame;
            endian::little_int32_t num_texture_verts;
            endian::little_int32_t num_faces;
            endian::little_int32_t num_frames;
            endian::little_int32_t texture_verts_per_frame;
            float radius;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(header, num_verts, verts_per_frame, num_texture_verts, num_faces,
                num_frames, texture_verts_per_frame, radius)

        struct vertex
        {
            std::uint8_t x;
            std::uint8_t y;
            std::uint8_t z;
            std::uint8_t normal;
        };

        static_assert(sizeof(vertex) == sizeof(std::int32_t));
        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(vertex, x, y, z, normal)

        struct texture_vertex
        {
            float x;
            float y;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(texture_vertex, x, y)

        struct face
        {
            endian::little_int32_t vi1;
            endian::little_int32_t ti1;
            endian::little_int32_t vi2;
            endian::little_int32_t ti2;
            endian::little_int32_t vi3;
            endian::little_int32_t ti3;
            endian::little_int32_t material;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(face, vi1, ti1, vi2, ti2, vi3, ti3, material)

        struct frame
        {
            endian::little_int32_t first_vert;
            float scale_x;
            float scale_y;
            float scale_z;
            float origin_x;
            float origin_y;
            float origin_z;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(frame, first_vert, scale_x, scale_y, scale_z, origin_x, origin_y, origin_z)
    }

    namespace material_list::v3
    {
        struct header
        {
            endian::little_int32_t num_details;
            endian::little_int32_t num_materials;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(header, num_details, num_materials)

        struct rgb_data
        {
            std::uint8_t red;
            std::uint8_t green;
            std::uint8_t blue;
            std::uint8_t rgb_flags;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(rgb_data, red, green, blue, rgb_flags)

        struct material
        {
            endian::little_int32_t flags;
            float alpha;
            endian::little_int32_t index;
            rgb_data rgb_data;

            std::array<char, 32> file_name;

            endian::little_int32_t type;
            float elasticity;
            float friction;
        };

        NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(material, flags, alpha, index, rgb_data, file_name, type, elasticity, friction)
    }

    struct mesh_v3
    {
        mesh::v3::header header;
        std::vector<mesh::v3::vertex> vertices;
        std::vector<mesh::v3::texture_vertex> texture_vertices;
        std::vector<mesh::v3::face> faces;
        std::vector<mesh::v3::frame> frames;
    };

    NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(mesh_v3, header, vertices, texture_vertices, faces, frames)

    struct material_list_v3
    {
        material_list::v3::header header;
        std::vector<material_list::v3::material> materials;
    };

    NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(material_list_v3, header, materials)

    struct shape_v7
    {
        shape::v7::header header;
        shape::v7::data data;
        std::vector<shape::v7::node> nodes;
        std::vector<shape::v7::sequence> sequences;
        std::vector<shape::v7::sub_sequence> sub_sequences;
        std::vector<shape::v7::keyframe> keyframes;
        std::vector<shape::v7::transform> transforms;
        std::vector<shape::v7::name> names;
        std::vector<shape::v7::object> objects;
        std::vector<shape::v7::detail> details;
        std::vector<shape::v7::transition> transitions;
        std::vector<shape::v7::frame_trigger> frame_triggers;
        shape::v7::footer footer;
        std::vector<mesh_v3> meshes;

        material_list_v3 material_list;
    };

    NLOHMANN_DEFINE_UNORDERED_TYPE_NON_INTRUSIVE(shape_v7, header, data, nodes, sequences, sub_sequences, keyframes, transforms,
                    names, objects, details, transitions, frame_triggers, footer, meshes, material_list)
}

#endif //DARKSTARDTSCONVERTER_STRUCTURES_HPP
