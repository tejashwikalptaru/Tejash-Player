using System;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using System.Runtime.Remoting;
using System.Collections.Generic;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting.Channels.Tcp;

namespace SingletonApp
{
    [Serializable]
    class SingletonController : MarshalByRefObject
    {
        private static TcpChannel m_TCPChannel = null;
        private static Mutex m_Mutex = null;

        public delegate void ReceiveDelegate(string[] args);

        static private ReceiveDelegate m_Receive = null;
        static public ReceiveDelegate Receiver
        {
            get
            {
                return m_Receive;
            }
            set
            {
                m_Receive = value;
            }
        }

        public static bool IamFirst(ReceiveDelegate r)
        {
            if (IamFirst())
            {
                Receiver += r;
                return true;
            }
            else
            {
                return false;
            }
        }

        public static bool IamFirst()
        {
            string m_UniqueIdentifier = "TechTejash-TejashPlayer-8F97C341-C049-46C0-AE43-96D8F6002F09";

            m_Mutex = new Mutex(false, m_UniqueIdentifier);

            if (m_Mutex.WaitOne(1, true))
            {
                //We locked it! We are the first instance!!!    
                CreateInstanceChannel();
                return true;
            }
            else
            {
                //Not the first instance!!!
                m_Mutex.Close();
                m_Mutex = null;
                return false;
            }
        }

        private static void CreateInstanceChannel()
        {
            m_TCPChannel = new TcpChannel(8888);
            ChannelServices.RegisterChannel(m_TCPChannel, false);
            RemotingConfiguration.RegisterWellKnownServiceType(
                Type.GetType("SingletonApp.SingletonController"),
                "SingletonController",
                WellKnownObjectMode.SingleCall);
        }

        public static void Cleanup()
        {
            if (m_Mutex != null)
            {
                m_Mutex.Close();
            }

            if (m_TCPChannel != null)
            {
                m_TCPChannel.StopListening(null);
            }

            m_Mutex = null;
            m_TCPChannel = null;
        }

        public static void Send(string[] s)
        {
            SingletonController ctrl;
            TcpChannel channel = new TcpChannel();
            ChannelServices.RegisterChannel(channel, false);
            try
            {
                ctrl = (SingletonController)Activator.GetObject(typeof(SingletonController), "tcp://localhost:8888/SingletonController");
            }
            catch (Exception e)
            {
                MessageBox.Show("SINGELTON:Exception: " + e.Message);
                throw;
            }
            ctrl.Receive(s);
        }

        public void Receive(string[] s)
        {
            if (m_Receive != null)
            {
                m_Receive(s);
            }
        }
    }
}
