#pragma once
#ifndef mirai_cpp_events_events_name_hpp_H_
#define mirai_cpp_events_events_name_hpp_H_

#include <functional>
#include "events/friend_message.hpp"
#include "events/group_message.hpp"
#include "events/temp_message.hpp"

namespace Cyan
{

	// Mirai �¼�����
	enum class MiraiEvent
	{
		Default,				// δ֪
		FriendMessage,			// ������Ϣ
		GroupMessage,			// Ⱥ����Ϣ
		TempMessage,			// ��ʱ��Ϣ
		GroupRecallEvent,		// Ⱥ��Ϣ����
		FriendRecallEvent,		// ������Ϣ����
		BotMuteEvent,			// Bot������
		BotUnmuteEvent,			// Bot��ȡ������
		BotJoinGroupEvent,		// Bot������һ����Ⱥ
		GroupNameChangeEvent,	// ĳ��Ⱥ���Ƹı�
		GroupMuteAllEvent,		// ȺȫԱ����
		MemberJoinEvent,		// ������Ⱥ�¼�
		MemberLeaveEventKick,	// ��Ա���߳�Ⱥ(�ó�Ա����Bot)
		MemberLeaveEventQuit,	// ��Ա�����뿪Ⱥ��
		MemberMuteEvent,		// Ⱥ��Ա������(�ó�Ա����Bot)
		MemberUnmuteEvent,		// Ⱥ��Ա��ȡ������(�ó�Ա����Bot)
		NewFriendRequestEvent,	// ��Ӻ�������
		MemberJoinRequestEvent	// �û���Ⱥ����
	};

	inline MiraiEvent MiraiEventStr(const string& miraiEvent)
	{
		if (miraiEvent == "FriendMessage") return MiraiEvent::FriendMessage;
		if (miraiEvent == "GroupMessage") return MiraiEvent::GroupMessage;
		if (miraiEvent == "TempMessage") return MiraiEvent::TempMessage;
		return MiraiEvent::Default;
	}

	inline string MiraiEventStr(MiraiEvent miraiEvent)
	{
		string result;
		switch (miraiEvent)
		{
		case Cyan::MiraiEvent::FriendMessage:
			result = "FriendMessage";
			break;
		case Cyan::MiraiEvent::GroupMessage:
			result = "GroupMessage";
			break;
		case Cyan::MiraiEvent::TempMessage:
			result = "TempMessage";
			break;
		default:
			result = "Default";
			break;
		}
		return result;
	}

	template<typename T>
	MiraiEvent GetEventName()
	{
		return "Default";
	}

	template<>
	MiraiEvent GetEventName<FriendMessage>()
	{
		return MiraiEvent::FriendMessage;
	}

	template<>
	MiraiEvent GetEventName<GroupMessage>()
	{
		return MiraiEvent::GroupMessage;
	}

	template<>
	MiraiEvent GetEventName<TempMessage>()
	{
		return MiraiEvent::TempMessage;
	}

}


#endif // !mirai_cpp_events_events_name_hpp_H_
