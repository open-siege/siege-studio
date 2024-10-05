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

namespace siege::resource
{
  bool is_resource_reader(std::istream& stream)
  {
    return vol::darkstar::vol_resource_reader::is_supported(stream) || 
        vol::three_space::vol_resource_reader::is_supported(stream) || 
        vol::three_space::dyn_resource_reader::is_supported(stream) || 
        vol::three_space::rmf_resource_reader::is_supported(stream) || 
        vol::three_space::vol_resource_reader::is_supported(stream) || 
        vol::trophy_bass::rbx_resource_reader::is_supported(stream) || 
        vol::trophy_bass::tbv_resource_reader::is_supported(stream) ||
        pak::pak_resource_reader::is_supported(stream); 
        // TODO make the check for cyclone resources stronger
//        cln::cln_resource_reader::is_supported(stream) || 
    //    atd::atd_resource_reader::is_supported(stream) || 
    //    zip::zip_resource_reader::is_supported(stream) || 
    //    cab::cab_resource_reader::is_supported(stream) || 
    //    iso::iso_resource_reader::is_supported(stream);
  }
  std::unique_ptr<siege::platform::resource_reader> make_resource_reader(std::istream& stream)
  {
    std::unique_ptr<siege::platform::resource_reader> resource;
    if (vol::darkstar::vol_resource_reader::is_supported(stream))
    {
      resource.reset(new vol::darkstar::vol_resource_reader());
    }
    else if (vol::three_space::vol_resource_reader::is_supported(stream))
    {
      resource.reset(new vol::three_space::vol_resource_reader());
    }
    else if (vol::three_space::dyn_resource_reader::is_supported(stream))
    {
      resource.reset(new vol::three_space::dyn_resource_reader());
    }
    else if (vol::three_space::rmf_resource_reader::is_supported(stream))
    {
      resource.reset(new vol::three_space::rmf_resource_reader());
    }
    else if (vol::trophy_bass::rbx_resource_reader::is_supported(stream))
    {
      resource.reset(new vol::trophy_bass::rbx_resource_reader());
    }
    else if (pak::pak_resource_reader::is_supported(stream))
    {
      resource.reset(new pak::pak_resource_reader());
    }
    // TODO make the check for cyclone resources stronger
 //   else if (cln::cln_resource_reader::is_supported(stream))
  //  {
 //     resource.reset(new cln::cln_resource_reader());
  //  }
    else if (atd::atd_resource_reader::is_supported(stream))
    {
      resource.reset(new atd::atd_resource_reader());
    }
    else if (zip::zip_resource_reader::is_supported(stream))
    {
      resource.reset(new zip::zip_resource_reader());
    }
    else if (cab::cab_resource_reader::is_supported(stream))
    {
      resource.reset(new cab::cab_resource_reader());
    }
    else if (iso::iso_resource_reader::is_supported(stream))
    {
      resource.reset(new iso::iso_resource_reader());
    }
    else
    {
      throw std::invalid_argument("Stream provided is not supported");
    }

    return resource;
  }
}// namespace siege::resource