using System;
using System.IO;
using System.Collections;
using System.Runtime.InteropServices;

namespace Siege.Tools 
{
    public static class Nuvol
    {
        public static void Main(string[] args)
        {
            Console.WriteLine("Hello Nuvol");

            var extensions = Siege.Content.Resource.GetSupportedExtensions();

            foreach (var value in extensions)
            {

                Console.WriteLine(value);
            }

            var categories = Siege.Content.Resource.GetSupportedFormatCategories(0);

            foreach (string value in categories)
            {
                var extensionsForCategory = Siege.Content.Resource.GetSupportedExtensions(category: value);

                foreach (var ext in extensionsForCategory)
                {
                    Console.WriteLine($"{value}: {ext}");
                }
            }

            var stream1 = new FileStream("C:\\open-siege\\Games\\Starsiege 1.0.0.0\\Desert.Sim.vol", FileMode.Open);

            if (Siege.Content.Resource.IsStreamSupported(stream1))
            {
                Console.WriteLine("Stream is supported");
                Console.WriteLine(Siege.Content.Resource.GetWindowClassForStream(stream1));
            }
            else
            {
                Console.WriteLine("Stream is not supported");
            }
        }
    }
}