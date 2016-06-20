using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Un4seen.Bass;

namespace Tejash_Player_2014
{
    public partial class SndDevices : Form
    {
        public CorePlayerEngine engine;
        public mainDialog main;
        public SndDevices(CorePlayerEngine e, mainDialog m)
        {
            InitializeComponent();
            engine = e;
            main = m;
            richTextBox1.GotFocus += new EventHandler(richTextBox1_GotFocus);
            comboBox1.DropDownStyle = ComboBoxStyle.DropDownList;

            RefreshDevices();
        }

        void richTextBox1_GotFocus(object sender, EventArgs e)
        {
            System.Windows.Forms.SendKeys.Send("{tab}");
        }

        public void RefreshDevices()
        {
            int a = 0, count = 0;
            BASS_DEVICEINFO info = new BASS_DEVICEINFO();
            GetDeviceData(Bass.BASS_GetDevice()); // Get current device id and pass to function
            comboBox1.Items.Clear();
            for (a = 0; Bass.BASS_GetDeviceInfo(a, info); a++)
            {
                if (info.IsEnabled) // if device is enabled
                {
                    count++;
                    comboBox1.Items.Add(info.name);
                }
            }
            comboBox1.SelectedIndex = Bass.BASS_GetDevice();
        }

        public void GetDeviceData(int Id) // this function is a part of Tejash Player v5 coded in C, converted to c# to use here
        {
            string details;
            BASS_INFO dinfo;
            BASS_DEVICEINFO info;
            info = Bass.BASS_GetDeviceInfo(Id);
            dinfo = Bass.BASS_GetInfo();
            details = "Current Device Details:\n\nName: " + info.name + "\nDriver: " + info.driver + "\nTotal H/W Memory: " + dinfo.hwsize.ToString() + "\nFree H/W Memory: " + dinfo.hwfree.ToString() + "\nMinimum Supported Rate: " + dinfo.minrate.ToString() + "\nMaximum Supported Rate: " + dinfo.maxrate.ToString() + "\nDirect Sound Version: " + dinfo.dsver.ToString() + "\nNumber of Speaker: " + dinfo.speakers.ToString() + "\nCurrent Sample Rate: " + dinfo.freq.ToString() + "\nDevice ID: " + Id.ToString();
            richTextBox1.Text = details;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            RefreshDevices();
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            GetDeviceData(comboBox1.SelectedIndex);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            main.SetEngineObject(comboBox1.SelectedIndex);
            this.Close();
        }
    }
}
