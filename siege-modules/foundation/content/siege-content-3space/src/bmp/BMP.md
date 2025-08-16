### BMP File Format

BMP files, short for Bitmap, is a basic image format, typically used without compression and with widespread support.

For most Dynamix games, before the introduction of the Torque engine, the BMP format was one of the most commonly used file formats used, especially for Windows based games.

While there may be a shared extension, there are various internal representations of BMP, which are somewhat similar but with differences in features:

* Windows BMP
  * The most supported format across games and software alike.
  * It is common for most of the BMP files to have 8-bit indexed colour, supporting up to 256 colours.
  * Despite having an internal palette, some games, such as Starsiege, may force usage of a different palette, in either [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md) or [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md) format. In this instance, if a new image is created, the palette needs to be remapped to the palette used by the game.
  * Reserved fields in the header are used to store the ID of the palette to use, depending on the game.
  * The magic file header is **BM**. Information about the data structures can be found here:
    * https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader
    * https://docs.microsoft.com/en-us/previous-versions/dd183376(v=vs.85)
    * https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfo
* Phoenix BMP
  * This format is fundamentally the same as normal BMP files, except with different headers and additional features.
  * Just like Windows BMP files, it is common for these files to make use of 8-bit indexed colour.
  * Palette information is not typically stored inside of these files, and thus are often used in tandem with [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md) or [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md) files. However, embedding of Microsoft [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md) files is supported for Phoenix bitmaps.
  * Mipmaps are stored directly in the file, so that the game does not have to generate smaller versions of the texture dynamically. Each mipmap will be half the size of the previous image, if present. If the main image is 256x256, and if there are two additional detail levels, there will be a 128x128 image and a 64x64 image.
  * The magic file header is **PBMP**, with multiple chunks being present in the file, similar to the RIFF format.
    * There is a **head** chunk, which contains information about the image.
    * There is a **data** chunk, which contains the raw pixels of the image. If there are mipmaps, they will follow the main image and then each other sequentially. 
    * There is a **DETL** chunk, which contains the number of detail levels/mip maps in the image (including the primary image).
    * There is a **PiDX** chunk, which contains an ID for which palette to use. [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md) files contain multiple palettes, each with a unique ID. 
* Other BMP formats
  * Depending on the game, there are other variations of the BMP format, which have not been documented. When more information is known about them, it will be added here.

#### Source Code
* BMP
    * [siege-modules/foundation/siege-content/include/siege/content/bitmap.hpp](/siege-modules/content/siege-content-3space/include/siege/content/bmp/bitmap.hpp)
    * [siege-modules/foundation/siege-content/src/bmp/bitmap.cpp](/siege-modules/content/siege-content-3space/src/bmp/bitmap.cpp)