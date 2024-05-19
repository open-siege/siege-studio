using System;
using System.Collections;
using System.Runtime.InteropServices;

namespace Siege.TwoD.Content 
{
    public static class Exports
    {
        [DllImport("siege-2d-content.dll")]
        public static extern ICollection GetSupportedExtensions();
    }
}