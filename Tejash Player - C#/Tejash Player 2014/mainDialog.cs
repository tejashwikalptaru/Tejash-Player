#region INCLUDES
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Reflection;
using System.Text;
using System.Windows.Forms;
using Tejash_Player_2014.Properties;
using System.Threading;
using System.Runtime.InteropServices;
using System.Timers;
using System.Collections;
using System.Net.NetworkInformation;
using System.Runtime.ExceptionServices;
using System.Linq;

using InTheHand.Net;
using InTheHand.Net.Sockets;
using InTheHand.Net.Bluetooth;
using InTheHand.Windows.Forms;

using SkinFramework;

using Microsoft.WindowsAPICodePack.Taskbar;
using Microsoft.WindowsAPICodePack.Shell;

using MovablePython;

using Un4seen.Bass;

#endregion

namespace Tejash_Player_2014
{
    public partial class mainDialog : Form
    {
        #region VARS
        CorePlayerEngine PlayerEngine;
        AppSettings settings;
        Thread lib_temp_loader;
        private ListViewColumnSorter lvwColumnSorter;
        string CurrentPlaylistLocation;
        SelectBluetoothDeviceDialog sbdd; // bluetooth select dialog
        ObexWebRequest request; // bluetooth request
        public Thread thrSend; // thread to send files via bluetooth
        public Thread thrSearch; // thread to search files
        public string PathToSearch;
        public string buffer; // Holds temp data
        public bool MakeNewPlayList;
        public Thread wm_copydata_parser;
        public string[] arguments;
        public bool load_args = false;
        public Thread thrargloader;
        public int ToIgnore = 0;
        public System.Timers.Timer seekTimer;
        public int skin = 1;
        TaskbarManager tbManager = TaskbarManager.Instance;
        ThumbnailToolBarButton play,next,prev,hide;
        Hotkey playhotkey, prevhotkey, nexthotkey, stophotkey, hidehotkey;
        Thread load_vis;
        LastFM_Scrobbling lpfm;
        public bool use_lastfm = false;
        public bool notify_sounds = false;
        public bool full_screen = false;
        public FulVis fulvis;
        public Thread temp;
#endregion
        #region BASS_SFX
        [DllImport("bass_sfx.dll")]
        public static extern bool BASS_SFX_Init(IntPtr hInstance, IntPtr hWnd);

        [DllImport("bass_sfx.dll")]
        public static extern int BASS_SFX_PluginCreate(string file, IntPtr hPluginWnd, int width, int height, int flags);

        [DllImport("bass_sfx.dll")]
        public static extern string BASS_SFX_PluginGetName(int handle);

        [DllImport("bass_sfx.dll")]
        public static extern bool BASS_SFX_PluginStart(int handle);

        [DllImport("bass_sfx.dll")]
        public static extern bool BASS_SFX_PluginStop(int handle);

        [DllImport("bass_sfx.dll")]
        public static extern bool BASS_SFX_PluginFree(int handle);

        [DllImport("bass_sfx.dll")]
        public static extern bool BASS_SFX_PluginSetStream(int handle, int stream);

        [DllImport("bass_sfx.dll")]
        public static extern bool BASS_SFX_PluginResize(int handle, int width, int height);

        [DllImport("bass_sfx.dll")]
        public static extern IntPtr BASS_SFX_PluginRender(int handle, int hStream, IntPtr hDC);

        [DllImport("bass_sfx.dll")]
        public static extern bool BASS_SFX_Free();

        [DllImport("User32.dll")]
        public static extern IntPtr GetDC(IntPtr hWnd);

        [DllImport("User32.dll")]
        public static extern Int32 ReleaseDC(IntPtr hWnd, IntPtr hDC);

        int hSFX = 0;
        public string visualpath;
        public bool visual = false;
        IntPtr hVisDC = IntPtr.Zero;

        #endregion
        #region SIZE_REMEMBER_AND_RESTORE
        public static void GeometryFromString(string thisWindowGeometry, Form formIn)
        {
            if (string.IsNullOrEmpty(thisWindowGeometry) == true)
            {
                return;
            }
            string[] numbers = thisWindowGeometry.Split('|');
            string windowString = numbers[4];
            if (windowString == "Normal")
            {
                Point windowPoint = new Point(int.Parse(numbers[0]),
                    int.Parse(numbers[1]));
                Size windowSize = new Size(int.Parse(numbers[2]),
                    int.Parse(numbers[3]));

                bool locOkay = GeometryIsBizarreLocation(windowPoint, windowSize);
                bool sizeOkay = GeometryIsBizarreSize(windowSize);

                if (locOkay == true && sizeOkay == true)
                {
                    formIn.Location = windowPoint;
                    formIn.Size = windowSize;
                    formIn.StartPosition = FormStartPosition.Manual;
                    formIn.WindowState = FormWindowState.Normal;
                }
                else if (sizeOkay == true)
                {
                    formIn.Size = windowSize;
                }
            }
            else if (windowString == "Maximized")
            {
                formIn.Location = new Point(100, 100);
                formIn.StartPosition = FormStartPosition.Manual;
                formIn.WindowState = FormWindowState.Maximized;
            }
        }

        private static bool GeometryIsBizarreLocation(Point loc, Size size)
        {
            bool locOkay;
            if (loc.X < 0 || loc.Y < 0)
            {
                locOkay = false;
            }
            else if (loc.X + size.Width > Screen.PrimaryScreen.WorkingArea.Width)
            {
                locOkay = false;
            }
            else if (loc.Y + size.Height > Screen.PrimaryScreen.WorkingArea.Height)
            {
                locOkay = false;
            }
            else
            {
                locOkay = true;
            }
            return locOkay;
        }

        private static bool GeometryIsBizarreSize(Size size)
        {
            return (size.Height <= Screen.PrimaryScreen.WorkingArea.Height &&
                size.Width <= Screen.PrimaryScreen.WorkingArea.Width);
        }

        public static string GeometryToString(Form mainForm)
        {
            return mainForm.Location.X.ToString() + "|" +
                mainForm.Location.Y.ToString() + "|" +
                mainForm.Size.Width.ToString() + "|" +
                mainForm.Size.Height.ToString() + "|" +
                mainForm.WindowState.ToString();
        }

        #endregion

        public void LoadOtherInstanceArgs()
        {
            try
            {
                if (wm_copydata_parser.IsAlive)
                    return;
            }
            catch { }
            wm_copydata_parser = new Thread(new ThreadStart(ParseTempList));
            wm_copydata_parser.Start();
        }

        public mainDialog(string[] args)
        {
            InitializeComponent();

            this.ResizeRedraw = true;
            this.DoubleBuffered = true;
            lvwColumnSorter = new ListViewColumnSorter();
            this.listView1.ListViewItemSorter = lvwColumnSorter;
            CurrentPlaylistLocation = "";
            seekTimer = new System.Timers.Timer();
            seekTimer.Interval = 50;
            seekTimer.Elapsed += new ElapsedEventHandler(seekTimer_Tick);
            if (args != null)
            {
                if (args.Length > 0)
                {
                    load_args = true;
                    arguments = new string[args.Length];
                    args.CopyTo(arguments, 0);
                }
            }
        }

        public void AskedPlay() // Always call this when song changed or loaded
        {
            if (PlayerEngine.isPlaying)
            {
                string name = "";
                string artist = "";
                string album = "";
                PlayerEngine.GetTrackData(PlayerEngine.current_track, out name, out artist, out album);
                SongName_Label.Text = name;
                artist_label.Text = artist;
                Track_Album.Text = album;
                if (System.Net.NetworkInformation.NetworkInterface.GetIsNetworkAvailable())
                {
                    try
                    {
                        if (lpfm.isReady)
                        {
                            lpfm.Scrobbling(name, album, artist, PlayerEngine.GetTrackTotalTime());
                            lastfm_pic.Visible = true;
                        }
                        else
                            lastfm_pic.Visible = false;
                    }
                    catch { use_lastfm = false; lastfm_pic.Visible = false; }
                }
                else
                    lastfm_pic.Visible = false;

                if (PlayerEngine.ShowCover == true)
                {
                    if (PlayerEngine.image == null)
                        coverImage.Image = Properties.Resources.no_art;
                    else
                        coverImage.Image = PlayerEngine.image;
                    visual = false;
                }
                else
                    coverImage.Image = Properties.Resources.art;
                if (full_screen)
                    fulvis.BackgroundImage = coverImage.Image;

                try
                {
                    seekBar.Maximum = (int)PlayerEngine.GetTrackTotalTime();
                    seekTimer.Enabled = true;
                }
                catch { }

                btnPlay.Image = Properties.Resources.Pause.GetThumbnailImage(16, 16, null, new IntPtr());
                if(visual == true)
                {
                    // Realign visual to newly played song
                    if (visualpath != null)
                    {
                        if (!full_screen)
                        {
                            ReleaseDC(coverImage.Handle, hVisDC);
                            hVisDC = GetDC(coverImage.Handle);
                            hSFX = BASS_SFX_PluginCreate(visualpath, coverImage.Handle, coverImage.Width, coverImage.Height, 0);
                        }
                        else
                        {
                            ReleaseDC(fulvis.Handle, hVisDC);
                            hVisDC = GetDC(fulvis.Handle);
                            hSFX = BASS_SFX_PluginCreate(visualpath, fulvis.Handle, fulvis.Width, fulvis.Height, 0);
                        }

                        BASS_SFX_PluginSetStream(hSFX, PlayerEngine.channel);
                        BASS_SFX_PluginStart(hSFX);
                        visTimer.Interval = 40;
                        visTimer.Enabled = true;
                        visual = true;
                        PlayerEngine.ShowCover = false;
                    }
                    else
                    {
                        visual = false;
                        visTimer.Enabled = false;
                    }
                }

            }
            else
            {
                btnPlay.Image = Properties.Resources.Play.GetThumbnailImage(16, 16, null, new IntPtr());
                seekTimer.Enabled = false;
                TrackTime_label.Text = "00:00 - 00:00";
                seekBar.Value = 0;
            }
        }

        protected override void OnSizeChanged(EventArgs e)
        {
            if (this.Handle != null)
            {
                this.BeginInvoke((MethodInvoker)delegate
                {
                    base.OnSizeChanged(e);
                });
            }
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            e.Graphics.Clear(SystemColors.Control);
        }

        private void mainDialog_Load(object sender, EventArgs e)
        {
            menuStrip1.Visible = true;
            menuStrip2.Visible = true;
            menuStrip3.Visible = true;
            menuStrip4.Visible = true;

            settings = new AppSettings();
            PlayerEngine = new CorePlayerEngine(this.Handle,-1);
            
            SongName_Label.Text = settings.playerName;
            if (settings.Reg)
            {
                // Set a kick here to dual check license..., if net check on server too...
                searchToolStripMenuItem.Enabled = false;
            }

            if (!BASS_SFX_Init(System.Diagnostics.Process.GetCurrentProcess().Handle, this.Handle))
            {
                MessageBox.Show("Unable to load visualization loader library", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Application.Exit();
            }
            else
                hVisDC = GetDC(coverImage.Handle);

            coverImage.Image = Properties.Resources.art;

            this.KeyDown += new KeyEventHandler(Form_KeyDown);
            this.FormClosing += new FormClosingEventHandler(mainDialog_FormClosing);
            this.listView1.DoubleClick += new EventHandler(listView1_DoubleClick);
            this.listView1.ColumnClick += new ColumnClickEventHandler(listView1_ColumnClick);
            this.textBox1.TextChanged += new EventHandler(textBox1_TextChanged);

            this.sysTrayIcon.BalloonTipIcon = System.Windows.Forms.ToolTipIcon.Info;
            this.sysTrayIcon.ContextMenuStrip = this.systemTrayMenu;
            this.sysTrayIcon.Icon = Properties.Resources.icon;
            this.sysTrayIcon.Visible = true;
            sysTrayIcon.Text = settings.playerName;
            sysTrayIcon.BalloonTipText = settings.playerName;
            sysTrayIcon.BalloonTipTitle = "Welcome";
            sysTrayIcon.ShowBalloonTip(1);


            btnPlay.Image = Properties.Resources.Play.GetThumbnailImage(16, 16, null, new IntPtr());
            btnStop.Image = Properties.Resources.Stop.GetThumbnailImage(16, 16, null, new IntPtr());
            btnPrev.Image = Properties.Resources.Prev.GetThumbnailImage(16, 16, null, new IntPtr());
            btnNext.Image = Properties.Resources.Next.GetThumbnailImage(16, 16, null, new IntPtr());
            btnBluetoothShare.Image = Properties.Resources.Bluetooth_icon.GetThumbnailImage(16, 16, null, new IntPtr());

            seekBar.Minimum = 0;
            seekBar.Maximum = 100;
            seekBar.Value = 0;

            volSlider.Minimum = 0;
            volSlider.Maximum = 100;
            volSlider.Value = 100;

            CheckForIllegalCrossThreadCalls = false;

            progressBar1.Visible = false;

            LoadSettings();
            if (System.Net.NetworkInformation.NetworkInterface.GetIsNetworkAvailable())
            {
                if (use_lastfm)
                {
                    lpfm = new LastFM_Scrobbling(settings.appdata_folder + "\\TejashPlayer.DLL");
                    lastfm_pic.Visible = true;
                }
                else
                    lastfm_pic.Visible = false;
            }
            else
                lastfm_pic.Visible = false;

            NetworkChange.NetworkAvailabilityChanged += AvalibilityChange;

            // Load if the arguments passed
            if (load_args)
            {
                thrargloader = new Thread(new ThreadStart(ArgLoader));
                thrargloader.Start();
            }

            lib_temp_loader = new Thread(new ThreadStart(LoadLibraryTemp));
            lib_temp_loader.Start();

            GeometryFromString(Properties.Settings.Default.WindowGeometry, this);

            SetSkin(Properties.Settings.Default.skin);

            CreateThumbnailButtons();

            RegisterHotKeys();

            load_vis = new Thread(new ThreadStart(LoadVisuals));
            load_vis.Start();

         //   wm_copydata_parser = new Thread(new ThreadStart(ParseTempList));
          //  wm_copydata_parser.Start();
        }

        private void AvalibilityChange(object sender, NetworkAvailabilityEventArgs e)
        {
            if (e.IsAvailable)
            {
                if (PlayerEngine.isPlaying)
                {
                    if (use_lastfm)
                    {
                        string name = "";
                        string artist = "";
                        string album = "";
                        PlayerEngine.GetTrackData(PlayerEngine.current_track, out name, out artist, out album);
                        SongName_Label.Text = name;
                        artist_label.Text = artist;
                        Track_Album.Text = album;
                        try
                        {
                            if (lpfm.isReady)
                            {
                                lpfm.Scrobbling(name, album, artist, PlayerEngine.GetTrackTotalTime());
                                lastfm_pic.Visible = true;
                            }
                        }
                        catch { use_lastfm = false; lastfm_pic.Visible = false; }
                    }
                }
            }
            else
            {
                if (use_lastfm)
                {
                    lastfm_pic.Visible = false;
                }
            }
        }

        private void SetSkin(int p)
        {
            switch (p)
            {
                case 1:
                    skinningManager1.DefaultSkin = DefaultSkin.Office2007Obsidian;
                    skin = 1;
                    blackToolStripMenuItem.Checked = true;
                    blueToolStripMenuItem.Checked = false;
                    silverToolStripMenuItem.Checked = false;
                    break;
                case 2:
                    skinningManager1.DefaultSkin = DefaultSkin.Office2007Luna;
                    skin = 2;
                    blackToolStripMenuItem.Checked = false;
                    blueToolStripMenuItem.Checked = true;
                    silverToolStripMenuItem.Checked = false;
                    break;
                case 3:
                    skinningManager1.DefaultSkin = DefaultSkin.Office2007Silver;
                    skin = 3;
                    blackToolStripMenuItem.Checked = false;
                    blueToolStripMenuItem.Checked = false;
                    silverToolStripMenuItem.Checked = true;
                    break;
            }
        }

        void Form_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Control && e.KeyCode == Keys.O)       // Ctrl-O Open file
            {
                openFileToolStripMenuItem_Click(sender, e);
                e.SuppressKeyPress = true;  // stops bing! also sets handeled which stop event bubbling
            }

            if (e.KeyCode == Keys.F) // toggle full screen visuals
            {
                if(tabControl1.SelectedTab == tabPage1)
                    coverImage_DoubleClick(sender, e);
            }
            if (e.KeyCode == Keys.MediaNextTrack)
                btnNext_Click(sender, e);
            if (e.KeyCode == Keys.MediaPreviousTrack)
                btnPrev_Click(sender, e);
            if (e.KeyCode == Keys.MediaPlayPause)
                btnPlay_Click(sender, e);
            if (e.KeyCode == Keys.MediaStop)
                btnStop_Click(sender, e);
        }

        private void openFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "Playable files|*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif;*.mo3;*.it;*.xm;*.s3m;*.mtm;*.mod;*.umx;*.cda;*.fla;*.flac;*.oga;*.ogg;*.wma;*.wv;*.aac;*.m4a;*.m4b;*.mp4;*.ac3;*.adx;*.aix;*.ape;*.mac;*.mpc;*mp+;*.mpp;*.ofr;*.ofs;*.tta";
            DialogResult result = openFileDialog1.ShowDialog();
            if (result == DialogResult.OK)
            {
                if (PlayerEngine.CanIPlay(openFileDialog1.FileName))
                {
                    AddToPlaylist(openFileDialog1.FileName, false,false,"");
                    PlayNewSong(openFileDialog1.FileName);
                }
                else
                    MessageBox.Show("The file format: " + System.IO.Path.GetExtension(openFileDialog1.FileName) + " is not supported by player","Error",MessageBoxButtons.OK,MessageBoxIcon.Error);
            }
        }

        public void PlayNewSong(string path)
        {
            PlayerEngine.Player(path);
            AskedPlay();
            tbManager.SetProgressState(TaskbarProgressBarState.Normal);
        }

        private void aboutToolStripMenuItem2_Click(object sender, EventArgs e)
        {
            AboutBox ab = new AboutBox();
            ab.Show();
        }

        private void seekTimer_Tick(object sender, ElapsedEventArgs e)
        {
            if (PlayerEngine.isPlaying)
            {
                if (PlayerEngine.GetTrackCurrentTime() == PlayerEngine.GetTrackTotalTime()) // if equal, then current track ended
                {
                    seekTimer.Enabled = false;
                    TrackTime_label.Text = "00:00 - 00:00";
                    seekBar.Value = 0;
                    btnPlay.Image = Properties.Resources.Play.GetThumbnailImage(16, 16, null, new IntPtr());
                    //Check if there something other to play here.... :)
                    {
                        // Next songs checking
                        if (NextSong() == 1)
                        {
                            seekTimer.Enabled = false;
                            TrackTime_label.Text = "00:00 - 00:00";
                            seekBar.Value = 0;
                            btnPlay.Image = Properties.Resources.Play.GetThumbnailImage(16, 16, null, new IntPtr());
                        }
                        else
                            return;
                    }
                    return;
                }
                double cur_pos = PlayerEngine.GetTrackCurrentTime();
                try
                {
                    seekBar.Value = (int)cur_pos;
                    tbManager.SetProgressValue((int)cur_pos, (int)PlayerEngine.GetTrackTotalTime());
                }
                catch { }
                uint time = (uint)cur_pos;
                double total_pos = PlayerEngine.GetTrackTotalTime();
                uint total_t = (uint)total_pos;
                string cur_time = string.Format("{0:d}:{1:d2}", time / 60, time % 60);
                string total_time = string.Format("{0:d}:{1:d2}", total_t / 60, total_t % 60);
                TrackTime_label.Text = cur_time + " - " + total_time;
            }
            else
            {
                tbManager.SetProgressState(TaskbarProgressBarState.NoProgress);
                seekTimer.Enabled = false;
                TrackTime_label.Text = "00:00 - 00:00";
                seekBar.Value = 0;
            }
        }

        private void seekBar_Scroll(object sender, ScrollEventArgs e)
        {
            if (PlayerEngine.isPlaying)
            {
                seekTimer.Enabled = false;
                PlayerEngine.SetTrackPos(seekBar.Value);
                seekTimer.Enabled = true;
            }
        }

        private void volSlider_Scroll(object sender, ScrollEventArgs e)
        {
            PlayerEngine.SetVolume(volSlider.Value);
            PlayerEngine.mute = false;
            muteCheckBox.CheckState = CheckState.Unchecked;
        }

        [HandleProcessCorruptedStateExceptions] 
        private void mainDialog_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (visual)
            {
                try
                {
                    BASS_SFX_PluginStop(hSFX);
                    BASS_SFX_PluginFree(hSFX);
                    BASS_SFX_Free();
                }
                catch { }
            }
            sysTrayIcon.Dispose();

            Settings.Default.PlaylistLocation = CurrentPlaylistLocation;

            try
            {
                ReleaseDC(coverImage.Handle, hVisDC);
                ReleaseDC(fulvis.Handle, hVisDC);
            }
            catch { }

            try
            {
                if (lib_temp_loader.IsAlive)
                    lib_temp_loader.Abort();
                if (thrSend.IsAlive)
                    thrSend.Abort();
            }
            catch { }

            Unregisterkeyhot();

            Settings.Default.sounds = notify_sounds;

            Settings.Default.Volume = PlayerEngine.volume;

            Settings.Default.LPFM = use_lastfm;

            Settings.Default.FirstRun = false;

            Properties.Settings.Default.WindowGeometry = GeometryToString(this);

            Properties.Settings.Default.skin = skin;

            // Save settings
            Settings.Default.Save();
        }

        private void LoadSettings()
        {
            if (Settings.Default.FirstRun == true)
            {
                FirstRun frun = new FirstRun();
                frun.Show();
            }

            if (File.Exists(Settings.Default.PlaylistLocation))
                CurrentPlaylistLocation = Settings.Default.PlaylistLocation;
            else
                CurrentPlaylistLocation = settings.appdata_folder + "\\Tejash-Player-Playlist.tpp";

            if (Settings.Default.RememberVolume == true)
            {
                PlayerEngine.volume = Settings.Default.Volume;
                volSlider.Value = (int)PlayerEngine.volume;
                checkBox3.CheckState = CheckState.Checked;
            }
            else
                checkBox3.CheckState = CheckState.Unchecked;

            if (Settings.Default.LPFM == true)
            {
                checkBox4.Checked = true;
                use_lastfm = true;
            }
            else
            {
                checkBox4.Checked = false;
                use_lastfm = false;
            }

            if (Settings.Default.sounds == true)
            {
                notify_sounds = true;
                checkBox5.Checked = true;
            }
            else
            {
                notify_sounds = false;
                checkBox5.Checked = false;
            }
        }

        private void muteCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (muteCheckBox.CheckState == CheckState.Checked)
                PlayerEngine.Mute();
            else
                PlayerEngine.Mute();
        }

        private void aboutToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            AboutBox ab = new AboutBox();
            ab.Show();
        }

        public void LoadLibraryTemp()
        {
            try
            {
                while (thrargloader.IsAlive)
                    System.Threading.Thread.Sleep(100);
            }
            catch { }

            // Loading the temp playlist in thread worker :)
            ListViewItem lvi;
            ListViewItem.ListViewSubItem lvsi;
            string name = "";
            string album = "";
            string artist = "";
            string path = "";

            if (File.Exists(CurrentPlaylistLocation))
            {
                this.progressBar1.Visible = true;
                this.Text = "Tejash Player | Loading playlist";
                // Playlist is there, do action...
                IniFile ini = new IniFile(CurrentPlaylistLocation);
                string Total = ini.IniReadValue("Total_Files", "Total");
                int items = Convert.ToInt32(Total);
                if (ToIgnore > 0)
                {
                    items = items - ToIgnore;
                    ToIgnore = 0;
                }
                this.progressBar1.Minimum = 0;
                this.progressBar1.Maximum = items;
                this.progressBar1.MarqueeAnimationSpeed = 100;
                this.progressBar1.Style = ProgressBarStyle.Blocks;
                int total = 0;
                listView1.BeginUpdate();
                for (int i = 1; i <= items; i++)
                {
                    path = ini.IniReadValue("songs", Convert.ToString(i));
                    if(File.Exists(path))
                    {
                        if (System.IO.Path.GetExtension(path).ToLower() != ".ignore") // Check if a track in playlist is ignored
                        {
                            PlayerEngine.GetTrackData(path, out name, out artist, out album);

                            lvi = new ListViewItem();
                            lvi.Text = name;

                            lvsi = new ListViewItem.ListViewSubItem();
                            lvsi.Text = album;
                            lvi.SubItems.Add(lvsi);

                            lvsi = new ListViewItem.ListViewSubItem();
                            lvsi.Text = artist;
                            lvi.SubItems.Add(lvsi);

                            lvsi = new ListViewItem.ListViewSubItem();
                            lvsi.Text = path;
                            lvi.SubItems.Add(lvsi);

                            this.listView1.BeginUpdate();
                            this.listView1.Items.Add(lvi);
                            this.listView1.EndUpdate();

                            total++;
                            this.progressBar1.Value = (i);
                        }
                        else
                            MessageBox.Show(path);
                    }
                }
                listView1.EndUpdate();
                PlayerEngine.TotalTrackInTempPlaylist = listView1.Items.Count-1;
                this.Text = "Playlist loading completed! " + "Loaded: " + listView1.Items.Count.ToString() + " song(s)";
                Thread.Sleep(2000);
                this.progressBar1.Visible = false;
                this.Text = "Tejash Player";
                try { lib_temp_loader.Abort(); }
                catch { }
            }
            else
            {
                this.Text = "No playlist found!";
                Thread.Sleep(2000);
                this.progressBar1.Visible = false;
                this.Text = "Tejash Player";
             // MessageBox.Show("No playlist found!\nPlease create a new playlist from Music menu to play songs with ease.");
                try { lib_temp_loader.Abort(); }
                catch { }
            }
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            ListViewItem foundItem;
            foundItem = listView1.FindItemWithText(textBox1.Text);
            if (foundItem != null)
            {
                string name = foundItem.Text;
                if (name.ToLower().Contains(textBox1.Text.ToLower()))
                {
                    foundItem.EnsureVisible();
                    foundItem.Focused = true;
                    foundItem.Selected = true;
                }
            }
            else
            {
                listView1.SelectedItems.Clear();
            }
        }

        private void listView1_DoubleClick(object sender, EventArgs e)
        {
            ListView lv = (ListView)sender;
            if (lv.SelectedItems.Count > 0)
            {
                for (int i = 0; i < lv.Items.Count; i++)
                {
                    // is i the index of the row I selected?
                    if (lv.Items[i].Selected == true)
                    {
                        btnStop_Click(sender, e);
                        PlayerEngine.CurrentTrackIndex = i;
                        //MessageBox.Show("Index = " + Convert.ToString(i)); // Shows the index
                        //MessageBox.Show(lv.Items[i].SubItems[3].Text); // Shows the path
                        seekTimer.Stop();
                        PlayNewSong(lv.Items[i].SubItems[3].Text);
                        tbManager.SetProgressState(TaskbarProgressBarState.Normal);
                        seekTimer.Start();
                        break;
                    }
                }
            }
        }

        public int NextSong()
        {
            int i = 0;
            string path = "";
            ListView lv = listView1;
            if (lv.Items.Count > 0)
            {
                if (PlayerEngine.TotalTrackInTempPlaylist > PlayerEngine.CurrentTrackIndex)
                {
                    try
                    {
                        i = PlayerEngine.CurrentTrackIndex;
                        PlayerEngine.CurrentTrackIndex = i + 1;
                        lv.Items[i + 1].Selected = true;
                        lv.Items[i + 1].EnsureVisible();
                        path = lv.Items[i + 1].SubItems[3].Text;
                        PlayNewSong(path);
                    }
                    catch { return 1; }
                    return 0; // 0 on success, 1 on fail
                }
                else
                {
                  //  MessageBox.Show("Compare not valid");
                    return 1;
                }
            }
            else
            {
              //  MessageBox.Show("Listview have no items");
                return 1;
            }
        }

        public int PrevSong()
        {
            int i = 0;
            string path = "";
            ListView lv = listView1;
            if (lv.Items.Count > 0)
            {
                if (PlayerEngine.CurrentTrackIndex > 0 && PlayerEngine.CurrentTrackIndex <= PlayerEngine.TotalTrackInTempPlaylist)
                {
                    try
                    {
                        i = PlayerEngine.CurrentTrackIndex;
                        PlayerEngine.CurrentTrackIndex = i - 1;
                        lv.Items[i - 1].Selected = true;
                        lv.Items[i - 1].EnsureVisible();
                        path = lv.Items[i - 1].SubItems[3].Text;
                        PlayNewSong(path);
                    }
                    catch { }
                    return 0; // 0 on success, 1 on fail
                }
                else
                    return 1;
            }
            else
                return 1;
        }

        public void btnPlay_Click(object sender, EventArgs e)
        {
            if (PlayerEngine.Pause == true)
            {
                PlayerEngine.Resume();
                PlayerEngine.Pause = false;
                PlayerEngine.Stop = false;
                PlayerEngine.Play = true;
                tbManager.SetProgressState(TaskbarProgressBarState.Normal);
                btnPlay.Image = Properties.Resources.Pause.GetThumbnailImage(16, 16, null, new IntPtr());
            }
            else if (PlayerEngine.Play == true)
            {
                PlayerEngine.Resume();
                PlayerEngine.Pause = true;
                PlayerEngine.Stop = false;
                PlayerEngine.Play = false;
                tbManager.SetProgressState(TaskbarProgressBarState.Paused);
                btnPlay.Image = Properties.Resources.Play.GetThumbnailImage(16, 16, null, new IntPtr());
            }
            else if (PlayerEngine.Stop == true)
            {
                tbManager.SetProgressValue(0, 100);
                tbManager.SetProgressState(TaskbarProgressBarState.NoProgress);
                if (PlayerEngine.current_track.Trim().Length > 3 && PlayerEngine.CurrentTrackIndex > -1 && PlayerEngine.TotalTrackInTempPlaylist > -1)
                {
                    PlayNewSong(PlayerEngine.current_track);
                }
            }
        }

        private void listView1_ColumnClick(object sender, ColumnClickEventArgs e)
        {
            if (e.Column == lvwColumnSorter.SortColumn)
            {
                // Reverse the current sort direction for this column.
                if (lvwColumnSorter.Order == SortOrder.Ascending)
                {
                    lvwColumnSorter.Order = SortOrder.Descending;
                }
                else
                {
                    lvwColumnSorter.Order = SortOrder.Ascending;
                }
            }
            else
            {
                // Set the column number that is to be sorted; default to ascending.
                lvwColumnSorter.SortColumn = e.Column;
                lvwColumnSorter.Order = SortOrder.Ascending;
            }

            // Perform the sort with these new sort options.
            this.listView1.Sort();
        }

        public void btnStop_Click(object sender, EventArgs e)
        {
            if (PlayerEngine.isPlaying)
            {
                tbManager.SetProgressValue(0, 100);
                tbManager.SetProgressState(TaskbarProgressBarState.NoProgress);
                btnPlay.Image = Properties.Resources.Play.GetThumbnailImage(16, 16, null, new IntPtr());
                seekTimer.Enabled = false;
                TrackTime_label.Text = "00:00 - 00:00";
                seekBar.Value = 0;
                PlayerEngine.StopSong();
            }
        }

        public void btnNext_Click(object sender, EventArgs e)
        {
            seekTimer.Stop();
            NextSong();
            seekTimer.Start();
        }

        public void btnPrev_Click(object sender, EventArgs e)
        {
            seekTimer.Stop();
            PrevSong();
            seekTimer.Start();
        }

        [HandleProcessCorruptedStateExceptions]
        public void VisualStart(string name)
        {
            string path = name;
            try
            {
                if (File.Exists(path))
                {
                    if (visual == true) // some SFX is already started....
                    {
                        BASS_SFX_PluginStop(hSFX);
                        BASS_SFX_PluginFree(hSFX);
                    }
                    visualpath = path;
                    if (!full_screen)
                    {
                        ReleaseDC(coverImage.Handle, hVisDC);
                        hVisDC = GetDC(coverImage.Handle);
                        hSFX = BASS_SFX_PluginCreate(path, coverImage.Handle, coverImage.Width, coverImage.Height, 0);
                    }
                    else
                    {
                        ReleaseDC(fulvis.Handle, hVisDC);
                        hVisDC = GetDC(fulvis.Handle);
                        hSFX = BASS_SFX_PluginCreate(path, fulvis.Handle, fulvis.Width, fulvis.Height, 0);
                    }

                    BASS_SFX_PluginSetStream(hSFX, PlayerEngine.channel);

                    BASS_SFX_PluginStart(hSFX);

                    visTimer.Interval = 40;
                    visTimer.Enabled = true;

                    visual = true;
                    PlayerEngine.ShowCover = false;
                }
                else
                {
                    MessageBox.Show("Requested visual '" + name + "' not found!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    visual = false;
                    visualpath = null;
                    coverImage.Image = Properties.Resources.art;
                }
            }
            catch { }
        }

        private void visTimer_Tick(object sender, EventArgs e)
        {
            if (visual)
            {
                if (hSFX != -1)
                    BASS_SFX_PluginRender(hSFX, PlayerEngine.channel, hVisDC);
            }
        }

        private void coverImage_ClientSizeChanged(object sender, EventArgs e)
        {
            if (hSFX != -1)
                BASS_SFX_PluginResize(hSFX, coverImage.Width, coverImage.Height);
            coverImage.Refresh();
        }

        [HandleProcessCorruptedStateExceptions]
        public string Visual_Get_Name(string path)
        {
            string name = null;
            try
            {
                int handle = BASS_SFX_PluginCreate(path, tempPicBox.Handle, tempPicBox.Width, tempPicBox.Height, 0);
                if (handle != 0)
                {
                    name = BASS_SFX_PluginGetName(handle);
                    if (name == null)
                        name = System.IO.Path.GetFileNameWithoutExtension(path);
                    BASS_SFX_PluginFree(handle);
                }
                else
                {
                    name = System.IO.Path.GetFileNameWithoutExtension(path);
                }

                return name;
            }
            catch
            {
                name = System.IO.Path.GetFileNameWithoutExtension(path);
                return name;
            }

        }

        public void LoadVisuals()
        {
            int count = 10; // Limited for home users
            string path = System.IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            path += "\\visuals";
            if (Directory.Exists(path))
            {
                string[] svp = Directory.GetFiles(path, "*.svp", SearchOption.AllDirectories);
                string[] dll = Directory.GetFiles(path, "*.dll", SearchOption.AllDirectories);
                ToolStripItem off = (coverContext.Items[1] as ToolStripMenuItem).DropDownItems.Add("OFF");
                off.Click += oFFVisual_Click;

                foreach (string item in svp)
                {
                    if (!settings.Reg)
                        count--;
                    if (count == 0)
                        break;
                    ToolStripItem ti = (coverContext.Items[1] as ToolStripMenuItem).DropDownItems.Add(Visual_Get_Name(item));
                    ti.Tag = item;
                    ti.Click += OnClickPlugin;
                }
                foreach (string item in dll)
                {
                    if (!settings.Reg)
                        count--;
                    if (count == 0)
                        break;
                    ToolStripItem ti = (coverContext.Items[1] as ToolStripMenuItem).DropDownItems.Add(Visual_Get_Name(item));
                    ti.Tag = item;
                    ti.Click += OnClickPlugin;
                }
            }
            else
            {
                ToolStripItem ti = (coverContext.Items[1] as ToolStripMenuItem).DropDownItems.Add("Sorry no visual plugin found!");
            }
        }

        private void OnClickPlugin(object sender, EventArgs e)
        {
            foreach (ToolStripMenuItem it in (coverContext.Items[1] as ToolStripMenuItem).DropDownItems)
            {
                it.Checked = false;
            }
            ToolStripItem item = sender as ToolStripItem;
            ToolStripMenuItem mi = sender as ToolStripMenuItem;
            mi.Checked = true;
            string path = item.Tag as string;
            VisualStart(path);
        }

        private void oFFVisual_Click(object sender, EventArgs e)
        {
            foreach (ToolStripMenuItem it in (coverContext.Items[1] as ToolStripMenuItem).DropDownItems)
            {
                it.Checked = false;
            }
            ToolStripMenuItem mi = sender as ToolStripMenuItem;
            mi.Checked = true;
            visual = false;
            visTimer.Enabled = false;
            try
            {
                BASS_SFX_PluginStop(hSFX);
                BASS_SFX_PluginFree(hSFX);
            }
            catch { }
            coverImage.Image = Properties.Resources.art;
        }

        private void onToolStripMenuItem_Click(object sender, EventArgs e) // cover on
        {
            if (visual)
            {
                visual = false;
                visTimer.Enabled = true;
                try
                {
                    BASS_SFX_PluginStop(hSFX);
                    BASS_SFX_PluginFree(hSFX);
                    ReleaseDC(coverImage.Handle, hVisDC);
                    ReleaseDC(fulvis.Handle, hVisDC);
                }
                catch { }
                coverImage.Image = Properties.Resources.art;
            }
            if (PlayerEngine.image == null)
                coverImage.Image = Properties.Resources.no_art;
            else
                coverImage.Image = PlayerEngine.image;
            visual = false;
            PlayerEngine.ShowCover = true;
            if (full_screen)
            {
                fulvis.BackColor = Color.Black;
                fulvis.Refresh();
                fulvis.BackgroundImage = coverImage.Image;
            }
        }

        private void oFFToolStripMenuItem_Click(object sender, EventArgs e) // cover off
        {
            PlayerEngine.ShowCover = false;
            visual = false;
            visTimer.Enabled = false;
            try
            {
                BASS_SFX_PluginStop(hSFX);
                BASS_SFX_PluginFree(hSFX);
                ReleaseDC(coverImage.Handle, hVisDC);
                ReleaseDC(fulvis.Handle, hVisDC);
            }
            catch { }
            coverImage.Image = Properties.Resources.art;
            if (full_screen)
            {
                fulvis.BackColor = Color.Black;
                fulvis.Refresh();
                fulvis.BackgroundImage = coverImage.Image;
            }
        }

        private void saveCoverArtToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (PlayerEngine.ShowCover)
            {
                if (PlayerEngine.image != null)
                {
                    saveFileDialog1.Filter = "PNG Image|*.png|Bitmap Image|*.bmp|JPEG Image|*.jpg";
                    System.Drawing.Imaging.ImageFormat format = System.Drawing.Imaging.ImageFormat.Png;
                    if (saveFileDialog1.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                    {
                        string ext = System.IO.Path.GetExtension(saveFileDialog1.FileName);
                        switch (ext)
                        {
                            case ".jpg":
                                format = System.Drawing.Imaging.ImageFormat.Jpeg;
                                break;
                            case ".bmp":
                                format = System.Drawing.Imaging.ImageFormat.Bmp;
                                break;
                        }
                        coverImage.Image.Save(saveFileDialog1.FileName, format);
                    }
                }
            }
        }

        private void tabControl1_Selecting(object sender, TabControlCancelEventArgs e)
        {
            if (e.TabPage == tabPage5)
            {
                if (settings.Reg == false)
                {
                    MessageBox.Show("This feature is available in Pro edition!\nKindly register Tejash Player to enjoy this feature", "Register", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    e.Cancel = true; // if true tab will not be opened.
                }
            }
            this.Invalidate();
            this.Refresh();
        }

        private void btnBluetoothShare_Click(object sender, EventArgs e)
        {
            try
            {
                if (thrSend.IsAlive == true)
                {
                    DialogResult dr = MessageBox.Show("Do you want to cancel the share?", "Confirmation", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                    if (dr == DialogResult.Yes)
                    {
                        try { thrSend.Abort(); }
                        catch { }
                        backgroundWorker1.CancelAsync();
                        backgroundWorker1.Dispose();
                        this.progressBar1.Visible = false;
                        btnBluetoothShare.Image = Properties.Resources.Bluetooth_icon.GetThumbnailImage(16, 16, null, new IntPtr());
                        TrayPopUp("File Share", "Bluetooth file sharing cancelled!", 10);
                        return;
                    }
                    else
                        return;
                }
            }
            catch { }
            if (PlayerEngine.isPlaying)
            {
                BluetoothRadio br = BluetoothRadio.PrimaryRadio;
                if (br != null)
                {
                    if (br.Mode == RadioMode.PowerOff)
                    {
                        br.Mode = RadioMode.Connectable;
                    }

                    sbdd = new SelectBluetoothDeviceDialog();
                    sbdd.ShowAuthenticated = true;
                    sbdd.ShowRemembered = true;
                    sbdd.ShowUnknown = true;
                    
                    if (sbdd.ShowDialog() == DialogResult.OK)
                    {
                        btnBluetoothShare.Image = Properties.Resources.cancel_icon.GetThumbnailImage(16, 16, null, new IntPtr());
                        backgroundWorker1.WorkerSupportsCancellation = true;
                        backgroundWorker1.RunWorkerAsync();
                    }
                }
                else
                {
                    TrayPopUp("Error", "No bluetooth device found!\nEither your bluetooth device is OFF or not supported\nSupported devices are:\nMicrosoft, Widcomm/Broadcom, BlueSoleil, Bluetopia and few others", 10);
                  //  MessageBox.Show("No bluetooth device found!\nEither your bluetooth device is OFF or not supported\nSupported devices are:\nMicrosoft, Widcomm/Broadcom, BlueSoleil, Bluetopia and few others");
                }
            }
            else
            {
            }
        }

        private void backgroundWorker1_DoWork(object sender, DoWorkEventArgs e)
        {
            this.progressBar1.Visible = true;
            Cursor.Current = Cursors.WaitCursor;
            thrSend = new Thread(new ThreadStart(sendnow));
            thrSend.Start();
            Cursor.Current = Cursors.Default;
        }

        private void sendnow()
        {
            try
            {
                System.Uri uri = new Uri("obex://" + sbdd.SelectedDevice.DeviceAddress.ToString() + "/" + System.IO.Path.GetFileName(PlayerEngine.current_track));
                request = new ObexWebRequest(uri);
                request.ReadFile(PlayerEngine.current_track);
                TrayPopUp("Sending", "The song: " + PlayerEngine.TrackName + " is now being sent to " + sbdd.SelectedDevice.DeviceName, 10);
                ObexWebResponse response = (ObexWebResponse)request.GetResponse();
               // MessageBox.Show(response.StatusCode.ToString());
                string buf = response.StatusCode.ToString();
                if(buf.Contains("Forbidden"))
                {
                    TrayPopUp("Unable to send file!", "The file can not be shared because the remote user rejected the request!", 10);
                }
                else if(buf.Contains("OK"))
                {
                    TrayPopUp("Sent!", "The file has been successfully send to " + sbdd.SelectedDevice.DeviceName, 10);
                }
                response.Close();
            }
            catch
            {
                this.progressBar1.Visible = false;
                btnBluetoothShare.Image = Properties.Resources.Bluetooth_icon.GetThumbnailImage(16, 16, null, new IntPtr());
                TrayPopUp("Error", "Unable to send the file: " + PlayerEngine.TrackName, 10);
                this.backgroundWorker1.Dispose();
                thrSend.Abort();
            }
            btnBluetoothShare.Image = Properties.Resources.Bluetooth_icon.GetThumbnailImage(16, 16, null, new IntPtr());
            this.progressBar1.Visible = false;
            this.backgroundWorker1.Dispose();
            try
            {
                thrSend.Abort();
            }
            catch { }
        }

        private void startYoutubeHomeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            webBrowser1.Visible = true;
            webBrowser1.Navigate("https://www.youtube.com");
        }

        private void stopToolStripMenuItem_Click(object sender, EventArgs e)
        {
            webBrowser1.Visible = false;
            webBrowser1.Navigate("http://127.0.0.1");
        }

        private void backToolStripMenuItem_Click(object sender, EventArgs e)
        {
            webBrowser1.GoBack();
        }

        private void forwardToolStripMenuItem_Click(object sender, EventArgs e)
        {
            webBrowser1.GoForward();
        }

        private void toolStripMenuItem1_Click(object sender, EventArgs e)
        {
            webBrowser2.Visible = true;
            webBrowser2.Navigate("https://www.facebook.com");
        }

        private void toolStripMenuItem2_Click(object sender, EventArgs e)
        {
            webBrowser2.Visible = false;
            webBrowser2.Navigate("http://127.0.0.1");
        }

        private void toolStripMenuItem3_Click(object sender, EventArgs e)
        {
            webBrowser2.GoBack();
        }

        private void toolStripMenuItem4_Click(object sender, EventArgs e)
        {
            webBrowser2.GoForward();
        }

        public void AddToPlaylist(string path,bool silent, bool CreateNew, string SavePath)
        {
            if (path != PlayerEngine.current_track)
            {
                string name = "";
                string artist = "";
                string album = "";
                ListViewItem lvi;
                int items;
                ListViewItem.ListViewSubItem lvsi;
                IniFile ini;
                if (CreateNew == false)
                {
                    if (File.Exists(CurrentPlaylistLocation) == false)
                    {
                        string folder = settings.appdata_folder;
                        string library_location = folder + "\\Tejash-Player-Playlist.tpp";
                        CurrentPlaylistLocation = library_location;
                    }
                    ini = new IniFile(CurrentPlaylistLocation);
                }
                else
                    ini = new IniFile(SavePath);

                string Total = ini.IniReadValue("Total_Files", "Total");

                if (Total != "")
                    items = Convert.ToInt32(Total);
                else
                    items = 0;

                if (items > 0)
                    items++;
                else
                    items = 1;

                ini.IniWriteValue("songs", items.ToString(), path);
                ini.IniWriteValue("Total_Files", "Total", items.ToString());

                // Add this to playlist, now
                PlayerEngine.GetTrackData(path, out name, out artist, out album);
                try
                {
                    lvi = new ListViewItem();
                    lvi.Text = name;

                    lvsi = new ListViewItem.ListViewSubItem();
                    lvsi.Text = album;
                    lvi.SubItems.Add(lvsi);

                    lvsi = new ListViewItem.ListViewSubItem();
                    lvsi.Text = artist;
                    lvi.SubItems.Add(lvsi);

                    lvsi = new ListViewItem.ListViewSubItem();
                    lvsi.Text = path;
                    lvi.SubItems.Add(lvsi);

                    this.listView1.Items.Add(lvi);

                    PlayerEngine.TotalTrackInTempPlaylist = listView1.Items.Count - 1;
                }
                catch { }

                if (silent == false)
                {
                    PlayerEngine.CurrentTrackIndex = items;
                    PlayerEngine.Player(path);
                    AskedPlay();
                }
            }
        }

        private void openFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (System.IO.File.Exists(CurrentPlaylistLocation))
            {
                if (MessageBox.Show("Do you want to add the new songs to current playlist or create a new one?\nPress OK to add songs to current, or CANCEL to create a new playlist", "Confirm", MessageBoxButtons.OKCancel, MessageBoxIcon.Question) == DialogResult.Cancel)
                {
                    saveFileDialog1.AddExtension = true;
                    saveFileDialog1.DefaultExt = "tpp";
                    saveFileDialog1.Filter = "Tejash Player Playlist (*.tpp)|*.tpp";
                    saveFileDialog1.OverwritePrompt = true;
                    if (saveFileDialog1.ShowDialog() == DialogResult.OK)
                    {
                        buffer = saveFileDialog1.FileName;
                        MakeNewPlayList = true;
                    }
                    else
                        return;
                }
            }
            else
            {
                MessageBox.Show("There is no active playlist!\nAll your songs will be saved in Default playlist");
                MakeNewPlayList = false;
            }
            folderBrowserDialog1.Description = "Select your folder or drive which contains the music files";
            if (folderBrowserDialog1.ShowDialog() == DialogResult.OK)
            {
                PathToSearch = folderBrowserDialog1.SelectedPath;
                thrSearch = new Thread(new ThreadStart(SearchFiles));
                thrSearch.Start();
            }
        }

        public void ArgLoader()
        {
            int before = listView1.Items.Count;
            try
            {
                foreach (string data in arguments)
                {
                    int p = listView1.Items.Count - 1;
                    FileAttributes attr = File.GetAttributes(data);
                    if ((attr & FileAttributes.Directory) == FileAttributes.Directory)
                    {
                        PathToSearch = data;
                        SearchFiles();
                    }
                    else
                    {
                        if (PlayerEngine.CanIPlay(data))
                            AddToPlaylist(data, true, false, "");
                    }
                    int after = listView1.Items.Count;
                    int l = listView1.Items.Count - 1;
                    ToIgnore = after - before;
                    /*   if (l > p)
                       {
                           PlayerEngine.CurrentTrackIndex = p;
                           NextSong();
                       }*/
                }
            }
            catch { }
            try { thrargloader.Abort(); }
            catch { }
        }

        public void ParseTempList()
        {
            int total = 0;
            string hold = string.Empty;
            string location = settings.appdata_folder + "\\temp.tpp";
            int p = listView1.Items.Count - 1 ;
            try
            {
                if (File.Exists(location))
                {
                    IniFile ini = new IniFile(location);
                    total = Convert.ToInt32(ini.IniReadValue("Total_Files", "Total"));
                    for (int i = 1; i <= total; i++)
                    {
                        hold = ini.IniReadValue("songs", i.ToString());
                        if (File.Exists(hold))
                        {
                            FileAttributes attr = File.GetAttributes(hold);
                            if ((attr & FileAttributes.Directory) == FileAttributes.Directory)
                            {
                                PathToSearch = hold;
                                SearchFiles();
                            }
                            else
                            {
                                if (PlayerEngine.CanIPlay(hold))
                                    AddToPlaylist(hold, true, false, "");
                            }
                        }
                    }
                    int l = listView1.Items.Count - 1;

                    if (l > p)
                    {
                        PlayerEngine.CurrentTrackIndex = p;
                        NextSong();
                    }
                    File.Delete(location);
                    try
                    {
                        wm_copydata_parser.Abort();
                    }
                    catch { }
                }
                else
                {
                    try
                    {
                        wm_copydata_parser.Abort();
                    }
                    catch { }
                }
            }
            catch { wm_copydata_parser.Abort(); }
        }

        public static bool IsLogicalDrive(string path)
        {
            bool IsRoot = false;
            DirectoryInfo d = new DirectoryInfo(path);
            if (d.Parent == null) { IsRoot = true; }
            return IsRoot;
        }

        public string[] getFiles(string SourceFolder, string Filter,System.IO.SearchOption searchOption)
        {
            ArrayList alFiles = new ArrayList();
            string[] MultipleFilters = Filter.Split('|');

            if (IsLogicalDrive(SourceFolder))
            {
                foreach (string d in Directory.GetDirectories(SourceFolder))
                {
                    foreach (string FileFilter in MultipleFilters)
                    {
                        try
                        {
                            alFiles.AddRange(Directory.GetFiles(d, FileFilter, searchOption));
                        }
                        catch { continue; }
                    }
                }
            }
            else
            {
                foreach (string FileFilter in MultipleFilters)
                {
                    try
                    {
                        alFiles.AddRange(Directory.GetFiles(SourceFolder, FileFilter, searchOption));
                    }
                    catch { continue; }
                }
            }

            return (string[])alFiles.ToArray(typeof(string));
        }

        public void SearchFiles()
        {
            string backup = this.Text;
            this.Text = "Please wait, searching...";
            int Total = 0;
            int Done = 0;
            string[] files = getFiles(PathToSearch,PlayerEngine.filter,SearchOption.AllDirectories); //Directory.GetFiles(PathToSearch, "*.mp3", SearchOption.AllDirectories);
            Total = files.Length;
            this.progressBar1.Visible = true;
            this.progressBar1.Style = ProgressBarStyle.Blocks;
            this.progressBar1.Maximum = Total;
            this.progressBar1.Minimum = 0;

            foreach (string file in files)
            {
                try
                {
                    Done++;
                    progressBar1.Value = Done;
                    if (MakeNewPlayList == false)
                        AddToPlaylist(file, true, false, "");
                    else
                        AddToPlaylist(file, true, true, buffer);
                }
                catch { }
            }

            this.Text = backup;
            this.progressBar1.Visible = false;
            try { thrSearch.Abort(); }
            catch { }
        }

        private void openLocationToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (PlayerEngine.isPlaying)
            {
                System.Diagnostics.Process.Start("explorer.exe", "/select, " + PlayerEngine.current_track);
            }
        }

        private void fromLibraryToolStripMenuItem_Click(object sender, EventArgs e) // remove playing song from list
        {
            if (PlayerEngine.isPlaying)
            {
                if (MessageBox.Show("Do you want to remove the current song from playlist?", "Confirm", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
                {
                    btnPlay.Image = Properties.Resources.Play.GetThumbnailImage(16, 16, null, new IntPtr());
                    seekTimer.Enabled = false;
                    TrackTime_label.Text = "00:00 - 00:00";
                    seekBar.Value = 0;
                    PlayerEngine.StopSong();
                    ListView lv = listView1;
                    string temp = "";
                    lv.Items[PlayerEngine.CurrentTrackIndex].Remove();
                    IniFile ini = new IniFile(CurrentPlaylistLocation);
                    int Total = Convert.ToInt32(ini.IniReadValue("Total_Files", "Total"));
                    for (int i = 1; i <= Total; i++)
                    {
                        if (ini.IniReadValue("songs", i.ToString()) == PlayerEngine.current_track)
                        {
                            temp = ini.IniReadValue("songs", i.ToString());
                            temp = temp + ".ignore";
                            ini.IniWriteValue("songs", i.ToString(), temp);
                            MessageBox.Show("Deleted!");
                        }
                    }
                }
            }
        }

        private void fromComputerToolStripMenuItem_Click(object sender, EventArgs e) // remove playing song from pc
        {
            if (PlayerEngine.isPlaying)
            {
                if (MessageBox.Show("Do you want to Delete the current song?\nThe song will be deleted from your system!", "Confirm", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
                {
                    btnStop_Click(sender, e);
                    btnPlay.Image = Properties.Resources.Play.GetThumbnailImage(16, 16, null, new IntPtr());
                    seekTimer.Enabled = false;
                    TrackTime_label.Text = "00:00 - 00:00";
                    seekBar.Value = 0;
                    PlayerEngine.StopSong();
                    ListView lv = listView1;
                    lv.Items[PlayerEngine.CurrentTrackIndex].Remove();
                    try
                    {
                        System.IO.File.Delete(PlayerEngine.current_track);
                    }
                    catch
                    {
                        MessageBox.Show("Error deleting file!\nThe file is not deleted");
                    }
                }
            }
        }

        private void listView1_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyData == Keys.Enter)
            {
                ListView lv = (ListView)sender;
                if (lv.SelectedItems.Count > 0)
                {
                    for (int i = 0; i < lv.Items.Count; i++)
                    {
                        // is i the index of the row I selected?
                        if (lv.Items[i].Selected == true)
                        {
                            btnStop_Click(sender, e);
                            PlayerEngine.CurrentTrackIndex = i;
                            //MessageBox.Show("Index = " + Convert.ToString(i)); // Shows the index
                            //MessageBox.Show(lv.Items[i].SubItems[3].Text); // Shows the path
                            PlayNewSong(lv.Items[i].SubItems[3].Text);
                            break;
                        }
                    }
                }
            }
            if (e.KeyData == Keys.Delete)
            {
            }
        }

        private void openTejashPlayerPlaylistToolStripMenuItem_Click(object sender, EventArgs e)
        {
            openFileDialog1.Filter = "Tejash Player Playlists|*.tpp";
            DialogResult result = openFileDialog1.ShowDialog();
            if (result == DialogResult.OK)
            {
                try
                {
                    try
                    {
                        if (lib_temp_loader.IsAlive == true)
                            lib_temp_loader.Abort();
                    }
                    catch { }

                    ListView lv = listView1;
                    lv.Items.Clear();

                    CurrentPlaylistLocation = openFileDialog1.FileName;

                    lib_temp_loader = new Thread(new ThreadStart(LoadLibraryTemp));
                    lib_temp_loader.Start();
                }
                catch 
                {
                    MessageBox.Show("Error occurred!\nThe playlist can not be opened correctly");
                }
            }
        }

        private void loadTemporaryListToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (File.Exists(settings.appdata_folder + "\\Tejash-Player-Playlist.tpp"))
            {
                try
                {
                    if (lib_temp_loader.IsAlive == true)
                        lib_temp_loader.Abort();

                    ListView lv = listView1;
                    lv.Items.Clear();

                    CurrentPlaylistLocation = settings.appdata_folder + "\\Tejash-Player-Playlist.tpp";

                    lib_temp_loader = new Thread(new ThreadStart(LoadLibraryTemp));
                    lib_temp_loader.Start();
                }
                catch { MessageBox.Show("Error occurred while loading temporary list!"); }
            }
            else
                MessageBox.Show("No temporary playlist found!\nEither it's deleted or not available to access");
        }

        private void organiseSongsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (settings.Reg)
            {
                // Kick a check here...
                Organiser org = new Organiser();
                org.Show();
            }
            else
                MessageBox.Show("This feature is available in Pro edition!\nKindly register Tejash Player to enjoy this feature", "Register", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
        }

        private void updateCurrentPlaylistToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void searchToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Registration reg = new Registration();
            reg.Show();
        }

        private void dragAndDropFileComponent1_FileDropped(object sender, DragAndDropFileControlLibrary.FileDroppedEventArgs e)
        {
            string[] Files = e.Filenames;
            foreach (string file in Files)
            {
                MessageBox.Show(file);
            }
        }

        private void mainDialog_Shown(object sender, EventArgs e)
        {
            if (load_args)
            {
                temp = new Thread(new ThreadStart(seekargs));
                temp.Start();
            }
        }

        public void seekargs()
        {
            int a = 0;
            while (listView1.Items.Count < 1)
            {
                System.Threading.Thread.Sleep(1000);
                a++;
                if (a > 60000) // is I am waiting from last one min?
                {
                    try { temp.Abort(); }
                    catch { }
                }
            }
            PlayerEngine.CurrentTrackIndex = -1;
            NextSong();
        }

        private void blackToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SetSkin(1);
        }

        private void blueToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SetSkin(2);
        }

        private void silverToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SetSkin(3);
        }

        private void CreateThumbnailButtons() // Function to create thumbnail buttons 
        {
            Bitmap bmp;
            Icon icon;

            bmp = new Bitmap(btnPlay.Image);
            bmp.SetResolution(16,16);
            icon = System.Drawing.Icon.FromHandle(bmp.GetHicon());
            play = new ThumbnailToolBarButton(icon, "Play,Pause");
            play.Click += (s, e) => btnPlay_Click(s,e);

            bmp = new Bitmap(btnPrev.Image);
            bmp.SetResolution(16, 16);
            icon = System.Drawing.Icon.FromHandle(bmp.GetHicon());
            prev = new ThumbnailToolBarButton(icon, "Previous");
            prev.Click += (s, e) => btnPrev_Click(s, e);

            bmp = new Bitmap(btnNext.Image);
            bmp.SetResolution(16, 16);
            icon = System.Drawing.Icon.FromHandle(bmp.GetHicon());
            next = new ThumbnailToolBarButton(icon, "Next");
            next.Click += (s, e) => btnNext_Click(s, e);

            hide = new ThumbnailToolBarButton(Properties.Resources.hide, "Hide");
            hide.Click += (s, e) => HideMe();

            TaskbarManager.Instance.ThumbnailToolBars.AddButtons(this.Handle, prev,play,next,hide);
        }

        public void TrayPopUp(string title,string message,int second) // Tray pop up functions 
        {
            TrayBalloon.TrayBalloon tb = new TrayBalloon.TrayBalloon();
            tb.PopUpTime = second*1000; // 7 second
            tb.Title = title;
            tb.Message = message;
            tb.TopMost = true;
            if (notify_sounds)
            {
                tb.SoundLocation = System.IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location) +"\\notify.wav";
            }
            tb.LightWeight = true;
            tb.Run();
        }

        public void HideMe() // Function to hide and show player window 
        {
            if (this.Visible)
            {
                this.Hide();
                TrayPopUp("Minimized to tray", "Tejash Player is minimized to tray, to restore either press ALT+H or double click the Tray Icon", 7);
            }
            else
                this.Show();
        }

        private void hideToolStripMenuItem_Click(object sender, EventArgs e) // Handle clicks on HideMe in menu
        {
            HideMe();
        }

        private void sysTrayIcon_MouseDoubleClick(object sender, MouseEventArgs e) // Handle clicks on system tray icon 
        {
            HideMe();
        }

        private void RegisterHotKeys() // Register hotkeys 
        {
            Object s = null;
            EventArgs e = null;

            playhotkey = new Hotkey();
            playhotkey.Alt = true;
            playhotkey.KeyCode = Keys.P;
            playhotkey.Pressed += delegate { btnPlay_Click(s, e); };

            prevhotkey = new Hotkey();
            prevhotkey.Alt = true;
            prevhotkey.KeyCode = Keys.Left;
            prevhotkey.Pressed += delegate { btnPrev_Click(s, e); };

            nexthotkey = new Hotkey();
            nexthotkey.Alt = true;
            nexthotkey.KeyCode = Keys.Right;
            nexthotkey.Pressed += delegate { btnNext_Click(s, e); };

            stophotkey = new Hotkey();
            stophotkey.Alt = true;
            stophotkey.KeyCode = Keys.S;
            stophotkey.Pressed += delegate { btnStop_Click(s, e); };

            hidehotkey = new Hotkey();
            hidehotkey.Alt = true;
            hidehotkey.KeyCode = Keys.H;
            hidehotkey.Pressed += delegate { HideMe(); };

            try
            {
                if (!playhotkey.GetCanRegister(this))
                {
                    MessageBox.Show("Failed to register hotkey!\nPlay");
                }
                else
                {
                    playhotkey.Register(this);
                }

                if (!prevhotkey.GetCanRegister(this))
                {
                    MessageBox.Show("Failed to register hotkey!\nPrev");
                }
                else
                {
                    prevhotkey.Register(this);
                }

                if (!nexthotkey.GetCanRegister(this))
                {
                    MessageBox.Show("Failed to register hotkey!\nNext");
                }
                else
                {
                    nexthotkey.Register(this);
                }

                if (!stophotkey.GetCanRegister(this))
                {
                    MessageBox.Show("Failed to register hotkey!\nStop");
                }
                else
                {
                    stophotkey.Register(this);
                }

                if (!hidehotkey.GetCanRegister(this))
                {
                    MessageBox.Show("Failed to register hotkey!\nHide");
                }
                else
                {
                    hidehotkey.Register(this);
                }
            }
            catch { }
        }

        private void Unregisterkeyhot() // Unregister hotkeys
        {
            try
            {
                if (playhotkey.Registered)
                    playhotkey.Unregister();
                if (prevhotkey.Registered)
                    prevhotkey.Unregister();
                if (nexthotkey.Registered)
                    nexthotkey.Unregister();
                if (stophotkey.Registered)
                    stophotkey.Unregister();
                if (hidehotkey.Registered)
                    hidehotkey.Unregister();
            }
            catch { }
        }

        private void checkBox3_CheckedChanged(object sender, EventArgs e) // remember volume
        {
            if (checkBox3.CheckState == CheckState.Checked)
                Settings.Default.RememberVolume = true;
            else
                Settings.Default.RememberVolume = false;
        }

        private void checkBox4_CheckedChanged(object sender, EventArgs e) // last.fm scrobbling
        {
            if (settings.Reg)
            {
                if (checkBox4.CheckState == CheckState.Checked)
                {
                    try
                    {
                        lpfm = new LastFM_Scrobbling(settings.appdata_folder + "\\TejashPlayer.DLL");
                        if (lpfm.isReady)
                        {
                            lastfm_pic.Visible = true;
                            use_lastfm = true;
                        }
                        else
                        {
                            use_lastfm = false;
                            lastfm_pic.Visible = false;
                            checkBox4.Checked = false;
                        }
                    }
                    catch
                    {
                        use_lastfm = false;
                        lastfm_pic.Visible = false;
                        checkBox4.Checked = false;
                    }
                }
                else
                {
                    use_lastfm = false;
                    try { lpfm.isReady = false; }
                    catch { }
                    lastfm_pic.Visible = false;
                }
            }
            else
            {
                checkBox4.Checked = false;
                MessageBox.Show("This feature is available in Pro edition!\nKindly register Tejash Player to enjoy this feature", "Register", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        private void checkBox5_CheckedChanged(object sender, EventArgs e) // notification sounds
        {
            if (checkBox5.CheckState == CheckState.Checked)
                notify_sounds = true;
            else
                notify_sounds = false;
        }

        private void toolStripMenuItem5_Click(object sender, EventArgs e) // Sound Devices menu strip
        {
            SndDevices dev = new SndDevices(PlayerEngine,this);
            dev.Show();
        }

        public void SetEngineObject(int id) // Setback the new player engine object which is changed by sound device changes
        {
            if (PlayerEngine.isPlaying)
            {
                double time = PlayerEngine.GetTrackCurrentTime();
                tbManager.SetProgressValue(0, 100);
                tbManager.SetProgressState(TaskbarProgressBarState.NoProgress);
                btnPlay.Image = Properties.Resources.Play.GetThumbnailImage(16, 16, null, new IntPtr());
                seekTimer.Enabled = false;
                TrackTime_label.Text = "00:00 - 00:00";
                seekBar.Value = 0;
                PlayerEngine.StopSong();
                PlayerEngine.RestartEngine(this.Handle, id);
                PlayNewSong(PlayerEngine.current_track);
                PlayerEngine.SetTrackPos(time);
            }
            else
            {
                tbManager.SetProgressValue(0, 100);
                tbManager.SetProgressState(TaskbarProgressBarState.NoProgress);
                btnPlay.Image = Properties.Resources.Play.GetThumbnailImage(16, 16, null, new IntPtr());
                seekTimer.Enabled = false;
                TrackTime_label.Text = "00:00 - 00:00";
                seekBar.Value = 0;
                PlayerEngine.StopSong();
                PlayerEngine.RestartEngine(this.Handle, id);
            }
        }

        private void coverImage_DoubleClick(object sender, EventArgs e) // Full screen visual start
        {
            if (!full_screen)
            {
                full_screen = true;
                fulvis = new FulVis(this);
                fulvis.Show();
            }
        }

        private void informationToolStripMenuItem_Click(object sender, EventArgs e) // Shows the file information
        {
            string details = "";
            try
            {
                FileInfo fi = new FileInfo(PlayerEngine.current_track);
                string type;
                if (PlayerEngine.chiptune)
                {
                    type = "Chiptune/MOD";
                    details = "File Name: " + fi.Name;
                }
                else
                {
                    type = "Audio/Music";
                    details = "File Name: " + PlayerEngine.TrackName;
                    details = details + Environment.NewLine + "Artist: " + PlayerEngine.artist;
                    details = details + Environment.NewLine + "Album: " + PlayerEngine.album;
                }
                details = details + Environment.NewLine + "Type: " + type;
                details = details + Environment.NewLine + "Directory: " + fi.Directory;
                details = details + Environment.NewLine + "Full Path: " + PlayerEngine.current_track;
                details = details + Environment.NewLine + "Creation Time: " + fi.CreationTime;
                details = details + Environment.NewLine + "Extension: " + fi.Extension;
                details = details + Environment.NewLine + "Size: " + (fi.Length) / 1024 + " KB";
                details = details + "\n\nWould you like to open file location?";
                if (MessageBox.Show(details, "Audio Information", MessageBoxButtons.YesNo, MessageBoxIcon.None) == DialogResult.Yes)
                {
                    openLocationToolStripMenuItem_Click(sender, e);
                }
            }
            catch { }
        }
    }
}
