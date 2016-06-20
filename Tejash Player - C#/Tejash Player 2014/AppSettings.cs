using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using Tejash_Player_2014.Properties;


namespace Tejash_Player_2014
{
    class AppSettings
    {
        public string appdata_folder;
        public string playerName = "Tejash Player Home Edition";
        public bool Reg = false;
        public string RegName;
        Core core = new Core();

        public AppSettings()
        {
            appdata_folder = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
           // MessageBox.Show(appdata_folder);
            if (Directory.Exists(appdata_folder + "\\Tejash Player") == false)
            {
                Directory.CreateDirectory(appdata_folder + "\\Tejash Player");
                appdata_folder = appdata_folder + "\\Tejash Player";
                Settings.Default.FirstRun = true;
                Settings.Default.Save();
            }
            else
                appdata_folder = appdata_folder + "\\Tejash Player";

            if (Directory.Exists(appdata_folder) == false)
                MessageBox.Show("Unable to locate the temporary data folder!\nSome features may not work properly");

            if (File.Exists(appdata_folder + "\\TejashPlayer.DLL"))
            {
                IniFile ini = new IniFile(appdata_folder + "\\TejashPlayer.DLL");
                string key = ini.IniReadValue("RegInfo", "Key");
                if (key.Trim().Length > 0)
                {
                    if (key.Length == 29)
                    {
                        if (core.CheckKey(key,appdata_folder))
                        {
                            Reg = true;
                            RegName = ini.IniReadValue("RegInfo", "Name");
                            playerName = "Tejash Player Pro";
                        }
                    }
                }
            }
        }

        ~AppSettings()
        {
        }
    }
}
