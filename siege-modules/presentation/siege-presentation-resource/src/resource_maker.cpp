#include <siege/resource/darkstar_resource.hpp>
#include <siege/resource/three_space_resource.hpp>
#include <siege/resource/common_resources.hpp>
#include <siege/resource/trophy_bass_resource.hpp>
#include <siege/resource/sword_resource.hpp>
#include <siege/resource/cyclone_resource.hpp>
#include <siege/resource/pak_resource.hpp>
#include <siege/resource/clm_resource.hpp>
#include <siege/resource/wad_resource.hpp>
#include <siege/resource/rsc_resource.hpp>
#include <siege/resource/res_resource.hpp>
#include <siege/resource/mw4_resource.hpp>
#include <siege/resource/prj_resource.hpp>

namespace siege::resource
{
  bool is_resource_readable(std::istream& stream)
  {
    return vol::darkstar::is_stream_supported(stream) || vol::three_space::is_stream_vol(stream) || vol::three_space::is_stream_dyn(stream) || vol::three_space::is_stream_rmf(stream) || vol::three_space::is_stream_vol(stream) || vol::trophy_bass::is_stream_rbx(stream) || vol::trophy_bass::is_stream_tbv(stream) || pak::is_stream_supported(stream) || clm::is_stream_supported(stream) || cln::is_stream_supported(stream) || atd::is_stream_supported(stream) || wad::is_stream_supported(stream) || rsc::is_stream_supported(stream) || res::is_stream_supported(stream) || mw4::is_stream_supported(stream) || prj::is_stream_supported(stream) || zip::is_stream_zip(stream) || cab::is_stream_supported(stream) || iso::is_stream_supported(stream);
  }

  siege::platform::resource_reader make_resource_reader(std::istream& stream)
  {
    if (vol::darkstar::is_stream_supported(stream))
    {
      return vol::darkstar::make_resource_reader();
    }
    else if (vol::three_space::is_stream_vol(stream))
    {
      return vol::three_space::make_vol_resource_reader();
    }
    else if (vol::three_space::is_stream_dyn(stream))
    {
      return vol::three_space::make_dyn_resource_reader();
    }
    else if (vol::three_space::is_stream_rmf(stream))
    {
      return vol::three_space::make_rmf_resource_reader();
    }
    else if (vol::trophy_bass::is_stream_rbx(stream))
    {
      return vol::trophy_bass::make_tbv_resource_reader();
    }
    else if (clm::is_stream_supported(stream))
    {
      return clm::make_resource_reader();
    }
    else if (pak::is_stream_supported(stream))
    {
      return pak::make_resource_reader();
    }
    else if (wad::is_stream_supported(stream))
    {
      return wad::make_resource_reader();
    }
    else if (rsc::is_stream_supported(stream))
    {
      return rsc::make_resource_reader();
    }
    else if (prj::is_stream_supported(stream))
    {
      return prj::make_resource_reader();
    }
    else if (mw4::is_stream_supported(stream))
    {
      return mw4::make_resource_reader();
    }
    else if (res::is_stream_supported(stream))
    {
      return res::make_resource_reader();
    }
    else if (cln::is_stream_supported(stream))
    {
      return cln::make_resource_reader();
    }
    else if (atd::is_stream_supported(stream))
    {
      return atd::make_resource_reader();
    }
    else if (zip::is_stream_zip(stream))
    {
      return zip::make_zip_resource_reader();
    }
    else if (cab::is_stream_supported(stream))
    {
      return cab::make_resource_reader();
    }
    else if (iso::is_stream_supported(stream))
    {
      return iso::make_resource_reader();
    }
    else
    {
      throw std::invalid_argument("Stream provided is not supported");
    }
  }
}// namespace siege::resource