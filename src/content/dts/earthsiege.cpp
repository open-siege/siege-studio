#include <array>
#include "shared.hpp"

namespace studio::content::dts::three_space
{
  using file_tag = std::array<std::byte, 4>;

  constexpr file_tag grid_shape_tag = shared::to_tag<4>({ 0x01, 0x00, 0xbc,  0x02 });
  constexpr file_tag rectangle_region_tag = shared::to_tag<4>({ 0x0c, 0x00, 0x28, 0x00 });
  constexpr file_tag edge_table_tag = shared::to_tag<4>({ 0x0d, 0x00, 0x28, 0x00 });

  constexpr file_tag null_obj_tag = shared::to_tag<4>({ 0x0e, 0x00, 0x14,  0x00 });
  constexpr file_tag poly_tag = shared::to_tag<4>({ 0x01, 0x00, 0x14,  0x00 });
  constexpr file_tag solid_poly_tag = shared::to_tag<4>({ 0x02, 0x00, 0x14,  0x00 });
  constexpr file_tag texture_for_poly_tag = shared::to_tag<4>({ 0x0f, 0x00, 0x14,  0x00 });
  constexpr file_tag shaded_polygon_tag = shared::to_tag<4>({ 0x03, 0x00, 0x14,  0x00  });
  constexpr file_tag gourand_poly_tag = shared::to_tag<4>({ 0x09, 0x00, 0x14,  0x00  });
  constexpr file_tag alias_solid_poly_tag = shared::to_tag<4>({ 0x10, 0x00, 0x14,  0x00  });
  constexpr file_tag alias_shaded_poly_tag = shared::to_tag<4>({ 0x11, 0x00, 0x14,  0x00  });
  constexpr file_tag alias_gourand_poly_tag = shared::to_tag<4>({ 0x12, 0x00, 0x14,  0x00  });
  constexpr file_tag group_tag = shared::to_tag<4>({ 0x14,  0x00, 0x14,  0x00  });
  constexpr file_tag bsp_group_tag = shared::to_tag<4>({ 0x0a,  0x00, 0x14,  0x00  });
  constexpr file_tag base_part_tag = shared::to_tag<4>({ 0x05, 0x00, 0x14,  0x00  });
  constexpr file_tag bitmap_part_tag = shared::to_tag<4>({ 0x13, 0x00, 0x14,  0x00  });
  constexpr file_tag part_list_tag = shared::to_tag<4>({ 0x07, 0x00, 0x14,  0x00  });
  constexpr file_tag cell_anim_part_tag = shared::to_tag<4>({ 0x0b, 0x00, 0x14,  0x00  });
  constexpr file_tag detail_part_tag = shared::to_tag<4>({ 0x0c, 0x00, 0x14,  0x00  });
  constexpr file_tag bsp_part_tag = shared::to_tag<4>({ 0x15, 0x00, 0x14,  0x00  });
  constexpr file_tag shape_tag = shared::to_tag<4>({ 0x08, 0x00, 0x14,  0x00  });
  constexpr file_tag an_sequence_tag = shared::to_tag<4>({ 0x01, 0x00, 0x1e,  0x00  });
  constexpr file_tag an_cyclic_sequence_tag = shared::to_tag<4>({ 0x04, 0x00, 0x1e,  0x00  });
  constexpr file_tag an_anim_list_tag = shared::to_tag<4>({ 0x02, 0x00, 0x1e,  0x00  });
  constexpr file_tag an_shape_tag = shared::to_tag<4>({ 0x03, 0x00, 0x1e,  0x00  });
  constexpr file_tag dc_shape_tag = shared::to_tag<4>({ 0xf8, 0x01, 0xbc,  0x02  });
  constexpr file_tag dc_shape_instance_tag = shared::to_tag<4>({ 0xf5, 0x01, 0xbc, 0x02 });

  constexpr file_tag nu_material_list_tag = shared::to_tag<4>({ 0x1f, 0x00, 0x14, 0x00 });
  constexpr file_tag nu_material_tag = shared::to_tag<4>({ 0x1e, 0x00, 0x14, 0x00 });
  constexpr file_tag nu_bsp_part_tag = shared::to_tag<4>({ 0x46, 0x00, 0x14, 0x00 });
  constexpr file_tag nu_cell_anim_part_tag = shared::to_tag<4>({ 0x51, 0x00, 0x14, 0x00 });
  constexpr file_tag nu_detail_part_tag = shared::to_tag<4>({ 0x5a, 0x00, 0x14, 0x00 });
  constexpr file_tag nu_mesh_tag = shared::to_tag<4>({ 0x28, 0x00, 0x14, 0x00 });
  constexpr file_tag nu_part_list_tag = shared::to_tag<4>({ 0x3c, 0x00, 0x14, 0x00 });
  constexpr file_tag nu_shape_tag = shared::to_tag<4>({ 0x64, 0x00, 0x14, 0x00 });
  constexpr file_tag nu_null_part_tag = shared::to_tag<4>({ 0x78, 0x00, 0x14, 0x00 });
  constexpr file_tag nu_bitmap_frame_tag = shared::to_tag<4>({ 0x6e, 0x00, 0x14, 0x00 });



}