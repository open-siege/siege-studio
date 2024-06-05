using System;
using System.Collections;
using System.Runtime.InteropServices;

namespace Siege.Content 
{
    public static class Exports
    {
        [DllImport("siege-content-2d.dll")]
        public static extern ICollection GetSupportedExtensions();
    }
}