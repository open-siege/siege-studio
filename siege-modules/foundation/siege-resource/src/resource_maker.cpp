#include <siege/resource/resource_maker.hpp>
#include <siege/resource/darkstar_resource.hpp>
#include <siege/resource/three_space_resource.hpp>
#include <siege/resource/zip_resource.hpp>
#include <siege/resource/trophy_bass_resource.hpp>
#include <siege/resource/sword_resource.hpp>
#include <siege/resource/cyclone_resource.hpp>
#include <siege/resource/iso_resource.hpp>
#include <siege/resource/seven_zip_resource.hpp>
#include <siege/resource/pak_resource.hpp>
#include <siege/resource/cab_resource.hpp>
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
    return vol::darkstar::vol_resource_reader::stream_is_supported(stream) || 
        vol::three_space::vol_resource_reader::stream_is_supported(stream) || 
        vol::three_space::dyn_resource_reader::stream_is_supported(stream) || 
        vol::three_space::rmf_resource_reader::stream_is_supported(stream) || 
        vol::three_space::vol_resource_reader::stream_is_supported(stream) || 
        vol::trophy_bass::rbx_resource_reader::stream_is_supported(stream) || 
        vol::trophy_bass::tbv_resource_reader::stream_is_supported(stream) ||
        pak::stream_is_supported(stream) ||
        clm::clm_resource_reader::stream_is_supported(stream) ||
        cln::cln_resource_reader::stream_is_supported(stream) ||
        atd::atd_resource_reader::stream_is_supported(stream) ||
        wad::wad_resource_reader::stream_is_supported(stream) ||
        rsc::rsc_resource_reader::stream_is_supported(stream) ||
        res::res_resource_reader::stream_is_supported(stream) ||
        mw4::mw4_resource_reader::stream_is_supported(stream) ||
        prj::prj_resource_reader::stream_is_supported(stream) ||
        zip::stream_is_supported(stream) ||
        cab::stream_is_supported(stream) || 
        iso::stream_is_supported(stream);
  }

  siege::platform::resource_reader make_resource_reader(std::istream& stream)
  {
    if (vol::darkstar::vol_resource_reader::stream_is_supported(stream))
    {
      return vol::darkstar::vol_resource_reader();
    }
    else if (vol::three_space::vol_resource_reader::stream_is_supported(stream))
    {
      return vol::three_space::vol_resource_reader();
    }
    else if (vol::three_space::dyn_resource_reader::stream_is_supported(stream))
    {
      return vol::three_space::dyn_resource_reader();
    }
    else if (vol::three_space::rmf_resource_reader::stream_is_supported(stream))
    {
      return vol::three_space::rmf_resource_reader();
    }
    else if (vol::trophy_bass::rbx_resource_reader::stream_is_supported(stream))
    {
      return vol::trophy_bass::rbx_resource_reader();
    }
    else if (clm::clm_resource_reader::stream_is_supported(stream))
    {
      return clm::clm_resource_reader();
    }
    else if (pak::stream_is_supported(stream))
    {
      return pak::pak_resource_reader();
    }
    else if (wad::wad_resource_reader::stream_is_supported(stream))
    {
      return wad::wad_resource_reader();
    }
    else if (rsc::rsc_resource_reader::stream_is_supported(stream))
    {
      return rsc::rsc_resource_reader();
    }
    else if (prj::prj_resource_reader::stream_is_supported(stream))
    {
      return prj::prj_resource_reader();
    }
    else if (mw4::mw4_resource_reader::stream_is_supported(stream))
    {
      return mw4::mw4_resource_reader();
    }
    else if (res::res_resource_reader::stream_is_supported(stream))
    {
      return res::res_resource_reader();
    }
    else if (cln::cln_resource_reader::stream_is_supported(stream))
    {
      return cln::cln_resource_reader();
    }
    else if (atd::atd_resource_reader::stream_is_supported(stream))
    {
      return atd::atd_resource_reader();
    }
    else if (zip::stream_is_supported(stream))
    {
      return zip::zip_resource_reader();
    }
    else if (cab::stream_is_supported(stream))
    {
      return cab::cab_resource_reader();
    }
    else if (iso::stream_is_supported(stream))
    {
      return iso::iso_resource_reader();
    }
    else
    {
      throw std::invalid_argument("Stream provided is not supported");
    }
  }
}// namespace siege::resource