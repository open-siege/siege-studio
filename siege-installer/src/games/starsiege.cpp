#include <filesystem>
#include <utility>
#include <string>
#include <fstream>
#include <iostream>
#include <cpr/cpr.h>

void unzipFile(std::string filename)
{
  std::system(("tar -xf " + filename).c_str());
}

namespace fs = std::filesystem;

int main()
{
  static_assert(sizeof(char) == sizeof(byte));
  cpr::Session session;

  if (!fs::exists("i5comp21.zip"))
  {
    std::ofstream file("i5comp21.zip", std::ios_base::binary | std::ios_base::trunc);
    session.SetUrl(cpr::Url{"https://sta.c64.org/winprg/i5comp21.zip"});
    session.SetWriteCallback(cpr::WriteCallback([&file](std::string data, std::intptr_t) {
      std::cout << "Reading " << data.size() << " bytes of data\n";
      file.write(data.data(), data.size());
      return true;
    }));

    auto response = session.Get();

    std::cout << "Response is " << response.downloaded_bytes << '\n';
  }

  unzipFile("i5comp21.zip");

  if (!fs::exists("starsiege-1000-en.001.zip"))
  {

    std::ofstream file("starsiege-1000-en.001.zip", std::ios_base::binary | std::ios_base::trunc);
    session.SetUrl(cpr::Url{"https://thesiegehub.files.wordpress.com/2022/02/starsiege-1000-en.001.docx"});
    session.SetWriteCallback(cpr::WriteCallback([&file](std::string data, std::intptr_t) {
      std::cout << "Reading " << data.size() << " bytes of data\n";
      file.write(data.data(), data.size());
      return true;
    }));

    auto response = session.Get();

    std::cout << "Response is " << response.downloaded_bytes << '\n';
  }

  unzipFile("starsiege-1000-en.001.zip");

  fs::create_directory("Starsiege");

  fs::current_path("Starsiege");
  std::cout << "Current path is" << fs::current_path() << '\n';
  std::system("..\\I5comp.exe x ..\\setup\\data1.cab");

  return 0;
}
