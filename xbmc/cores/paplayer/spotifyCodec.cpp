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

//ugly as hell!
SpotifyCodec *SpotifyCodec::m_currentPlayer = 0;
bool SpotifyCodec::playerIsFree = true;

SpotifyCodec::SpotifyCodec()
{
  m_SampleRate = 44100;
  m_Channels = 2;
  m_BitsPerSample = 16;
  m_Bitrate = 320;
  m_CodecName = "spotify";
  m_TotalTime = 0;
  m_currentTrack = 0;
  m_isPlayerLoaded = false;
  m_buffer = 0;
  m_hasPlayer = false;
}

SpotifyCodec::~SpotifyCodec()
{
  DeInit();
  delete m_buffer;
}
void SpotifyCodec::DeInit()
{
  CLog::Log( LOGDEBUG, "Spotifylog: deinit");
  if (m_hasPlayer)
    unloadPlayer();
}

bool SpotifyCodec::Init(const CStdString &strFile1, unsigned int filecache)
{
  CLog::Log( LOGDEBUG, "Spotifylog: init");
  if (reconnect())
  {
    m_bufferSize = 2048 * sizeof(int16_t) * 2 * 10;
    m_buffer = new char[m_bufferSize];
    CStdString uri = CUtil::GetFileName(strFile1);
    //if its a song from our library we need to get the uri
    if (strFile1.Left(7) == "musicdb")
    {
      CFileMusicDatabase *musicDb = new CFileMusicDatabase();
      uri = CUtil::GetFileName(musicDb->TranslateUrl(strFile1));
      delete musicDb;
    }
    CUtil::RemoveExtension(uri);
    CLog::Log(LOGDEBUG, "Spotifylog: loading spotifyCodec, %s", uri.c_str());

    if (m_currentPlayer != 0)
      m_currentPlayer->DeInit();
    m_currentPlayer = this;

    sp_link *spLink = sp_link_create_from_string(uri.c_str());
    m_currentTrack = sp_link_as_track(spLink);
    if (!sp_track_is_available(m_currentTrack))
    {
      CLog::Log(LOGERROR, "Spotifylog: track is not available in this region");
      unloadPlayer();
      sp_link_release(spLink);
      return false;
    }
    sp_track_add_ref(m_currentTrack);
    sp_link_release(spLink);
    m_totalTime = 0.001 * sp_track_duration(m_currentTrack);
    m_endOfTrack = false;
    m_bufferPos = 0;
    m_startStream = false;
    m_isPlayerLoaded = false;
    playerIsFree = false;
    m_hasPlayer = true;
    loadPlayer();
    return true;
  }
  return false;
}


bool SpotifyCodec::loadPlayer()
{
  //CLog::Log( LOGDEBUG, "Spotifylog: music load player");
  if (reconnect())
  {
    //CLog::Log( LOGDEBUG, "Spotifylog: music load player2");
    if (!m_isPlayerLoaded)
    {
      //CLog::Log( LOGDEBUG, "Spotifylog: music load player3");
      //do we have a track at all?
      if (m_currentTrack)
      {
        CStdString name;
        name =  sp_track_name(m_currentTrack);
        CLog::Log( LOGDEBUG, "Spotifylog: music load player4, track: %s", name.c_str());
        if (sp_track_is_loaded(m_currentTrack))
        {
          //CLog::Log( LOGDEBUG, "Spotifylog: music load player5");
          sp_error error = sp_session_player_load (getSession(), m_currentTrack);
          CStdString message;
          message.Format("%s",sp_error_message(error));
          CLog::Log( LOGDEBUG, "Spotifylog: music load player5 errormsg %s", message.c_str());
          if(SP_ERROR_OK == error)
          {
            //CLog::Log( LOGDEBUG, "Spotifylog: music load player6");
            if(SP_ERROR_OK == sp_session_player_play (getSession(), true))
            {
              CLog::Log( LOGDEBUG, "Spotifylog: music load, play" );
              m_isPlayerLoaded = true;
              return true;
            }
          }
        }
      }else
        return false;
    }else
      return true;
  }
  return false;
}

bool SpotifyCodec::unloadPlayer()
{
  CLog::Log( LOGDEBUG, "Spotifylog: music unloadplayer1");
  if (m_isPlayerLoaded && m_hasPlayer)
  {
    CLog::Log( LOGDEBUG, "Spotifylog: music unloadplayer hasplayer");
    sp_session_player_play (getSession(), false);
    sp_session_player_unload (getSession());
    m_currentPlayer = 0;
    playerIsFree = true;
  }

  if (m_currentTrack)
  {
    sp_track_release(m_currentTrack);
  }

  m_currentTrack = 0;
  m_isPlayerLoaded = false;
  m_hasPlayer = false;
  m_endOfTrack = true;
  return true;
}

__int64 SpotifyCodec::Seek(__int64 iSeekTime)
{
  if (m_hasPlayer && !m_isPlayerLoaded)
    loadPlayer();

  if (m_isPlayerLoaded)
  {
    if (SP_ERROR_OK == sp_session_player_seek (getSession(), iSeekTime))
    {
      CLog::Log( LOGDEBUG, "Spotifylog: player seek, offset %i", (int)iSeekTime);
      m_bufferPos = 0;
      if (SP_ERROR_OK == sp_session_player_play (getSession(), true))
        return iSeekTime;
    }
  }
  CLog::Log( LOGDEBUG, "Spotifylog: player seek, return false. offset %i", (int)iSeekTime);
  return 0;
}

//music delivery callbacks
int SpotifyCodec::cb_musicDelivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames)
{
  //CLog::Log( LOGDEBUG, "Spotifylog: music delivery");
  if (!m_currentPlayer)
  {
    sp_session_player_play (g_spotifyInterface->getSession(), false);
    sp_session_player_unload (g_spotifyInterface->getSession());
    return 0;
  }
  //CLog::Log( LOGDEBUG, "Spotifylog: music delivery");
  int amountToMove = num_frames * (int)sizeof(int16_t) * format->channels;

  if ((m_currentPlayer->m_bufferPos +  amountToMove) >= m_currentPlayer->m_bufferSize)
  {
    amountToMove = m_currentPlayer->m_bufferSize - m_currentPlayer->m_bufferPos;
    //now the buffer is full, start playing
    m_currentPlayer->m_startStream = true;
  }
  m_currentPlayer->m_channels = format->channels;
  m_currentPlayer->m_sampleRate = format->sample_rate;
  memcpy (m_currentPlayer->m_buffer + m_currentPlayer->m_bufferPos, frames, amountToMove);
  m_currentPlayer->m_bufferPos += amountToMove;

  return amountToMove / ((int)sizeof(int16_t) * format->channels);
}

void SpotifyCodec::cb_endOfTrack(sp_session *sess)
{
  //CLog::Log( LOGDEBUG, "Spotifylog: music endoftrack callback");
  if (!m_currentPlayer)
  {
    sp_session_player_play (g_spotifyInterface->getSession(), false);
    sp_session_player_unload (g_spotifyInterface->getSession());
    return;
  }
  m_currentPlayer->m_endOfTrack = true;
  playerIsFree = true;
}

int SpotifyCodec::ReadPCM(BYTE *pBuffer, int size, int *actualsize)
{
  //CLog::Log( LOGDEBUG, "Spotifylog: readpcm");
  *actualsize = 0;
  if (m_hasPlayer && !m_isPlayerLoaded)
    loadPlayer();

  if (m_startStream)
  {
    if (m_endOfTrack && m_bufferPos == 0)
    {
      return READ_EOF;
    }
    else if (m_bufferPos > 0)
    {
      int amountToMove = m_bufferPos;
      if (m_bufferPos > size)
        amountToMove = size;
      memcpy (pBuffer, m_buffer, amountToMove);
      memmove(m_buffer, m_buffer + amountToMove, m_bufferSize - amountToMove);
      m_bufferPos -= amountToMove;
      *actualsize = amountToMove;
    }
  }
  return READ_SUCCESS;
}

bool SpotifyCodec::CanInit()
{
  return reconnect();
}

