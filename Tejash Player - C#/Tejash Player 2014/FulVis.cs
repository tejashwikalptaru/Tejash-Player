using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Tejash_Player_2014
{
    public partial class FulVis : Form
    {
        public mainDialog main;
        public FulVis(mainDialog m)
        {
            InitializeComponent();
            main = m;
        }

        private void FulVis_Load(object sender, EventArgs e)
        {
            this.ContextMenuStrip = main.coverContext;
            if (main.visual)
            {
                main.VisualStart(main.visualpath);
            }
            else
                this.BackgroundImage = main.coverImage.Image;
        }

        private void FulVis_DoubleClick(object sender, EventArgs e)
        {
            main.full_screen = false;
            if (main.visual)
                main.VisualStart(main.visualpath);
            this.Close();
        }

        private void FulVis_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Escape || e.KeyCode == Keys.F)
                FulVis_DoubleClick(sender, e);
            if (e.KeyCode == Keys.MediaNextTrack)
                main.btnNext_Click(sender, e);
            if (e.KeyCode == Keys.MediaPreviousTrack)
                main.btnPrev_Click(sender, e);
            if (e.KeyCode == Keys.MediaPlayPause)
                main.btnPlay_Click(sender, e);
            if (e.KeyCode == Keys.MediaStop)
                main.btnStop_Click(sender, e);
        }
    }
}
