using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Tejash_Player_2014.Properties;

namespace Tejash_Player_2014
{
    public partial class AboutBox : Form
    {
        public string pro = "Tejash Player v5.0 Aplha 1 test preview [Pro Version]";
        public string free = "Tejash Player v5.0 Alpha 1 test preview [Free Version]";
        public string free2 = Environment.NewLine + "For non commercial use only." + Environment.NewLine + "Get a donated commercial version for $5";
        public AboutBox()
        {
            InitializeComponent();
        }

        private void AboutBox_Load(object sender, EventArgs e)
        {
            AppSettings settings = new AppSettings();
            if (settings.Reg)
            {
                // kick a dual check here :)
                RegisteredToLabel.Text = "Registered to: " + settings.RegName;
                string text = pro + Environment.NewLine + richTextBox1.Text;
                richTextBox1.Text = text;
            }
            else
            {
                string text = free + Environment.NewLine + richTextBox1.Text + free2;
                richTextBox1.Text = text;
            }
            pictureBox1.Image = Properties.Resources.logo;
            linkLabel1.LinkClicked +=new LinkLabelLinkClickedEventHandler(linkLabel1_LinkClicked);
            richTextBox1.GotFocus += new EventHandler(richTextBox1_GotFocus);
            richTextBox1.LinkClicked +=new LinkClickedEventHandler(richTextBox1_LinkClicked);
        }

        private void linkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            linkLabel1.LinkVisited = true;
            System.Diagnostics.Process.Start("http://www.tejashwi.com/techtejash/tejashplayer.html"); 
        }

        private void richTextBox1_GotFocus(object sender, System.EventArgs e)
        {
            System.Windows.Forms.SendKeys.Send("{tab}");
        }

        private void richTextBox1_LinkClicked(object sender, LinkClickedEventArgs e)
        {
            string text = richTextBox1.Text;
            System.Diagnostics.Process.Start(e.LinkText);
            richTextBox1.Text = text;
        }
    }
}
