using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.Runtime.InteropServices;
using Siege.Platform;

namespace Siege.Platform
{
    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid("00020400-0000-0000-C000-000000000046")]
    public interface IReadOnlyCollection
    {
        [DispId(-4)]
        System.Collections.IEnumerator GetEnumerator();

        [DispId(0)]
        object Item(int index);

        int Count
        {
            [DispId(1)]
            get;
        }
    }

    internal enum STGM
    {
        STGM_READ = 0x00000000,
        STGM_WRITE = 0x00000001,
        STGM_READWRITE = 0x00000002
    }

    internal enum STGTY
    {
        STGTY_STORAGE = 1,
        STGTY_STREAM = 2,
        STGTY_LOCKBYTES = 3,
        STGTY_PROPERTY = 4
    }

    internal enum STREAM_SEEK
    {
        STREAM_SEEK_SET = 0,
        STREAM_SEEK_CUR = 1,
        STREAM_SEEK_END = 2
    }

    public struct StreamAdaptor : System.Runtime.InteropServices.ComTypes.IStream
    {
        private System.IO.Stream stream;

        public StreamAdaptor(System.IO.Stream stream)
        {
            this.stream = stream;
        }

        public void Read(byte[] buffer, int bufferSize, IntPtr bytesReadPtr)
        {
            var bytesRead = stream.Read(buffer, 0, bufferSize);

            if (bytesReadPtr != IntPtr.Zero)
            {
                Marshal.WriteInt32(bytesReadPtr, bytesRead);
            }
        }

        public void Write(byte[] buffer, int bufferSize, IntPtr bytesWrittenPtr)
        {
            stream.Write(buffer, 0, bufferSize);

            if (bytesWrittenPtr != IntPtr.Zero)
            {
                Marshal.WriteInt32(bytesWrittenPtr, bufferSize);
            }
        }

        public void Seek(long offset, int origin, IntPtr newPositionPtr)
        {
            System.IO.SeekOrigin seekOrigin;

            switch ((STREAM_SEEK)origin)
            {
                case STREAM_SEEK.STREAM_SEEK_SET:
                    seekOrigin = SeekOrigin.Begin;
                    break;
                case STREAM_SEEK.STREAM_SEEK_CUR:
                    seekOrigin = SeekOrigin.Current;
                    break;
                case STREAM_SEEK.STREAM_SEEK_END:
                    seekOrigin = SeekOrigin.End;
                    break;
                default:
                    throw new ArgumentOutOfRangeException("origin");
            }
            long position = stream.Seek(offset, seekOrigin);

            if (newPositionPtr != IntPtr.Zero)
            {
                Marshal.WriteInt64(newPositionPtr, position);
            }
        }

        public void SetSize(long newSize)
        {
            stream.SetLength(newSize);
        }

        public void Stat(out System.Runtime.InteropServices.ComTypes.STATSTG info, int flags)
        {
            info = new System.Runtime.InteropServices.ComTypes.STATSTG();
            info.type = (int)STGTY.STGTY_STREAM;
            info.cbSize = stream.Length;

            info.grfMode = 0; 
            if (stream.CanRead && stream.CanWrite)
            {
                info.grfMode |= (int)STGM.STGM_READWRITE;
            }
            else if (stream.CanRead)
            {
                info.grfMode |= (int)STGM.STGM_READ;
            }
            else if (stream.CanWrite)
            {
                info.grfMode |= (int)STGM.STGM_WRITE;
            }
            else
            {
                throw new System.IO.IOException();
            }
        }

        public void Commit(int flags)
        {
            throw new NotSupportedException();
        }

        public void Revert()
        {
            throw new NotSupportedException();
        }

        public void CopyTo(System.Runtime.InteropServices.ComTypes.IStream _, long bufferSize, IntPtr buffer, IntPtr bytesWrittenPtr)
        {
            throw new NotSupportedException();
        }

        public void Clone(out System.Runtime.InteropServices.ComTypes.IStream copy)
        {
            copy = null;
            throw new NotSupportedException();
        }

        public void LockRegion(long offset, long byteCount, int lockType)
        {
            throw new NotSupportedException();
        }

        public void UnlockRegion(long offset, long byteCount, int lockType)
        {
            throw new NotSupportedException();
        }
    }

    public struct EnumeratorAdaptor : IEnumerator<string>
    {
        private IEnumerator enumerator;

        public EnumeratorAdaptor(IEnumerator enumerator)
        {
            this.enumerator = enumerator;
        }

        public bool MoveNext()
        {
            return enumerator.MoveNext();
        }

        public void Reset()
        {
            enumerator.Reset();
        }

        public void Dispose()
        {
        }

        object IEnumerator.Current
        {
            get => enumerator.Current;
        }

        string IEnumerator<string>.Current
        {
            get => (enumerator.Current as string) ?? string.Empty;
        }
    }

    public struct ReadOnlyStringCollection : IReadOnlyCollection<string>
    {
        private IReadOnlyCollection collection;

        public ReadOnlyStringCollection(IReadOnlyCollection collection)
        {
            this.collection = collection;
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return collection.GetEnumerator();
        }

        IEnumerator<string> IEnumerable<string>.GetEnumerator()
        {
            return new EnumeratorAdaptor(collection.GetEnumerator());
        }

        public int Count
        {
            get => collection.Count;
        }
    }
}