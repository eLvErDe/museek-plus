/*  Museek - A SoulSeek client written in C++
    Copyright (C) 2006-2007 Ingmar K. Steen (iksteen@gmail.com)
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

MAP_MESSAGE(0x0000, IPing, pingEvent)
MAP_MESSAGE(0x0002, ILogin, loginEvent)
MAP_MESSAGE(0x0004, ICheckPrivileges, checkPrivilegesEvent)
MAP_MESSAGE(0x0005, ISetStatus, setStatusEvent)
MAP_C_MESSAGE(0x0012, INewPassword, newPasswordEvent)

// These messages require the crypto context
MAP_C_MESSAGE(0x0101, IConfigSet, setConfigEvent)
MAP_C_MESSAGE(0x0102, IConfigRemove, removeConfigEvent)
MAP_MESSAGE(0x0103, IConfigSetUserImage, setUserImageEvent)

MAP_MESSAGE(0x0201, IPeerExists, getPeerExistsEvent)
MAP_MESSAGE(0x0202, IPeerStatus, getPeerStatusEvent)
MAP_MESSAGE(0x0203, IPeerStats, getPeerStatsEvent)
MAP_MESSAGE(0x0204, IUserInfo, getUserInfoEvent)
MAP_MESSAGE(0x0205, IUserShares, getUserSharesEvent)
MAP_MESSAGE(0x0206, IPeerAddress, getPeerAddressEvent)
MAP_MESSAGE(0x0207, IGivePrivileges, givePrivilegesEvent)

MAP_MESSAGE(0x0301, IRoomList, getRoomListEvent)
MAP_MESSAGE(0x0302, IPrivateMessage, sendPrivateMessageEvent)
MAP_MESSAGE(0x0303, IJoinRoom, joinRoomEvent)
MAP_MESSAGE(0x0304, ILeaveRoom, leaveRoomEvent)
MAP_MESSAGE(0x0307, ISayRoom, sayRoomEvent)
MAP_MESSAGE(0x0309, IRoomTickerSet, setRoomTickerEvent)
MAP_MESSAGE(0x0310, IMessageUsers, messageUsersEvent)
MAP_MESSAGE(0x0311, IMessageBuddies, messageBuddiesEvent)
MAP_MESSAGE(0x0312, IMessageDownloading, messageDownloadingEvent)
MAP_MESSAGE(0x0313, IAskPublicChat, askPublicChatEvent)
MAP_MESSAGE(0x0314, IStopPublicChat, stopPublicChatEvent)
MAP_MESSAGE(0x0320, IPrivRoomToggle, privRoomToggleEvent)
MAP_MESSAGE(0x0322, IPrivRoomAddUser, privRoomAddUserEvent)
MAP_MESSAGE(0x0323, IPrivRoomRemoveUser, privRoomRemoveUserEvent)
MAP_MESSAGE(0x0328, IPrivRoomAddOperator, privRoomAddOperatorEvent)
MAP_MESSAGE(0x0329, IPrivRoomRemoveOperator, privRoomRemoveOperatorEvent)
MAP_MESSAGE(0x0330, IPrivRoomDismember, privRoomDismemberEvent)
MAP_MESSAGE(0x0331, IPrivRoomDisown, privRoomDisownEvent)

MAP_MESSAGE(0x0401, ISearch, startGlobalSearchEvent)
MAP_MESSAGE(0x0402, ISearchReply, stopSearchEvent)
MAP_MESSAGE(0x0403, IUserSearch, startUserSearchEvent)
MAP_MESSAGE(0x0405, IWishListSearch, startWishListSearchEvent)
MAP_MESSAGE(0x0406, IAddWishItem, addWishItemEvent)
MAP_MESSAGE(0x0407, IRemoveWishItem, removeWishItemEvent)

MAP_MESSAGE(0x0501, ITransferUpdate, updateTransferEvent)
MAP_MESSAGE(0x0502, ITransferRemove, removeTransferEvent)
MAP_MESSAGE(0x0503, IDownloadFile, downloadFileEvent)
MAP_MESSAGE(0x0507, IDownloadFileTo, downloadFileToEvent)
MAP_MESSAGE(0x0504, IDownloadFolder, downloadFolderEvent)
MAP_MESSAGE(0x0508, IDownloadFolderTo, downloadFolderToEvent)
MAP_MESSAGE(0x0505, ITransferAbort, abortTransferEvent)
MAP_MESSAGE(0x0509, IUploadFolder, uploadFolderEvent)
MAP_MESSAGE(0x0506, IUploadFile, uploadFileEvent)

MAP_MESSAGE(0x0600, IGetRecommendations, getRecommendationsEvent)
MAP_MESSAGE(0x0601, IGetGlobalRecommendations, getGlobalRecommendationsEvent)
MAP_MESSAGE(0x0602, IGetSimilarUsers, getSimilarUsersEvent)
MAP_MESSAGE(0x0603, IGetItemRecommendations, getItemRecommendationsEvent)
MAP_MESSAGE(0x0604, IGetItemSimilarUsers, getItemSimilarUsersEvent)
MAP_MESSAGE(0x0610, IAddInterest, addInterestEvent)
MAP_MESSAGE(0x0611, IRemoveInterest, removeInterestEvent)
MAP_MESSAGE(0x0612, IAddHatedInterest, addHatedInterestEvent)
MAP_MESSAGE(0x0613, IRemoveHatedInterest, removeHatedInterestEvent)
MAP_MESSAGE(0x0614, IUserInterests, getUserInterestsEvent)

MAP_MESSAGE(0x0700, IConnectServer, connectToServerEvent)
MAP_MESSAGE(0x0701, IDisconnectServer, disconnectFromServerEvent)
MAP_MESSAGE(0x0703, IReloadShares, reloadSharesEvent)
