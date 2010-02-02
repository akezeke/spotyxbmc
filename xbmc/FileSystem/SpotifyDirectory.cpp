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

#include "SpotifyDirectory.h"
#include "MusicDatabase.h"
#include "URL.h"
#include "FileItem.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"
#include "spotinterface.h"
#include "GUIWindowManager.h"

using namespace XFILE;


CMusicSpotifyDirectory::CMusicSpotifyDirectory(void)
{
}

CMusicSpotifyDirectory::~CMusicSpotifyDirectory(void)
{
}

bool CMusicSpotifyDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items)
{
  CLog::Log(LOGERROR, "  Spotifylog: get directory %s ", strPath.c_str());

  //new search without input dialog
  if (strPath.Left(20) == "spotify://didyoumean")
  {
    if (g_spotifyInterface->reconnect())
    {
       CStdString spotifySearch = strPath;
       spotifySearch.Delete(0, 20);
       CUtil::RemoveSlashAtEnd(spotifySearch);
       g_spotifyInterface->search(spotifySearch);
    }
    return true;
  }

  // searchmenu
  if (strPath.Left(20) == "spotify://searchmenu")
  {
      if (g_spotifyInterface->reconnect() && !g_spotifyInterface->hasSearchResults())
      {
          CStdString spotifySearch ="";
          spotifySearch = g_spotifyInterface->getSearchString();
          CLog::Log(LOGERROR, "Spotifylog: search");
          if (spotifySearch !="")
              return g_spotifyInterface->search(spotifySearch);
      }
      else
      {
          g_spotifyInterface->getSearchMenuItems(items);
      }
      return true;
  }

  // newsearch
    else if (strPath.Left(19) == "spotify://newsearch")
    {
        if (g_spotifyInterface->reconnect())
        {
            CStdString spotifySearch ="";
            spotifySearch = g_spotifyInterface->getSearchString();
            CLog::Log(LOGERROR, "  Spotifylog: search");
            if (spotifySearch !="")
            //GetDirectory(CUtil::GetParentPath(strPath), items);
           // Update(CUtil::GetParentPath(strPath));


                      //CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
      //  message.SetStringParam("spotify://searchmenu/");
      //  g_windowManager.SendThreadMessage(message);

            g_spotifyInterface->search(spotifySearch);
        }
        return true;
    }

    //disconnect
    else if (strPath.Left(20) == "spotify://disconnect")
    {
        g_spotifyInterface->disconnect();
        CFileItemList items;
        CStdString strPath = "spotify://menu";
        GetDirectory(strPath, items);

        //Update(strPath);
        return true;
    }

    //connect
    else if (strPath.Left(17) == "spotify://connect")
    {
        g_spotifyInterface->reconnect(false);
        CFileItemList items;
        CStdString strPath = "spotify://menu";
        GetDirectory(strPath, items);
        //Update(strPath);
        return true;
    }

    //reconnect
    else if (strPath.Left(19) == "spotify://reconnect")
    {
        g_spotifyInterface->reconnect(true);
        CFileItemList items;
        CStdString strPath = "spotify://settings";
        //GetDirectory(strPath, items);
       // Update(strPath);
        return true;
    }

    //add an album to the library
    else if (strPath.Left(18) == "spotify://addalbum")
    {
        //g_spotifyInterface->getMainMenuItems(items);
        g_spotifyInterface->addAlbumToLibrary();
        return true;
    }

    else if (strPath.Left(14)=="spotify://menu")
    {
        g_spotifyInterface->getMainMenuItems(items);
    }

    else  if (strPath.Left(19)=="spotify://playlists")
    {
        g_spotifyInterface->getPlaylistItems(items);
    }

    else  if (strPath.Left(18)=="spotify://toplists")
    {
        g_spotifyInterface->getToplistItems(items);
    }

    else  if (strPath.Left(18)=="spotify://settings")
    {
        g_spotifyInterface->getSettingsMenuItems(items);
    }

    else if (strPath.Left(20)=="spotify://disclaimer")
    {
        g_spotifyInterface->showDisclaimer();
    }

    else
    {
        return false;
    }
    return true;
}

bool CMusicSpotifyDirectory::Exists(const char* strPath)
{
    return true;
}

