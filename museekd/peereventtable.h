/*  Museek - A SoulSeek client written in C++
    Copyright (C) 2006-2007 Ingmar K. Steen (iksteen@gmail.com)
    Copyright (C) 2006-2007 daelstorm (daelstorm@gmail.com)
    Copyright 2008 little blue poney <lbponey@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

MAP_MESSAGE(4, PSharesRequest, sharesRequestedEvent)
MAP_MESSAGE(5, PSharesReply, sharesReceivedEvent)
MAP_MESSAGE(8, PSearchRequest, searchRequestedEvent)
MAP_MESSAGE(9, PSearchReply, searchResultsReceivedEvent)
MAP_MESSAGE(15, PInfoRequest, infoRequestedEvent)
MAP_MESSAGE(16, PInfoReply, infoReceivedEvent)
MAP_MESSAGE(36, PFolderContentsRequest, folderContentsRequestedEvent)
MAP_MESSAGE(37, PFolderContentsReply, folderContentsReceivedEvent)
MAP_MESSAGE(40, PTransferRequest, transferRequestedEvent)
MAP_MESSAGE(41, PTransferReply, transferReplyReceivedEvent)
MAP_MESSAGE(42, PUploadPlacehold, uploadPlaceholdRequestedEvent)
MAP_MESSAGE(43, PQueueDownload, queueDownloadRequestedEvent)
MAP_MESSAGE(44, PPlaceInQueueReply, placeInQueueReceivedEvent)
MAP_MESSAGE(46, PUploadFailed, uploadFailedEvent)
MAP_MESSAGE(50, PQueueFailed, queueFailedEvent)
MAP_MESSAGE(51, PPlaceInQueueRequest, placeInQueueRequestedEvent)
MAP_MESSAGE(52, PUploadQueueNotification, uploadQueueNotificationReceivedEvent)
