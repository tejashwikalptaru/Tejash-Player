using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Security.Cryptography;

namespace Tejash_Player_2014
{
    public partial class Registration : Form
    {
        Core core = new Core();
        AppSettings settings = new AppSettings();
        public Registration()
        {
            InitializeComponent();
        }

        private void glassButton1_Click(object sender, EventArgs e)
        {
            string[] s = null;
            if (textBox1.Text.Trim().Length > 2)
            {
                if (textBox2.Text.Trim().Length > 4)
                {
                    if (core.IsEmailValid(textBox2.Text))
                    {
                        if (textBox3.Text.Trim().Length > 4)
                        {
                            try
                            {
                                s = textBox3.Text.Split('+');
                                if (s.Length == 2)
                                {
                                    if (core.CheckSerialNumber(s[0], s[1]))
                                    {
                                        IniFile ini = new IniFile(settings.appdata_folder + "\\TejashPlayer.DLL");
                                        ini.IniWriteValue("RegInfo", "Name", textBox1.Text);
                                        ini.IniWriteValue("RegInfo", "EMAIL", textBox2.Text);
                                        ini.IniWriteValue("RegInfo", "Key", s[0]);
                                        core.SaveHash(s[0],settings.appdata_folder + "\\TejashPlayer.DLL");
                                        MessageBox.Show("Thank you for your registration, Your product is now unlocked\nPlayer will restart to update itself", "Tejash Player Pro", MessageBoxButtons.OK, MessageBoxIcon.Information);
                                        Application.Restart();
                                    }
                                    else
                                        MessageBox.Show("Invalid Serial Number!\nMake sure you copy and paste your serial number same as provided you in your email", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                                }
                                else
                                    MessageBox.Show("Invalid Serial Number!\nMake sure you copy and paste your serial number same as provided you in your email", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            }
                            catch 
                            {
                                MessageBox.Show("Invalid Serial Number!\nMake sure you copy and paste your serial number same as provided you in your email", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            }
                        }
                        else
                            MessageBox.Show("Invalid Serial Number!\nMake sure you copy and paste your serial number same as provided you in your email", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                    else
                        MessageBox.Show("Invalid EMAIL ID!\nPlease enter a valid EMAIL", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
                else
                    MessageBox.Show("Invalid EMAIL ID!\nPlease enter a valid EMAIL", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            else
                MessageBox.Show("Invalid Name Format!\nPlease enter your full name", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        private void glassButton2_Click(object sender, EventArgs e)
        {

        }
    }
}
