using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Reflection;
using Un4seen.Bass;
using TagLib;
using System.IO;
using System.Runtime.InteropServices;

namespace Tejash_Player_2014
{
    public class CorePlayerEngine
    {
        public bool PlayerEngine_started = false;
        public string TrackName;
        public string album;
        public string artist;
        public int channel;
        public string current_track;
        public bool isPlaying = false;
        public IPicture[] cover;
        public IPicture cover_image;
        public System.Drawing.Image image = null;
        public string current_dir;
        public float volume = 100;
        public bool mute = false;
        public Int32 TotalTrackInTempPlaylist;
        public Int32 CurrentTrackIndex;
        public bool Pause = false;
        public bool Play = false;
        public bool Stop = false;
        public bool ShowCover = true;
        public bool chiptune = false;
        public string filter = "*.mp3|*.mp2|*.mp1|*.ogg|*.wav|*.aif|*.mo3|*.it|*.xm|*.s3m|*.mtm|*.mod|*.umx|*.cda|*.fla|*.flac|*.oga|*.wma|*.wv|*.aac|*.m4a|*.m4b|*.mp4|*.ac3|*.adx|*.aix|*.ape|*.mac|*.mpc|*.mp+|*.mpp|*.ofr|*.ofs|*.tta";

        public CorePlayerEngine(IntPtr handle,int Id)
        {
            Dictionary<int, string> loadedplugins = null;
            BassNet.Registration("support@tejashwi.com", "2X2881820182322");
            Bass.BASS_SetConfig(BASSConfig.BASS_CONFIG_DEV_DEFAULT, true);
            if (Bass.BASS_Init(Id, 44100, BASSInit.BASS_DEVICE_CPSPEAKERS, handle))
            {
                PlayerEngine_started = true;
                current_dir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
                try
                {
                    loadedplugins = Bass.BASS_PluginLoadDirectory(current_dir + "\\decoders");
                }
                catch
                {
                    MessageBox.Show("Unable to load the decoders!\nPlease install the player. If problem persist then contact the support team at helpdesk@techtejash.com", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    Application.Exit();
                }
                if (loadedplugins == null)
                {
                    MessageBox.Show("No decoders found!\nPlease install the player. If problem persist then contact the support team at helpdesk@techtejash.com", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    Application.Exit();
                }
            }
            else
            {
                PlayerEngine_started = false;
                MessageBox.Show("Error starting engine");
            }
        }

        ~CorePlayerEngine()
        {
            Bass.BASS_PluginFree(0);
            Bass.BASS_Free();
        }

        public void RestartEngine(IntPtr handle, int Id)
        {
            Bass.BASS_PluginFree(0);
            Bass.BASS_Free();
            Dictionary<int, string> loadedplugins = null;
            Bass.BASS_SetConfig(BASSConfig.BASS_CONFIG_DEV_DEFAULT, true);
            if (Bass.BASS_Init(Id, 44100, BASSInit.BASS_DEVICE_CPSPEAKERS, handle))
            {
                PlayerEngine_started = true;
                current_dir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
                try
                {
                    loadedplugins = Bass.BASS_PluginLoadDirectory(current_dir + "\\decoders");
                }
                catch
                {
                    MessageBox.Show("Unable to load the decoders!\nPlease install the player. If problem persist then contact the support team at helpdesk@techtejash.com", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    Application.Exit();
                }
                if (loadedplugins == null)
                {
                    MessageBox.Show("No decoders found!\nPlease install the player. If problem persist then contact the support team at helpdesk@techtejash.com", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    Application.Exit();
                }
            }
            else
            {
                PlayerEngine_started = false;
                MessageBox.Show("Error starting engine");
            }
        }

        public void Player(string filename)
        {
            try
            {
                if (PlayerEngine_started)
                {
                    if (System.IO.File.Exists(filename))
                    {
                        Bass.BASS_Stop();
                        if (chiptune == true)
                            Bass.BASS_MusicFree(channel);
                        else
                            Bass.BASS_StreamFree(channel);

                        channel = Bass.BASS_StreamCreateFile(filename, 0, 0, BASSFlag.BASS_SAMPLE_FX | BASSFlag.BASS_STREAM_AUTOFREE);
                        if (channel != 0)
                            chiptune = false;

                        if (channel == 0)
                        {
                            channel = Bass.BASS_MusicLoad(filename, 0, 0, BASSFlag.BASS_MUSIC_PRESCAN | BASSFlag.BASS_MUSIC_AUTOFREE | BASSFlag.BASS_MUSIC_RAMP | BASSFlag.BASS_SAMPLE_FX, 0);
                            if (channel != 0)
                                chiptune = true;
                        }

                        if (channel != 0)
                        {
                            current_track = filename;
                            Bass.BASS_Start();
                            isPlaying = true;
                            SetVolume(volume);
                            if (Bass.BASS_ChannelPlay(channel, false))
                            {
                                GetTags();
                                isPlaying = true;
                                Play = true;
                                Pause = false;
                                Stop = false;
                            }
                            else
                            {
                                MessageBox.Show("Unable to play channel");
                                isPlaying = false;
                                Play = false;
                                Pause = false;
                                Stop = false;
                            }
                        }
                        else
                        {
                            BASSError e = Bass.BASS_ErrorGetCode();
                            MessageBox.Show("Unable to create the stream to play!\nIf there is songs in playlist, then next song from playlist will be played!\nCode = " + e.ToString());
                        }
                    }
                    else
                    {
                        MessageBox.Show("Bad file named passed! Can not play!\nIf there is songs in playlist, then next song will be played!");
                    }
                }
                else
                {
                    MessageBox.Show("Due to some technical error, the player PlayerEngine is not started!\nPlease restart the player. If the problem continues then reinstall the player\nSorry for the problem caused!");
                }
            }
            catch { }
        }

        public void GetTags()
        {
            if (isPlaying)
            {
                if (chiptune == false)
                {
                    TagLib.File file = TagLib.File.Create(current_track);
                    TrackName = file.Tag.Title;
                    album = file.Tag.Album;
                    string[] Artist = file.Tag.AlbumArtists;
                    artist = "";
                    for (int i = 0; i < Artist.Length; i++)
                    {
                        artist = artist + " " + Artist[i];
                    }
                    cover = file.Tag.Pictures;
                    if (cover.Length > 0)
                    {
                        cover_image = cover[0];
                        MemoryStream stream = new MemoryStream(cover_image.Data.Data);
                        if (stream == null)
                            image = null;
                        else
                        {
                            try
                            {
                                image = System.Drawing.Image.FromStream(stream);
                            }
                            catch
                            {
                                image = null;
                            }
                        }
                    }
                    else
                        image = null;
                }
                else
                {
                    // There is chiptune.... get details
                    IntPtr p = Bass.BASS_ChannelGetTags(channel, BASSTag.BASS_TAG_MUSIC_NAME);
                    if (p != IntPtr.Zero)
                        TrackName = Marshal.PtrToStringAnsi(p);
                    else
                        TrackName = Path.GetFileNameWithoutExtension(current_track);
                    image = null;
                }
            }
            else
                MessageBox.Show("No track is currently playing, sorry!");
        }

        public double GetTrackCurrentTime()
        {
            long pos = Bass.BASS_ChannelGetPosition(channel,BASSMode.BASS_POS_BYTES);
            double time = Bass.BASS_ChannelBytes2Seconds(channel, pos);
            return time;
        }

        public double GetTrackTotalTime()
        {
            long pos = Bass.BASS_ChannelGetLength(channel, BASSMode.BASS_POS_BYTES);
            double time = Bass.BASS_ChannelBytes2Seconds(channel, pos);
            return time;
        }

        public void SetTrackPos(double value)
        {
            long seconds = Bass.BASS_ChannelSeconds2Bytes(channel, value);
            Bass.BASS_ChannelSetPosition(channel, seconds,BASSMode.BASS_POS_BYTES);
        }

        public void SetVolume(float pos)
        {
            if (!mute)
            {
                volume = pos;
                if (isPlaying)
                    Bass.BASS_ChannelSetAttribute(channel, BASSAttribute.BASS_ATTRIB_VOL, (pos / 100));
            }
            else
            {
                if(isPlaying)
                    Bass.BASS_ChannelSetAttribute(channel, BASSAttribute.BASS_ATTRIB_VOL,0);
            }
        }

        public void Mute()
        {
            if (mute)
            {
                Bass.BASS_ChannelSetAttribute(channel, BASSAttribute.BASS_ATTRIB_VOL, (volume / 100));
                mute = false;
            }
            else
            {
                Bass.BASS_ChannelSetAttribute(channel, BASSAttribute.BASS_ATTRIB_VOL, 0);
                mute = true;
            }
        }

        public void GetTrackData(string path,out string Name,out string TArtist,out string Album)
        {
            Name = "";
            TArtist = "";
            Album = "";
            if (IsMOD(path) == false)
            {
                try
                {
                    TagLib.File f = TagLib.File.Create(path);
                    if (f.Tag.Title != null)
                    {
                        Name = f.Tag.Title;
                        if (Name.Trim().Length < 4)
                            Name = Path.GetFileNameWithoutExtension(path);
                    }
                    else
                        Name = Path.GetFileNameWithoutExtension(path);

                    if (f.Tag.Album != null)
                        Album = f.Tag.Album;
                    else
                        Album = " ";

                    string[] Artist = f.Tag.AlbumArtists;
                    TArtist = "";
                    for (int i = 0; i < Artist.Length; i++)
                    {
                        TArtist = TArtist + " " + Artist[i];
                    }
                    if (TArtist.Length < 1)
                        TArtist = " ";
                }
                catch { }
            }
            else
            {
                try
                {
                    // There is chiptune.... get details
                    int temp_channel = Bass.BASS_MusicLoad(path, 0, 0, BASSFlag.BASS_MUSIC_PRESCAN | BASSFlag.BASS_MUSIC_AUTOFREE, 0);
                    IntPtr p = Bass.BASS_ChannelGetTags(temp_channel, BASSTag.BASS_TAG_MUSIC_NAME);
                    if (p != IntPtr.Zero)
                        Name = Marshal.PtrToStringAnsi(p);
                    else
                        Name = Path.GetFileNameWithoutExtension(current_track);
                    Bass.BASS_MusicFree(temp_channel);
                    TArtist = "";
                    Album = "";
                }
                catch { }
            }
        }

        public void Resume()
        {
            if (Play)
            {
                Play = false;
                Pause = true;
                Stop = false;
                Bass.BASS_Pause();
            }
            else
            {
                Play = true;
                Pause = false;
                Stop = false;
                Bass.BASS_Start();
            }
        }

        public void StopSong()
        {
            Play = false;
            Pause = false;
            Stop = true;
            Bass.BASS_ChannelStop(channel);
            try
            {
                Bass.BASS_StreamFree(channel);
                Bass.BASS_MusicFree(channel);
            }
            catch { }
        }

        public bool IsMOD(string path)
        {
            bool ret = false;
            string ext = System.IO.Path.GetExtension(path);
            switch (ext.ToLower())
            {
                case ".xm":
                    ret = true;
                    break;
                case ".it":
                    ret = true;
                    break;
                case ".mod":
                    ret = true;
                    break;
                case ".mtm":
                    ret = true;
                    break;
                case ".s3m":
                    ret = true;
                    break;
                case ".umx":
                    ret = true;
                    break;
            }
            return ret;
        }

        public bool CanIPlay(string path)
        {
            bool ret = false;
            string ext = System.IO.Path.GetExtension(path);
            switch (ext.ToLower())
            {
                case ".mp3":
                    ret = true;
                    break;
                case ".mp2":
                    ret = true;
                    break;
                case ".mp1":
                    ret = true;
                    break;
                case ".ogg":
                    ret = true;
                    break;
                case ".wav":
                    ret = true;
                    break;
                case ".aif":
                    ret = true;
                    break;
                case ".mo3":
                    ret = true;
                    break;
                case ".it":
                    ret = true;
                    break;
                case ".xm":
                    ret = true;
                    break;
                case ".s3m":
                    ret = true;
                    break;
                case ".mtm":
                    ret = true;
                    break;
                case ".mod":
                    ret = true;
                    break;
                case ".umx":
                    ret = true;
                    break;
                case ".cda":
                    ret = true;
                    break;
                case ".fla":
                    ret = true;
                    break;
                case ".flac":
                    ret = true;
                    break;
                case ".oga":
                    ret = true;
                    break;
                case ".wma":
                    ret = true;
                    break;
                case ".wv":
                    ret = true;
                    break;
                case ".aac":
                    ret = true;
                    break;
                case ".m4a":
                    ret = true;
                    break;
                case ".m4b":
                    ret = true;
                    break;
                case ".mp4":
                    ret = true;
                    break;
                case ".ac3":
                    ret = true;
                    break;
                case ".adx":
                    ret = true;
                    break;
                case ".aix":
                    ret = true;
                    break;
                case ".ape":
                    ret = true;
                    break;
                case ".mac":
                    ret = true;
                    break;
                case ".mpc":
                    ret = true;
                    break;
                case ".mp+":
                    ret = true;
                    break;
                case ".mpp":
                    ret = true;
                    break;
                case ".ofr":
                    ret = true;
                    break;
                case ".ofs":
                    ret = true;
                    break;
                case ".tta":
                    ret = true;
                    break;
            }
            return ret;
        }
    }
}
