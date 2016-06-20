using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using SharpFactory.Yamo.Core;
using System.IO;
using System.Security.AccessControl;
using System.Security.Principal;

namespace Tejash_Player_2014
{
    public partial class Organiser : Form
    {
        public string path;
        public Thread DoWork;
        public Options options;
        public Engine engine;
        public RichTextBox rtb;
        public const int WM_CLOSE = 0x0010;

        public Organiser()
        {
            InitializeComponent();
        }

        private void Organiser_Load(object sender, EventArgs e)
        {
            wizard1.NextEnabled = false;
            radioButton1.Checked = true;
            radioButton3.Checked = true;
        }

        public static bool HasFolderWritePermission(string path)
        {
            int c = 0;
            try
            {
                DirectoryInfo di = new DirectoryInfo(path);
                DirectorySecurity acl = di.GetAccessControl();
                AuthorizationRuleCollection rules = acl.GetAccessRules(true, true, typeof(NTAccount));

                WindowsIdentity currentUser = WindowsIdentity.GetCurrent();
                WindowsPrincipal principal = new WindowsPrincipal(currentUser);
                foreach (AuthorizationRule rule in rules)
                {
                    FileSystemAccessRule fsAccessRule = rule as FileSystemAccessRule;
                    if (fsAccessRule == null)
                        continue;

                    if ((fsAccessRule.FileSystemRights & FileSystemRights.WriteData) > 0)
                    {
                        NTAccount ntAccount = rule.IdentityReference as NTAccount;
                        if (ntAccount == null)
                        {
                            continue;
                        }
                        if (principal.IsInRole(ntAccount.Value))
                        {
                            c++;
                            continue;
                        }
                    }
                }
                if (c > 0) return true;
                else return false;
            }
            catch (UnauthorizedAccessException)
            {
                return false;
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            folderBrowserDialog1.ShowNewFolderButton = false;
            folderBrowserDialog1.Description = "Select the folder containing music files to organize";
            if (folderBrowserDialog1.ShowDialog() == DialogResult.OK)
            {
                if (System.IO.Directory.Exists(folderBrowserDialog1.SelectedPath))
                {
                    textBox1.Text = folderBrowserDialog1.SelectedPath;
                    path = textBox1.Text;
                    if (HasFolderWritePermission(path))
                    {
                        wizard1.NextEnabled = true;
                        rtb = richTextBox1;
                        errorProvider1.Clear();
                    }
                    else
                    {
                        errorProvider1.SetError(textBox1, "You are not allowed to access the selected folder!\nTry to run the player with administrator permission.");
                    }
                }
            }
        }

        private void wizardPage3_ShowFromNext(object sender, EventArgs e)
        {
            wizard1.BackEnabled = false;
            options = new Options();
            engine = new Engine();
            engine.EngineEvent +=new EngineEventHandler(engine_EngineEvent);

            // First command line argument is the path 
            // of the music library we want to organize.
            options.LibraryRoot = path;

            // (***NEW***) Enable music library reorganization.
            options.OrganizeMusic = true;

            // organize songs by album, and then by genere
            if (radioButton2.Checked == true)
            {
                options.FolderOptions.Add(
                    new FolderOption(FolderType.Genre, "Unknown Genre"));
                options.FolderOptions.Add(
                    new FolderOption(FolderType.Album, "Unknown Album"));
            }
            else
            {
                options.FolderOptions.Add(
                    new FolderOption(FolderType.Album, "Unknown Album"));
                options.FolderOptions.Add(
                    new FolderOption(FolderType.Genre, "Unknown Genre"));
            }

            // Song file names will contain track and title.
            // This corresponds to the '[Track] - [Title]' 
            // format string in the Yamo Wizard.
            if (radioButton3.Checked == true)
            {
                options.FilenameOptions.Add(
                    new FilenameOption(FilenameComponent.Track));
                options.FilenameOptions.Add(
                    new FilenameOption(" - "));
                options.FilenameOptions.Add(
                    new FilenameOption(FilenameComponent.Title));
            }
            else
            {
                options.FilenameOptions.Add(
                    new FilenameOption(FilenameComponent.Title));
                options.FilenameOptions.Add(
                    new FilenameOption(" - "));
                options.FilenameOptions.Add(
                    new FilenameOption(FilenameComponent.Track));
            }

            // Enable the Windows media player tag reader
            try
            {
                options.TagReader = TagReaders.WindowsMedia;
            }
            catch
            {
                // error in using WMP tag reader, try Shazam !
                MessageBox.Show("Unable to use Windows Media Player engine as Tag reader!\nTrying Shazam engine");
                options.TagReader = TagReaders.Shazam;
            }

            // Enable album art downloading and activate the
            // Amazon media information provider.

            // Dont use art downloading, YAMO engine have BUG :(

           // options.DownloadAlbumArt = true;
            //options.MediaInformationProviders.Add("Amazon");
            progressBar1.Maximum = 100;
            progressBar1.Minimum = 0;
            progressBar1.Value = 0;

            engine.Start(options);


            timer1.Enabled = true;
        }

        private void engine_EngineEvent(object sender, EngineEventArgs args)
        {
            progressBar1.Value = args.PercentCompleted;
            label5.Text = args.PercentCompleted.ToString() + "% Completed";
            if(args.EventType != EngineEventType.Message) 
            {
                richTextBox1.AppendText(args.Message + Environment.NewLine);
                label5.Text = "Done";
            }
        }

        protected override void WndProc(ref Message m)
        {
            DialogResult dr;
            if (m.Msg == WM_CLOSE)
            {
                try
                {
                    if (engine.IsRunning)
                    {
                        dr = MessageBox.Show("You are closing the organizer and your work is not completed yet\nDo you want to close it now?", "Confirm", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                        if (dr == DialogResult.Yes)
                        {
                            engine.Stop();
                            System.IO.File.Delete(path + "\\YamoLog.txt");
                            timer1.Enabled = false;
                        }
                        else
                            return;
                    }
                }
                catch { }
            }
            base.WndProc(ref m);
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (engine.IsRunning == false)
            {
                this.Invalidate(true);
                this.Update();
                System.IO.File.Delete(path + "\\YamoLog.txt");
                timer1.Enabled = false;
                MessageBox.Show("All your music files are now organized!", "Done", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

    }
}
