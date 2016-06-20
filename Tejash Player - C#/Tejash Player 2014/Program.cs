using System;
using System.IO;
using System.Linq;
using System.Data;
using System.Text;
using SingletonApp;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;
using System.ComponentModel;
using System.Collections.Generic;

namespace Tejash_Player_2014
{
    static class Program
    { 
        static private mainDialog mainform;
        public static void WriteArgs(string[] args)
        {
            int index = 0;
            string appdata;
            appdata = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
            appdata = appdata + "\\Tejash Player\\temp.tpp";
            IniFile ini = new IniFile(appdata);
            if (File.Exists(appdata))
            {
                string t = ini.IniReadValue("Total_Files", "Total");
                index = Convert.ToInt32(t);
            }
            foreach (string file in args)
            {
                index++;
                ini.IniWriteValue("songs", index.ToString(), file);
            }
            ini.IniWriteValue("Total_Files", "Total", index.ToString());
        }

        static void myReceive(string[] args)
        {
            WriteArgs(args);
            mainform.LoadOtherInstanceArgs();
        }

        [STAThread]
        static void Main(string[] args)
        {
            string[] arg = args.Distinct().ToArray();
            if (SingletonController.IamFirst(new SingletonController.ReceiveDelegate(myReceive)))
            {
                // This is first instance
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);
                mainform = new mainDialog(arg);
                Application.Run(mainform);
            }
            else
            {
                SingletonController.Send(arg);
            }

            SingletonController.Cleanup();
        }
    }
}
