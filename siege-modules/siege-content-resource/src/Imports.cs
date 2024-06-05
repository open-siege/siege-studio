using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.Runtime.InteropServices;
using Siege.Platform;

namespace Siege.Content
{
    public static class Resource
    {
        public static IReadOnlyCollection<string> GetSupportedExtensions()
        {
            return new Platform.ReadOnlyStringCollection(Imports.GetSupportedExtensions());
        }

        public static IReadOnlyCollection<string> GetSupportedFormatCategories(uint locale)
        {
            return new Platform.ReadOnlyStringCollection(Imports.GetSupportedFormatCategories(locale));
        }

        public static IReadOnlyCollection<string> GetSupportedExtensions(string category)
        {
            return new Platform.ReadOnlyStringCollection(Imports.GetSupportedExtensionsForCategory(category));
        }

        public static bool IsStreamSupported(System.IO.Stream reader)
        {
            return Imports.IsStreamSupported(new StreamAdaptor(reader)) == 0;
        }

        public static string GetWindowClassForStream(System.IO.Stream reader)
        {
            return Marshal.PtrToStringAuto(Imports.GetWindowClassForStream(new StreamAdaptor(reader))) ?? string.Empty;
        }
    }

    internal static class Imports
    {
        [DllImport("siege-content-resource.dll", CharSet = CharSet.Unicode, PreserveSig = false)]
        public static extern Platform.IReadOnlyCollection GetSupportedExtensions();

        [DllImport("siege-content-resource.dll", CharSet = CharSet.Unicode, PreserveSig = false)]
        public static extern Platform.IReadOnlyCollection GetSupportedFormatCategories(uint locale);

        [DllImport("siege-content-resource.dll", CharSet = CharSet.Unicode, PreserveSig = false)]
        public static extern Platform.IReadOnlyCollection GetSupportedExtensionsForCategory(string category);

        [DllImport("siege-content-resource.dll", CharSet = CharSet.Unicode, PreserveSig = true)]
        public static extern uint IsStreamSupported(System.Runtime.InteropServices.ComTypes.IStream stream);

        [DllImport("siege-content-resource.dll", CharSet = CharSet.Unicode, PreserveSig = false)]
        public static extern IntPtr GetWindowClassForStream(System.Runtime.InteropServices.ComTypes.IStream stream);
    }
}