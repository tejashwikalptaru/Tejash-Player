/*

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

                        MP3FileInfo 1.0
                    -----------------------

  MP3FileInfo is a wrapper class for id3lib, a library capable
  of reading and writing ID3 Tags of MP3 Files. It has a
  simple interface and provides all data as public members.

  Written by Christian Richardt (cr@whizer.net).

  Release history:
    Mai 31, 2003: Version 1.0. First release.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*/

/**
  \file    MP3FileInfo.h
  \brief   Reads Header and ID3 Tags of MP3 Files
  \date    2003-05-31
  \author  Christian Richardt (cr@whizer.net)
**/

#ifndef MP3FILEINFO_H
#define MP3FILEINFO_H


/* ---- use id3lib ---- */

    // "If you use id3lib.dll (you link dynamic) you should add
    //  ID3LIB_LINKOPTION=3 to your preprocessor definitions of your project."
  #define ID3LIB_LINKOPTION 3
  #include "id3/tag.h"
  #pragma comment(lib, "id3lib.lib")


    /**
      \brief   Reads Header and ID3 Tags of MP3 Files

      MP3FileInfo is a wrapper class for id3lib, a library capable
      of reading and writing ID3 Tags of MP3 Files. It has a
      simple interface and provides all data as public members.

      How to use MP3FileInfo:

      \code
        MP3FileInfo obj; // don't use at this point!

        if(!obj.Init("some-file.mp3"))
         { ... error ... }

        ... use MP3FileInfo ...
      \end

      The object frees all allocated memory if deleted. Always
      Initalize this class by calling Init(). It returns false if
      an error occured. You can call Init() as often as you want.
      All data is being freed before the data of the new file is
      extracted.

      Always call isOK() to check whether the class is all right!
    **/
  class MP3FileInfo
   {
    public:
      inline MP3FileInfo(void) :
        m_bOK(true),
        m_id3tag(0),
        m_mp3header(0),

          // Fileinfo
        szFilename(0), nFileSize(-1),
        bHasLyrics(false), bHasV1Tag(false), bHasV2Tag(false),

          // Headerinfo
        nBitRate(0), nCbrBitRate(0), nVbrBitRate(0), nSampleRate(0), nLength(0),
        szBitRate(0), szMpegLayer(0), szMpegVersion(0), szChannelMode(0),

          // ID3V1 Tag elements
        szArtist(0), szAlbum(0), szGenre(0), szTitle(0), szYear(0), szComment(0), szTrack(0),

          // ID3V2 Tag elements
        szComposer(0), szCopyright(0), szEncodedBy(0), szOriginalArtist(0), szURL(0)
       {
        // nothing to do; use Init() instead
       }

      virtual ~MP3FileInfo(void) { Free(); }


      bool Init(const char* cszFilename);
      void Free(void);
      inline bool isOK(void) const { return m_bOK; }


      /* Fileinfo */
      char* szFilename;
      bool  bHasLyrics, bHasV1Tag, bHasV2Tag;
      int   nFileSize;

      /* Headerinfo */
      int nCbrBitRate, nVbrBitRate, nBitRate, nSampleRate, nLength;
      char* szBitRate;
      char* szMpegLayer;
      char* szMpegVersion;
      char* szChannelMode;

      /* V1 up */
      char* szArtist;
      char* szTitle;
      char* szAlbum;
      char* szComment;
      char* szTrack; int nTrack;
      char* szYear; int nYear;
      char* szGenre;

      /* V2 only */
      char* szComposer;
      char* szCopyright;
      char* szEncodedBy;
      char* szOriginalArtist;
      char* szURL;

      void  ShowAllFrames(void);


    protected:
      bool m_bOK;

      ID3_Tag* m_id3tag;
      const Mp3_Headerinfo* m_mp3header;

      void m_parseFile(void);
      void m_parseHeader(void);
      void m_parseGenre(void);

      bool  m_getString(ID3_FrameID fid, char* szBuffer);
      char* m_getNewString(ID3_FrameID fid);
   };



#endif // MP3FILEINFO_H