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
        public static IReadOnlyCollection<string> get_supported_extensions()
        {
            return new Platform.ReadOnlyStringCollection(Imports.get_supported_extensions());
        }

        public static IReadOnlyCollection<string> get_supported_format_categories(uint locale)
        {
            return new Platform.ReadOnlyStringCollection(Imports.get_supported_format_categories(locale));
        }

        public static IReadOnlyCollection<string> get_supported_extensions(string category)
        {
            return new Platform.ReadOnlyStringCollection(Imports.get_supported_extensions_for_category(category));
        }

        public static bool is_stream_supported(System.IO.Stream reader)
        {
            return Imports.is_stream_supported(new StreamAdaptor(reader)) == 0;
        }

        public static string get_window_class_for_stream(System.IO.Stream reader)
        {
            return Marshal.PtrToStringAuto(Imports.get_window_class_for_stream(new StreamAdaptor(reader))) ?? string.Empty;
        }
    }

    internal static class Imports
    {
        [DllImport("siege-content-resource.dll", CharSet = CharSet.Unicode, PreserveSig = false)]
        public static extern Platform.IReadOnlyCollection get_supported_extensions();

        [DllImport("siege-content-resource.dll", CharSet = CharSet.Unicode, PreserveSig = false)]
        public static extern Platform.IReadOnlyCollection get_supported_format_categories(uint locale);

        [DllImport("siege-content-resource.dll", CharSet = CharSet.Unicode, PreserveSig = false)]
        public static extern Platform.IReadOnlyCollection get_supported_extensions_for_category(string category);

        [DllImport("siege-content-resource.dll", CharSet = CharSet.Unicode, PreserveSig = true)]
        public static extern uint is_stream_supported(System.Runtime.InteropServices.ComTypes.IStream stream);

        [DllImport("siege-content-resource.dll", CharSet = CharSet.Unicode, PreserveSig = false)]
        public static extern IntPtr get_window_class_for_stream(System.Runtime.InteropServices.ComTypes.IStream stream);
    }
}