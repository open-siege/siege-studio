using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.Runtime.InteropServices;

namespace Siege.Extension
{
    public static class Guids
    {
        public const string IDispatch = "00020400-0000-0000-C000-000000000046";
    }

    public class RealExportNameAttribute : Attribute
    {
        private string name;

        public RealExportNameAttribute(string name)
        {
            this.name = name;
        }
    }
}