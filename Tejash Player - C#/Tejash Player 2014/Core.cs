using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using TechTejash;
using Security;
using System.Threading;
using System.IO;
using System.Security.Cryptography;
using System.Reflection;

namespace Tejash_Player_2014
{
    class Core
    {
        private string key = "DDDEDC1CAEDFC5DDA6B583DB5C2B38B0";
        string lic_loc = null;
        Thread fp_check;
        TechTejashLicense lic = new TechTejashLicense();
        private MD5 md5;
        public Core()
        {
            md5 = MD5.Create();
            string engine = "44ADF191FBFD055F67BA76A401D22E5F";
            string TechTejash = "0CC5E55CC7176748C8DD6DC6459125E6";
            string path_engine = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location) + "\\Engine.dll";
            string path_TechTejash = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location) + "\\TechTejash.dll";
            if (engine == CalculateChecksum(path_engine))
            {
                if (TechTejash != CalculateChecksum(path_TechTejash))
                {
                    MessageBox.Show("Internal files are corrupted!\nSorry, but please reinstall player to solve this issue", "Unable to run!", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    Application.Exit();
                }
            }
            else
            {
                MessageBox.Show("Internal files are corrupted!\nSorry, but please reinstall player to solve this issue", "Unable to run!", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Application.Exit();
            }
        }
        private string CalculateChecksum(string file)
        {
            using (FileStream stream = File.OpenRead(file))
            {
                byte[] checksum = md5.ComputeHash(stream);
                return (BitConverter.ToString(checksum).Replace("-", string.Empty));
            }
        }
        public bool IsEmailValid(string email)
        {
            bool ret = false;
            if (email.Contains("@"))
            {
                if (email.Contains("."))
                    ret = true;
            }
            return ret;
        }

        static class Engine
        {
            [DllImport("Engine.dll",CallingConvention = CallingConvention.Cdecl)]
            public static extern int StartEngine(String key, String hash);
        }

        public void SaveHash(string key,string loc)
        {
            IniFile ini = new IniFile(loc);
            string id = FingerPrint.Value();
            ini.IniWriteValue("FingerPrint", "ID", id);
        }

        public void FingerPrintCheck()
        {
            IniFile ini = new IniFile(lic_loc);
            string id = ini.IniReadValue("FingerPrint", "ID");
            if (id != FingerPrint.Value())
            {
                MessageBox.Show("Tejash Player detected another hardware rather it was licensed on.\nKindly re-enter your registration details and Serial number\n\nPlayer will now restart...", "Hardware not licensed", MessageBoxButtons.OK, MessageBoxIcon.Error);
                System.IO.File.Delete(lic_loc);
                Application.Restart();
            }
            else
            {
                try { fp_check.Abort(); }
                catch { }
            }
        }

        public bool CheckKey(string s,string path)
        {
            if (Part1Valid(s))
            {
                lic_loc = path + "\\TejashPlayer.DLL";
                fp_check = new Thread(new ThreadStart(FingerPrintCheck));
                fp_check.Start();
                return true;
            }
            else
                return false;
        }

        public bool CheckSerialNumber(string part1, string part2)
        {
            bool ret;
            int val = 1;
            if (Part1Valid(part1))
            {
                try
                {
                    val = Engine.StartEngine(part2, key);
                    if (val==0)
                        ret = true;
                    else
                        ret = false;
                }
                catch { ret = false; }
            }
            else
                ret = false;

            return ret;
        }

        public bool Part1Valid(string part1)
        {
            bool ret;
            if (lic.VerifyCode(part1))
                ret = true;
            else
                ret = false;
            return ret;
        }
    }
}
