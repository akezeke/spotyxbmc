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

#include "CachingCodec.h"
#include "spotinterface.h"

class SpotifyCodec : public CachingCodec
{
public:
    SpotifyCodec();
    virtual ~SpotifyCodec();

    virtual bool Init(const CStdString &strFile, unsigned int filecache);
    virtual void DeInit();
    virtual __int64 Seek(__int64 iSeekTime);
    virtual int ReadPCM(BYTE *pBuffer, int size, int *actualsize);
    virtual bool CanInit();
private:
    bool m_isInit;
};
