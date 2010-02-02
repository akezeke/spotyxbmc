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

#pragma once

#ifndef SP_CALLCONV
#ifdef _WIN32
#define SP_CALLCONV __stdcall
#else
#define SP_CALLCONV
#endif
#endif

#include <spotify/api.h>
#include <stdio.h>
#include <stdint.h>
#include <cstdlib>
#include <vector>
#include "StringUtils.h"
#include "GUIDialogProgress.h"
#include "GUIDialogOK.h"
#include "GUIDialogKeyboard.h"
#include "Util.h"
#include "FileItem.h"
#include "utils/TimeUtils.h"
#include "GUIDialog.h"

class SpotifyInterface
{
public:
    SpotifyInterface();
    ~SpotifyInterface();

    enum SPOTIFY_TYPE{
        SEARCH_ARTIST,
        SEARCH_ALBUM,
        SEARCH_TRACK,
        ARTISTBROWSE_ALBUM,
        ALBUMBROWSE_TRACK,
        PLAYLIST_TRACK,
        TOPLIST_ALBUM,
        TOPLIST_TRACK
    };

    //session functions
    bool connect(bool forceNewUser = false);
    bool disconnect();
    bool reconnect(bool forceNewUser = false);
    bool processEvents();

    //callback functions definied in api.h
    static void SP_CALLCONV cb_connectionError(sp_session *session, sp_error error);
    static void SP_CALLCONV cb_loggedIn(sp_session *session, sp_error error);
    static void SP_CALLCONV cb_notifyMainThread(sp_session *session);
    static void SP_CALLCONV cb_logMessage(sp_session *session, const char *data);
    static void SP_CALLCONV cb_searchComplete(sp_search *search, void *userdata);
    static void SP_CALLCONV cb_albumBrowseComplete(sp_albumbrowse *result, void *userdata);
    static void SP_CALLCONV cb_artistBrowseComplete(sp_artistbrowse *result, void *userdata);
    static int SP_CALLCONV cb_musicDelivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames);
    static void SP_CALLCONV cb_endOfTrack(sp_session *sess);
    static void SP_CALLCONV cb_imageLoaded(sp_image *image, void *userdata);

    //functions for searching
    bool search(CStdString searchstring);
    bool hasSearchResults() { return (!m_searchArtistVector.IsEmpty() || !m_searchAlbumVector.IsEmpty() || !m_searchTrackVector.IsEmpty() || m_didYouMeanStr != "");}
    void getSearchArtists(CFileItemList &items) { items.Append(m_searchArtistVector); }
    void getSearchAlbums(CFileItemList &items) { items.Append(m_searchAlbumVector); }
    void getSearchTracks(CFileItemList &items) { items.Append(m_searchTrackVector); }

    void getMainMenuItems(CFileItemList &items);
    void getSettingsMenuItems(CFileItemList &items);
    void getSearchMenuItems(CFileItemList &items);
    void getPlaylistItems(CFileItemList &items);
    void getToplistItems(CFileItemList &items);

    //functions for browsing
    bool browseArtist(CStdString uri);
    bool browseAlbum(CStdString uri);
    void getBrowseAlbumTracks(CFileItemList &items) { items.Append(m_browseAlbumVector); m_isBrowsingAlbum = false; }
    void getBrowseArtistAlbums(CFileItemList &items) { items.Append(m_browseArtistVector); m_isBrowsingArtist = false; }
    bool addAlbumToLibrary();
    bool isBrowsingAlbum() { return m_isBrowsingAlbum; }
    bool isBrowsingArtist() { return m_isBrowsingArtist; }

    //handling playlists
    bool getPlaylistTracks(CFileItemList &items, int playlist);

    //functions to handle the audio
    bool spotifyPlayerLoad(CStdString trackURI, __int64 &totalTime);
    bool spotifyPlayerStart();
    bool spotifyPlayerUnload();
    int spotifyPlayerSeek(int offset);
    bool spotifyPlayerIsFree(){ return (m_endOfTrack && m_bufferPos == 0) || !m_isPlayerLoaded;}
    int spotifyPlayerGetFrames( char* pBuffer, int size, int &channels, int &bitsPerSample, int &bitrate, bool &endOfTrack);

private:
    CGUIDialogProgress *searchProgress;
    sp_session_config m_config;
    sp_error m_error;
    sp_session *m_session;
    sp_session_callbacks m_callbacks;
    bool m_showDisclaimer;
    int m_nextEvent;
    const char *m_uri;
    CStdString m_thumbDir;
    CStdString m_playlistsThumbDir;
    CStdString m_toplistsThumbDir;
    CStdString m_currentPlayingDir;

    void clean(bool search, bool artistbrowse, bool albumbrowse, bool playlists, bool toplists, bool searchthumbs, bool playliststhumbs, bool toplistthumbs, bool currentplayingthumbs);

    void clean();

    //usefull dialogs
    CGUIDialogOK *m_reconectingDialog;
    CGUIDialogProgress *m_progressDialog;

    //dialog functions
    CStdString getUsername();
    CStdString getPassword();
public:
    CStdString getSearchString();
    void showDisclaimer();
private:
    bool m_isShowingReconnect;
    void showReconectingDialog();
    void hideReconectingDialog();
    void showProgressDialog(CStdString message);
    void hideProgressDialog();
    void showConnectionErrorDialog(sp_error error);

    //search
    sp_search *m_search;
    CFileItemList m_searchArtistVector;
    CFileItemList m_searchAlbumVector;
    CFileItemList m_searchTrackVector;
    CStdString m_didYouMeanStr;

    //browsing
    bool m_isBrowsingArtist;
    bool m_isBrowsingAlbum;
    CStdString m_artistBrowseStr;
    CStdString m_albumBrowseStr;
    sp_artistbrowse *m_artistBrowse;
    sp_albumbrowse *m_albumBrowse;
    CFileItemList m_browseAlbumVector;
    CFileItemList m_browseArtistVector;

    //playlists
    CFileItemList m_playlistItems;

    //converting functions
    CFileItemPtr spArtistToItem(sp_artist *spArtist);
    CFileItemPtr spAlbumToItem(sp_album *spAlbum, SPOTIFY_TYPE type);
    CFileItemPtr spTrackToItem(sp_track *spTrack, SPOTIFY_TYPE type, bool loadthumb = false);

    //thumbnail handling
    /*typedef std::pair<sp_image*,CFileItemPtr> imageItemPair;
    std::vector<imageItemPair> m_searchWaitingThumbs;
    std::vector<imageItemPair> m_artistWaitingThumbs;
    std::vector<imageItemPair> m_playlistWaitingThumbs;
    std::vector<imageItemPair> m_toplistWaitingThumbs;*/
    bool requestThumb(unsigned char *imageId, CStdString Uri, CFileItemPtr pItem, SPOTIFY_TYPE type);

    //player
    sp_track *m_currentTrack;
    bool m_startStream;
    int m_instances;
    int m_sampleRate;
    int m_channels;
    int m_bitsPerSample;
    int m_bitrate;
    int64_t m_totalTime;
    bool m_isPlayerLoaded;
    bool m_endOfTrack;
    int m_bufferSize;
    char *m_buffer;
    int m_bufferPos;
    bool loadPlayer();
    bool unloadPlayer();
};

extern SpotifyInterface *g_spotifyInterface;

