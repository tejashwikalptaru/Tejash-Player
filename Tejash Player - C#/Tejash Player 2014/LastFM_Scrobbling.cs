using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Lpfm.LastFmScrobbler;
using Lpfm.LastFmScrobbler.Api;
using System.Threading;

namespace Tejash_Player_2014
{
    class LastFM_Scrobbling
    {
        private const string ApiKey = "c9d1ac2ce697c1797e951d324b71a363";
        private const string ApiSecret = "981de7ebcb25cb7fca23480bd8882654";
        private readonly QueuingScrobbler _scrobbler;
        private IniFile ini;
        public bool isReady = false;
        private Track CurrentTrack;

        public LastFM_Scrobbling(string appsettings_path)
        {
            ini = new IniFile(appsettings_path);
            try
            {
                string sessionKey = GetSessionKey();

                // instantiate the async scrobbler
                if(isReady)
                    _scrobbler = new QueuingScrobbler(ApiKey, ApiSecret, sessionKey);

            }
            catch (Exception exception)
            {
                MessageBox.Show("Unable to connect to Last.FM!\n" + exception.Message,"Error",MessageBoxButtons.OK,MessageBoxIcon.Error);
                throw exception;
            }
        }

        private string GetSessionKey()
        {
            // try get the session key from the registry
            string sessionKey = GetRegistrySetting();

            if (string.IsNullOrEmpty(sessionKey))
            {
                // instantiate a new scrobbler
                var scrobbler = new Scrobbler(ApiKey, ApiSecret);

                //NOTE: This is demo code. You would not normally do this in a production application
                while (string.IsNullOrEmpty(sessionKey))
                {
                    // Try get session key from Last.fm
                    try
                    {
                        sessionKey = scrobbler.GetSession();

                        // successfully got a key. Save it to the registry for next time
                        SetRegistrySetting(sessionKey);

                        isReady = true;
                    }
                    catch (LastFmApiException exception)
                    {
                        // get a url to authenticate this application
                        string url = scrobbler.GetAuthorisationUri();

                        // Block this application while the user authenticates
                        if (MessageBox.Show("The application is not authorized to use the LastFM!\nLogin to your LastFM account and please authorize to use this feature.\nDo you want to authorize now?\n\nAdditional details:\n" + exception.Message, "Authentication needed", MessageBoxButtons.YesNo) == DialogResult.Yes)
                        {
                            // open the URL in the default browser
                            System.Diagnostics.Process.Start(url);
                            if (MessageBox.Show("Click OK when authorized!", "Wait...", MessageBoxButtons.OK, MessageBoxIcon.Hand) != DialogResult.OK)
                            {
                                isReady = false;
                                break;
                            }
                            else
                                isReady = true;
                        }
                        else
                        {
                            isReady = false;
                            break;
                        }

                    }
                }
            }
            else
                isReady = true;
            return sessionKey;
        }

        private string GetRegistrySetting()
        {
            return ini.IniReadValue("LastFM", "S-Key");
        }

        private void SetRegistrySetting(string value)
        {
            ini.IniWriteValue("LastFM", "S-Key", value);
        }

        private Track PrepareTrack(string Title,string Album, string Artist,double Seconds)
        {
            var track = new Track
            {
                TrackName = Title,
                AlbumName = Album,
                ArtistName = Artist,
                TrackNumber = (int)1,
                Duration = new TimeSpan(0,0,(int)Seconds)
            };
            return track;
        }

        private delegate void ProcessScrobblesDelegate();
        private void ProcessScrobbles()
        {
            // Processes the scrobbles and discards any responses. This could be improved with thread-safe
            //  logging and/or error handling
            _scrobbler.Process();
        }

        public void Scrobbling(string Title,string Album,string Artist,double Seconds)
        {
            var doProcessScrobbles = new ProcessScrobblesDelegate(ProcessScrobbles);

            CurrentTrack = PrepareTrack(Title, Album, Artist,Seconds);
            CurrentTrack.WhenStartedPlaying = DateTime.Now;

            // we are using the Queuing scrobbler here so that we don't block the form while the scrobble request is being sent
            //  to the Last.fm web service. The request will be sent when the Process() method is invoked
            _scrobbler.NowPlaying(CurrentTrack);
            // Begin invoke with no callback fires and forgets the scrobbler process. Processing runs asynchronously while 
            //  the form thread continues
            doProcessScrobbles.BeginInvoke(null, null);   
        }
    }
}
