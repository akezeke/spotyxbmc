  /*
    spotyxbmc - A project to integrate Spotify into XBMC
    Copyright (C) 2010  David Erenger

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    For contact with the author:
    david.erenger@gmail.com
*/

#include "spotifyCodec.h"
#include "FileSystem/FileMusicDatabase.h"
#include "Util.h"
#include "utils/log.h"

using namespace MUSIC_INFO;
using namespace XFILE;

SpotifyCodec::SpotifyCodec()
{
    m_SampleRate = 44100;
    m_Channels = 2;
    m_BitsPerSample = 16;
    m_Bitrate = 320;
    m_CodecName = "spotify";
    m_TotalTime = 0;
}

SpotifyCodec::~SpotifyCodec()
{
    DeInit();
}

bool SpotifyCodec::Init(const CStdString &strFile1, unsigned int filecache)
{
    CStdString uri = CUtil::GetFileName(strFile1);
    //if its a song from our library we need to get the uri
    if (strFile1.Left(7) == "musicdb")
    {
        CFileMusicDatabase *musicDb = new CFileMusicDatabase();
        uri = CUtil::GetFileName(musicDb->TranslateUrl(strFile1));
        delete musicDb;
    }
    CUtil::RemoveExtension(uri);
    CLog::Log(LOGERROR, "Spotifylog: loading spotifyCodec, %s", uri.c_str());
    m_isInit = g_spotifyInterface->spotifyPlayerLoad(uri, m_TotalTime);
    return m_isInit;
}

void SpotifyCodec::DeInit()
{
    if (m_isInit)
    {
        g_spotifyInterface->spotifyPlayerUnload();
        m_isInit = false;
    }
}

__int64 SpotifyCodec::Seek(__int64 iSeekTime)
{
    return g_spotifyInterface->spotifyPlayerSeek(iSeekTime);
}

int SpotifyCodec::ReadPCM(BYTE *pBuffer, int size, int *actualsize)
{
    bool endOfTrack = false;
    //get the data from the spotify interface
    *actualsize = g_spotifyInterface->spotifyPlayerGetFrames((char*)pBuffer, size, m_Channels, m_BitsPerSample, m_Bitrate, endOfTrack);
    if (endOfTrack)
    {
        //CLog::Log(LOGERROR, "Spotifylog: spotifyCodec endoftrack");
        return READ_EOF;
    }
    return READ_SUCCESS;
}

bool SpotifyCodec::CanInit()
{
    return true;
}
