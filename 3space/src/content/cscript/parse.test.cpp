#include <catch2/catch.hpp>
#include <string>
#include <iostream>
#include <functional>
#include <tao/pegtl/contrib/parse_tree.hpp>
#include "lua53.hpp"

TEST_CASE("Text is parsed correctly", "[pal.microsoft]")
{
  auto content = R"(function onAbilityCast(%caster, %targetPos)
  {
    %units = findUnitsInRadius(targetPos, 500);
    %enemies = __TS__ArrayFilter(
        units,
        function(____, unit) { return caster.isEnemy(unit) }
    );
    for ____, enemy in ipairs(enemies) do {
        enemy.kill();
    }
})";


  tao::pegtl::string_input in(content, "from_content");
  auto root = tao::pegtl::parse_tree::parse<lua53::grammar>(in);

  int level = 0;
  std::function<void(const tao::pegtl::parse_tree::node&)> write_node = [&](auto& child) {
    if (child.has_content())
    {
      std::cout << "Level " << level << " " << child.type << " " << child.string_view() << std::endl;
    }
    else
    {
      std::cout << "Level " << level << " " << child.type << std::endl;
    }


    level++;
    for (auto& child_child : child.children)
    {
      write_node(*child_child);
    }
    level--;
  };

  write_node(*root);

  REQUIRE(tao::pegtl::parse<lua53::grammar>(in) == true);
}