using System;
using System.Collections.Generic ;
using System.Linq;
using System.Runtime.InteropServices ;
using System.Threading ;
using System.Threading.Tasks ;

using ArkaneSystems.Arkane.MessagePump;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Arkane.MessagePump.Test
{
    [TestClass]
    public class MessagePumpUnitTest
    {
        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        static extern IntPtr SendMessage (IntPtr hWnd, UInt32 msg, UIntPtr wParam, IntPtr lParam) ;

        [TestMethod]
        public void SendingMessages()
        {
            var messagePump = new MessageOnlyWindow () ;
            var messageList = new List <WindowMessage> () ;
            var loopHasStarted = new ManualResetEventSlim(false);

            // ReSharper disable once InconsistentNaming
            const uint WM_APP = 0x8000 ;

            messagePump.MessageReceived += message =>
                                           {
                                               lock (messageList)
                                               {
                                                   messageList.Add (message) ;
                                               }
                                               loopHasStarted.Set () ;
                                               return IntPtr.Zero ;
                                           } ;

            bool loopHasStopped = false ;

            Task.Factory.StartNew (() =>
                                   {
                                       messagePump.StartMessageLoop () ;
                                       loopHasStopped = true ;
                                   }) ;

            Thread.Sleep(1000);

            SendMessage (messagePump.WindowHandle, WM_APP, UIntPtr.Zero, IntPtr.Zero) ;
            var result = loopHasStarted.Wait (5000) ;
            Assert.AreNotEqual(result, false, "Timeout: a message hasn't arrived.");

            SendMessage (messagePump.WindowHandle, WM_APP + 1, UIntPtr.Zero, IntPtr.Zero);
            SendMessage (messagePump.WindowHandle, WM_APP + 2, UIntPtr.Zero, IntPtr.Zero);

            Assert.AreNotEqual(loopHasStopped, true, "The message loop should not be stopped until we call StopMessageLoop().");

            messagePump.StopMessageLoop();
            Thread.Sleep(3000);

            Assert.AreEqual (loopHasStopped, true, "The message loop should be stopped since we have called StopMessageLoop().");

            var numberOfAppMessages =
                messageList.Count (
                                   message =>
                                       message.Message == WM_APP ||
                                       message.Message == WM_APP + 1 ||
                                       message.Message == WM_APP + 2) ;
            Assert.AreEqual (numberOfAppMessages, 3, "There should be three WM_APP + x messages in the list.");
        }
    }
}
