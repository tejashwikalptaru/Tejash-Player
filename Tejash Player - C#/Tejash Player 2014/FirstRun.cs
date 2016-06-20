using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Tejash_Player_2014.Properties;

namespace Tejash_Player_2014
{
    public partial class FirstRun : Form
    {
        int count = 1;
        public FirstRun()
        {
            InitializeComponent();
        }

        private void Switch(int ch)
        {
            switch (count)
            {
                case 1:
                    pictureBox3.Image = Properties.Resources.welcome_1;
                    textBox1.Text = "Welcome to the all new Tejash Player" + Environment.NewLine + "Now enjoy crisp, clear and amplified music with inbuilt SRS WOW True HD sound effect*" + Environment.NewLine + "Tejash Player is built to perform and an all-in-one solution for all music needs, rip, record, burn, share and your effects" + Environment.NewLine + "With all this, we are ready to help you, need something? Write down about your need and in next update you will see your demand in your player";
                    break;
                case 2:
                    pictureBox3.Image = Properties.Resources.welcome_2;
                    textBox1.Text = "Think!" + Environment.NewLine + "Yes, your new player is a result of our think, while thinking we keep in mind your demand, ease of use and stability" + Environment.NewLine + "Enjoy unmatched quality, support and ease of use which is not provided by any other player in the current market*";
                    break;
                case 3:
                    pictureBox3.Image = Properties.Resources.welcome_3;
                    textBox1.Text = "Now enjoy True SRS WOW HD Sound effect with the blend of Beats Audio" + Environment.NewLine + "Tejash Player have inbuilt driver support for true HD 7.1 effect and gives crisp clear sound with Beats Audio";
                    break;
                case 4:
                    pictureBox3.Image = Properties.Resources.welcome_4;
                    textBox1.Text = "With effective ease of use interface of your new player, you will never be confused" + Environment.NewLine + "As all the regular media players have nasty and conflicting interface or shortcuts, and you often gets confused" + Environment.NewLine + "With the new concept of GUI and Shortcuts enjoy pure music with just few clicks";
                    break;
                case 5:
                    pictureBox3.Image = Properties.Resources.welcome_5;
                    textBox1.Text = "Now with built-in music organizer, keep your collection without having tension to organize them" + Environment.NewLine + "Throw your files to Tejash Player and it will organize your collection according to Album or Genre as per your choice. So from now, say bye to unmanaged large music collection";
                    break;
                case 6:
                    pictureBox3.Image = Properties.Resources.welcome_6;
                    textBox1.Text = "Need a help?" + Environment.NewLine + "We are ready to assist you, contact to our helpdesk at helpdesk@techtejash.com." + Environment.NewLine + "Thank you for using Tejash Player, we hope you find this unique!";
                    break;
            }
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {
            if (count < 6)
            {
                count++;
                Switch(count);
            }
        }

        private void pictureBox2_Click(object sender, EventArgs e)
        {
            if (count > 1)
            {
                count--;
                Switch(count);
            }
        }

        private void FirstRun_Load(object sender, EventArgs e)
        {
           // Handle = 273, w = 492
            textBox1.GotFocus +=new EventHandler(textBox1_GotFocus);
            this.TopMost = true;
            pictureBox3.Image = Properties.Resources.welcome_1;
            textBox1.Text = "Welcome to the all new Tejash Player" + Environment.NewLine + "Now enjoy crisp, clear and amplified music with inbuilt SRS WOW True HD sound effect*" + Environment.NewLine + "Tejash Player is built to perform and an all-in-one solution for all music needs, rip, record, burn, share and your effects" + Environment.NewLine + "With all this, we are ready to help you, need something? Write down about your need and in next update you will see your demand in your player";
        }

        private void textBox1_GotFocus(object sender, System.EventArgs e)
        {
            pictureBox3.Focus();
        }
    }
}
