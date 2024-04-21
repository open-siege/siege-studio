using System;
using System.Collections;
using System.Runtime.InteropServices;

namespace Siege.UI.TwoD.Content 
{
    public static class Exports
    {
        [DllImport("siege-ui-2d-content.dll")]
        public static extern ICollection GetSupportedExtensions();
    }
}