#ifndef DARKSTARDTSCONVERTER_STRUCTURES_HPP
#define DARKSTARDTSCONVERTER_STRUCTURES_HPP

#include <boost/endian/arithmetic.hpp>
#include <array>

namespace darkstar::dts
{
    namespace endian = boost::endian;
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

    struct quaternion4s
    {
        endian::little_int16_t x;
        endian::little_int16_t y;
        endian::little_int16_t z;
        endian::little_int16_t w;
    };

    struct quaternion4f
    {
        float x;
        float y;
        float z;
        float w;
    };

    static_assert(sizeof(quaternion4s) == sizeof(std::array<endian::little_int16_t, 4>));

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

        struct data
        {
            float radius;
            vector3f centre;
        };

        static_assert(sizeof(data) == sizeof(std::array<float, 4>));

        struct node
        {
            endian::little_int32_t name;
            endian::little_int32_t parent;
            endian::little_int32_t num_sub_sequences;
            endian::little_int32_t first_sub_sequence;
            endian::little_int32_t default_transform;
        };

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

        struct sub_sequence
        {
            endian::little_int32_t sequence_index;
            endian::little_int32_t num_key_frames;
            endian::little_int32_t first_key_frame;
        };

        struct keyframe
        {
            float position;
            endian::little_uint32_t key_value;
            endian::little_uint32_t mat_index;
        };

        struct transform
        {
            quaternion4s rotation;
            vector3f translation;
            vector3f scale;
        };

        static_assert(sizeof(transform) == sizeof(std::array<std::int32_t, 8>));

        using name = std::array<std::byte, 24>;

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

        struct detail
        {
            endian::little_int32_t name_index;
            float size;
        };

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

        struct frame_trigger
        {
            float position;
            float value;
        };

        struct footer
        {
            endian::little_int32_t num_default_materials;
            endian::little_int32_t always_node;
        };

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

        struct vertex
        {
            std::uint8_t x;
            std::uint8_t y;
            std::uint8_t z;
            std::uint8_t normal;
        };

        struct texture_vertex
        {
            float x;
            float y;
        };

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

        static_assert(sizeof(vertex) == sizeof(std::int32_t));
    }

    namespace material_list::v3
    {
        struct header
        {
            endian::little_int32_t num_details;
            endian::little_int32_t num_materials;
        };

        struct material
        {
            endian::little_int32_t flags;
            float alpha;
            endian::little_int32_t index;
            std::uint8_t red;
            std::uint8_t green;
            std::uint8_t blue;
            std::uint8_t rgb_flags;

            std::array<std::byte, 32> file_name;

            endian::little_int32_t type;
            float elasticity;
            float friction;
        };
    }

    struct mesh_v3
    {
        mesh::v3::header header;
        std::vector<mesh::v3::vertex> vertices;
        std::vector<mesh::v3::texture_vertex> texture_vertices;
        std::vector<mesh::v3::face> faces;
        std::vector<mesh::v3::frame> frames;
    };

    struct material_list_v3
    {
        material_list::v3::header header;
        std::vector<material_list::v3::material> materials;
    };

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
}

#endif //DARKSTARDTSCONVERTER_STRUCTURES_HPP
