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

MAP_MESSAGE(1, SLogin, loggedInEvent)
MAP_MESSAGE(3, SGetPeerAddress, peerAddressReceivedEvent)
MAP_MESSAGE(5, SAddUser, addUserReceivedEvent)
MAP_MESSAGE(7, SGetStatus, userStatusReceivedEvent)
MAP_MESSAGE(13, SSayRoom, roomMessageReceivedEvent)
MAP_MESSAGE(14, SJoinRoom, roomJoinedEvent)
MAP_MESSAGE(15, SLeaveRoom, roomLeftEvent)
MAP_MESSAGE(16, SUserJoinedRoom, userJoinedRoomEvent)
MAP_MESSAGE(17, SUserLeftRoom, userLeftRoomEvent)
MAP_MESSAGE(18, SConnectToPeer, connectToPeerRequestedEvent)
MAP_MESSAGE(22, SPrivateMessage, privateMessageReceivedEvent)
MAP_MESSAGE(26, SFileSearch, fileSearchRequestedEvent)
MAP_MESSAGE(32, SPing, pingReceivedEvent)
MAP_MESSAGE(36, SGetUserStats, userStatsReceivedEvent) // Deprecated
MAP_MESSAGE(41, SKicked, kickedEvent)
MAP_MESSAGE(42, SUserSearch, userSearchRequestedEvent)
MAP_MESSAGE(54, SGetRecommendations, recommendationsReceivedEvent)
MAP_MESSAGE(56, SGetGlobalRecommendations, globalRecommendationsReceivedEvent)
MAP_MESSAGE(57, SUserInterests, userInterestsReceivedEvent)
MAP_MESSAGE(64, SRoomList, roomListReceivedEvent)
MAP_MESSAGE(65, SExactFileSearch, exactFileSearchRequestedEvent)
MAP_MESSAGE(66, SGlobalMessage, globalMessageReceivedEvent)
MAP_MESSAGE(69, SPrivilegedUsers, privilegedUsersReceivedEvent)
MAP_MESSAGE(83, SParentMinSpeed, parrentMinSpeedReceivedEvent)
MAP_MESSAGE(84, SParentSpeedRatio, parentSpeedRatioReceivedEvent)
MAP_MESSAGE(86, SParentInactivityTimeout, parentInactivityTimeoutReceivedEvent)
MAP_MESSAGE(87, SSearchInactivityTimeout, searchInactivityTimeoutReceivedEvent)
MAP_MESSAGE(88, SMinParentsInCache, minParentsInCacheReceivedEvent)
MAP_MESSAGE(90, SDistribAliveInterval, distribAliveIntervalReceivedEvent)
MAP_MESSAGE(91, SAddPrivileged, privilegedUserAddedEvent)
MAP_MESSAGE(92, SCheckPrivileges, privilegesReceivedEvent)
MAP_MESSAGE(93, SSearchRequest, searchRequestedEvent)
MAP_MESSAGE(102, SNetInfo, netInfoReceivedEvent)
MAP_MESSAGE(104, SWishlistInterval, wishlistIntervalReceivedEvent)
MAP_MESSAGE(110, SGetSimilarUsers, similarUsersReceivedEvent)
MAP_MESSAGE(111, SGetItemRecommendations, itemRecommendationsReceivedEvent)
MAP_MESSAGE(112, SGetItemSimilarUsers, itemSimilarUsersReceivedEvent)
MAP_MESSAGE(113, SRoomTickers, roomTickersReceivedEvent)
MAP_MESSAGE(114, SRoomTickerAdd, roomTickerAddedEvent)
MAP_MESSAGE(115, SRoomTickerRemove, roomTickerRemovedEvent)
MAP_MESSAGE(120, SRoomSearch, roomSearchRequestedEvent)
MAP_MESSAGE(122, SUserPrivileges, userPrivilegesReceivedEvent) // Not used
MAP_MESSAGE(125, SAckNotifyPrivileges, ackNotifyPrivilegesReceivedEvent) // Not used
MAP_MESSAGE(133, SPrivRoomAlterableMembers, privRoomAlterableMembersReceivedEvent)
MAP_MESSAGE(134, SPrivRoomAddUser, privRoomAddedUserEvent)
MAP_MESSAGE(135, SPrivRoomRemoveUser, privRoomRemovedUserEvent)
MAP_MESSAGE(138, SPrivRoomUnknown138, privRoomUnknown138ReceivedEvent)
MAP_MESSAGE(139, SPrivRoomAdded, privRoomAddedReceivedEvent)
MAP_MESSAGE(140, SPrivRoomRemoved, privRoomRemovedReceivedEvent)
MAP_MESSAGE(141, SPrivRoomToggle, privRoomToggleReceivedEvent)
MAP_MESSAGE(142, SNewPassword, newPasswordReceivedEvent)
MAP_MESSAGE(143, SPrivRoomAddOperator, privRoomAddedOperatorEvent)
MAP_MESSAGE(144, SPrivRoomRemoveOperator, privRoomRemovedOperatorEvent)
MAP_MESSAGE(145, SPrivRoomOperatorAdded, privRoomOperatorAddedReceivedEvent)
MAP_MESSAGE(146, SPrivRoomOperatorRemoved, privRoomOperatorRemovedReceivedEvent)
MAP_MESSAGE(148, SPrivRoomAlterableOperators, privRoomAlterableOperatorsReceivedEvent)
MAP_MESSAGE(152, SPublicChat, publicChatReceivedEvent)
MAP_MESSAGE(1001, SCannotConnect, cannotConnectNotifyReceivedEvent)
