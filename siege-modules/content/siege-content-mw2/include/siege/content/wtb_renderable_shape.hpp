#ifndef SIEGE_WTB_RENDERABLE_SHAPE_HPP
#define SIEGE_WTB_RENDERABLE_SHAPE_HPP

#include <siege/content/renderable_shape.hpp>
#include <siege/content/wtb.hpp>
#include <siege/content/bwd.hpp>

namespace siege::content::wtb
{
  renderable_shape_value make_renderable_shape(wtb_shape shape);
  renderable_shape_value make_renderable_shape(bwd::bwd_model shape);
}// namespace siege::content::wtb

#endif// DARKSTARDTSCONVERTER_DTS_RENDER_HPP
