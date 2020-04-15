#pragma once
#ifndef mirai_cpp__mirai_hpp_H_
#define mirai_cpp__mirai_hpp_H_
#include <string>
#include <vector>
#include <exception>
#include <thread>
#include <sstream>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include "CURLWrapper.h"
#include "defs/defs.hpp"

using std::string;
using std::runtime_error;
using std::vector;
using std::stringstream;
using std::function;
using nlohmann::json;

#ifdef SendMessage
#undef SendMessage
#endif

namespace Cyan
{
	typedef std::function<void(FriendMessage)> FriendMessageProcesser;
	typedef std::function<void(GroupMessage)> GroupMessageProcesser;
	typedef std::function<void(TempMessage)> TempMessageProcesser;


	class MiraiBot
	{
	public:
		MiraiBot() :pool_(4), qq_(0) {}
		MiraiBot(const string& host, int port) : pool_(4), qq_(0)
		{
			stringstream ss;
			ss << "http://" << host << ":" << port;
			api_url_prefix_ = ss.str();
		}
		MiraiBot(const string& url_prefix) :api_url_prefix_(url_prefix), pool_(4), qq_(0) {}
		~MiraiBot()
		{
			Release();
		}
		bool Auth(const string& authKey, QQ_t qq)
		{
			static const string api_url = api_url_prefix_ + "/auth";

			json j;
			j["authKey"] = authKey;
			string pData = j.dump();
			HTTP http; http.SetContentType("application/json;charset=UTF-8");
			auto res = http.Post(api_url, pData);

			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				int code = reJson["code"].get<int>();
				if (code == 0)
				{
					this->sessionKey_ = reJson["session"].get<string>();
					this->qq_ = qq;
					// 启动消息循环
					//std::async(std::launch::async, [&]() { EventLoop(); });
					return SessionVerify();
				}
				else
					throw runtime_error("Auth Key 不正确");
			}
			else
				throw runtime_error(res.ErrorMsg);
		}
		MessageId SendMessage(QQ_t target, const MessageChain& messageChain)
		{
			static const string api_url = api_url_prefix_ + "/sendFriendMessage";

			json j;
			j["sessionKey"] = sessionKey_;
			j["target"] = int64_t(target);
			j["messageChain"] = messageChain.ToJson();

			string pData = j.dump();
			HTTP http; http.SetContentType("application/json;charset=UTF-8");
			auto res = http.Post(api_url, pData);
			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				int code = reJson["code"].get<int>();
				if (code == 0)
				{
					MessageId msgId = reJson["messageId"].get<int>();
					return msgId;
				}
				else
				{
					string msg = reJson["msg"].get<string>();
					throw runtime_error(msg);
				}
			}
			else
				throw runtime_error(res.ErrorMsg);
		}
		MessageId SendMessage(GID_t target, const MessageChain& messageChain)
		{
			static const string api_url = api_url_prefix_ + "/sendGroupMessage";
			json j;
			j["sessionKey"] = sessionKey_;
			j["target"] = int64_t(target);
			j["messageChain"] = messageChain.ToJson();

			string pData = j.dump();
			HTTP http; http.SetContentType("application/json;charset=UTF-8");
			auto res = http.Post(api_url, pData);
			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				int code = reJson["code"].get<int>();
				if (code == 0)
				{
					MessageId msgId = reJson["messageId"].get<int>();
					return msgId;
				}
				else
				{
					string msg = reJson["msg"].get<string>();
					throw runtime_error(msg);
				}
			}
			else
				throw runtime_error(res.ErrorMsg);
		}
		MessageId SendMessage(GID_t gid, QQ_t qq, const MessageChain& messageChain)
		{
			static const string api_url = api_url_prefix_ + "/sendTempMessage";
			json j;
			j["sessionKey"] = sessionKey_;
			j["qq"] = int64_t(qq);
			j["group"] = int64_t(gid);
			j["messageChain"] = messageChain.ToJson();

			string pData = j.dump();
			HTTP http; http.SetContentType("application/json;charset=UTF-8");
			auto res = http.Post(api_url, pData);
			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				int code = reJson["code"].get<int>();
				if (code == 0)
				{
					MessageId msgId = reJson["messageId"].get<int>();
					return msgId;
				}
				else
				{
					string msg = reJson["msg"].get<string>();
					throw runtime_error(msg);
				}
			}
			else
				throw runtime_error(res.ErrorMsg);
		}
		FriendImage UploadFriendImage(const string& fileName)
		{
			static const string api_url = api_url_prefix_ + "/uploadImage";

			HTTP http; http.SetContentType("multipart/form-data");
			http.AddPostData("sessionKey", sessionKey_);
			http.AddPostData("type", "friend");
			http.AddFile("img", fileName);
			auto res = http.Post(api_url);
			FriendImage fImg;

			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				if (!reJson.is_object()) throw runtime_error("解析返回 JSON 时出错");
				fImg.ID = reJson["imageId"].get<string>();
				fImg.Url = reJson["url"].get<string>();
				fImg.Path = reJson["path"].get<string>();
			}
			else
				throw runtime_error(res.ErrorMsg);
			return fImg;
		}
		GroupImage UploadGroupImage(const string& fileName)
		{
			static const string api_url = api_url_prefix_ + "/uploadImage";

			HTTP http; http.SetContentType("multipart/form-data");
			http.AddPostData("sessionKey", sessionKey_);
			http.AddPostData("type", "group");
			http.AddFile("img", fileName);
			auto res = http.Post(api_url);
			GroupImage gImg;

			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				if (!reJson.is_object()) throw runtime_error("解析返回 JSON 时出错");
				gImg.ID = reJson["imageId"].get<string>();
				gImg.Url = reJson["url"].get<string>();
				gImg.Path = reJson["path"].get<string>();
			}
			else
				throw runtime_error(res.ErrorMsg);
			return gImg;
		}
		vector<Friend_t> GetFriendList()
		{
			static const string api_url = api_url_prefix_ + "/friendList?sessionKey=" + sessionKey_;
			vector<Friend_t> result;

			HTTP http;
			auto res = http.Get(api_url);
			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				if (!reJson.is_array()) throw runtime_error("解析返回 JSON 时出错");
				for (const auto& ele : reJson)
				{
					Friend_t f;
					f.Set(ele);
					result.push_back(f);
				}
			}
			else
				throw runtime_error(res.ErrorMsg);
			return result;

		}
		vector<Group_t> GetGroupList()
		{
			static const string api_url = api_url_prefix_ + "/groupList?sessionKey=" + sessionKey_;
			vector<Group_t> result;

			HTTP http;
			auto res = http.Get(api_url);
			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				if (!reJson.is_array()) throw runtime_error("解析返回 JSON 时出错");
				for (const auto& ele : reJson)
				{
					Group_t f;
					f.Set(ele);
					result.push_back(f);
				}
			}
			else
				throw runtime_error(res.ErrorMsg);
			return result;
		}
		vector<GroupMember_t> GetGroupMembers(GID_t target)
		{
			stringstream api_url;
			api_url
				<< api_url_prefix_
				<< "/memberList?sessionKey="
				<< sessionKey_
				<< "&target="
				<< target;

			vector<GroupMember_t> result;

			HTTP http;
			auto res = http.Get(api_url.str());
			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				if (!reJson.is_array()) throw runtime_error("解析返回 JSON 时出错");
				for (const auto& ele : reJson)
				{
					GroupMember_t f;
					f.Set(ele);
					result.push_back(f);
				}
			}
			else
				throw runtime_error(res.ErrorMsg);
			return result;

		}
		bool MuteAll(GID_t target)
		{
			static const string api_url = api_url_prefix_ + "/muteAll";
			json j;
			j["sessionKey"] = sessionKey_;
			j["target"] = int64_t(target);

			string pData = j.dump();
			HTTP http; http.SetContentType("application/json;charset=UTF-8");
			auto res = http.Post(api_url, pData);
			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				int code = reJson["code"].get<int>();
				if (code == 0)
					return true;
				else
				{
					string msg = reJson["msg"].get<string>();
					throw runtime_error(msg);
				}
			}
			else
				throw runtime_error(res.ErrorMsg);
			return false;
		}
		bool UnMuteAll(GID_t target)
		{
			static const string api_url = api_url_prefix_ + "/unmuteAll";
			json j;
			j["sessionKey"] = sessionKey_;
			j["target"] = int64_t(target);

			string pData = j.dump();
			HTTP http; http.SetContentType("application/json;charset=UTF-8");
			auto res = http.Post(api_url, pData);
			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				int code = reJson["code"].get<int>();
				if (code == 0)
					return true;
				else
				{
					string msg = reJson["msg"].get<string>();
					throw runtime_error(msg);
				}
			}
			else
				throw runtime_error(res.ErrorMsg);
			return false;

		}
		bool Mute(GID_t GID, QQ_t memberID, unsigned int time_seconds)
		{
			static const string api_url = api_url_prefix_ + "/mute";
			json j;
			j["sessionKey"] = sessionKey_;
			j["target"] = int64_t(GID);
			j["memberId"] = int64_t(memberID);
			j["time"] = time_seconds;

			string pData = j.dump();
			HTTP http; http.SetContentType("application/json;charset=UTF-8");
			auto res = http.Post(api_url, pData);
			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				int code = reJson["code"].get<int>();
				if (code == 0)
					return true;
				else
				{
					string msg = reJson["msg"].get<string>();
					throw runtime_error(msg);
				}
			}
			else
				throw runtime_error(res.ErrorMsg);
			return false;

		}
		bool UnMute(GID_t GID, QQ_t memberID)
		{
			static const string api_url = api_url_prefix_ + "/unmute";
			json j;
			j["sessionKey"] = sessionKey_;
			j["target"] = int64_t(GID);
			j["memberId"] = int64_t(memberID);

			string pData = j.dump();
			HTTP http; http.SetContentType("application/json;charset=UTF-8");
			auto res = http.Post(api_url, pData);
			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				int code = reJson["code"].get<int>();
				if (code == 0)
					return true;
				else
				{
					string msg = reJson["msg"].get<string>();
					throw runtime_error(msg);
				}
			}
			else
				throw runtime_error(res.ErrorMsg);
			return false;

		}
		bool Kick(GID_t GID, QQ_t memberID, const string& msg = "")
		{
			static const string api_url = api_url_prefix_ + "/kick";
			json j;
			j["sessionKey"] = sessionKey_;
			j["target"] = int64_t(GID);
			j["memberId"] = int64_t(memberID);
			j["msg"] = msg;

			string pData = j.dump();
			HTTP http; http.SetContentType("application/json;charset=UTF-8");
			auto res = http.Post(api_url, pData);
			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				int code = reJson["code"].get<int>();
				if (code == 0)
					return true;
				else
				{
					string msg = reJson["msg"].get<string>();
					throw runtime_error(msg);
				}
			}
			else
				throw runtime_error(res.ErrorMsg);
			return false;
		}

		void OnFriendMessageReceived(FriendMessageProcesser friendMessageProcesser)
		{
			friendMessageProcesser_ = friendMessageProcesser;
		}
		void OnGroupMessageReceived(GroupMessageProcesser groupMessageProcesser)
		{
			groupMessageProcesser_ = groupMessageProcesser;
		}
		void OnTempMessageReceived(TempMessageProcesser tempMessageProcesser)
		{
			tempMessageProcesser_ = tempMessageProcesser;
		}


		void inline static SleepSeconds(int sec)
		{
			std::this_thread::sleep_for(std::chrono::seconds(sec));
		}

		void inline static SleepMilliseconds(int ms)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(ms));
		}

		void EventLoop()
		{
			const unsigned count_per_loop = 20;
			const unsigned time_interval = 100;
			while (true)
			{
				unsigned count = FetchMessagesAndEvents(count_per_loop);
				if (count < count_per_loop)
				{
					SleepMilliseconds(time_interval);
				}
			}
		}

	private:
		bool SessionVerify() const
		{
			static const string api_url = api_url_prefix_ + "/verify";
			json j;
			j["sessionKey"] = sessionKey_;
			j["qq"] = int64_t(qq_);

			string pData = j.dump();
			HTTP http; http.SetContentType("application/json;charset=UTF-8");
			auto res = http.Post(api_url, pData);
			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				int code = reJson["code"].get<int>();
				if (code == 0)
					return true;
				else
				{
					string msg = reJson["msg"].get<string>();
					throw runtime_error(msg);
				}
			}
			else
				throw runtime_error(res.ErrorMsg);
			return false;
		}
		bool SessionRelease() const
		{
			static const string api_url = api_url_prefix_ + "/release";
			json j;
			j["sessionKey"] = sessionKey_;
			j["qq"] = int64_t(qq_);

			string pData = j.dump();
			HTTP http; http.SetContentType("application/json;charset=UTF-8");
			auto res = http.Post(api_url, pData);
			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				int code = reJson["code"].get<int>();
				if (code == 0)
					return true;
				else
				{
					string msg = reJson["msg"].get<string>();
					throw runtime_error(msg);
				}
			}
			else
				throw runtime_error(res.ErrorMsg);
			return false;
		}

		unsigned int FetchMessagesAndEvents(unsigned int count = 10)
		{
			stringstream api_url;
			api_url
				<< api_url_prefix_
				<< "/fetchMessage?sessionKey="
				<< sessionKey_
				<< "&count="
				<< count;

			int received_count = 0;

			HTTP http;
			auto res = http.Get(api_url.str());
			if (res.Ready)
			{
				json reJson;
				reJson = reJson.parse(res.Content);
				if (!reJson.is_object()) throw runtime_error("解析返回 JSON 时出错");
				for (const auto& ele : reJson["data"])
				{
					MiraiEvent type = MiraiEventStr(ele["type"].get<string>());
					if (groupMessageProcesser_ && type == MiraiEvent::GroupMessage)
					{
						GroupMessage gm;
						gm.Set(ele);
						boost::asio::post(pool_, [=]() { groupMessageProcesser_(gm); });
						continue;
					}
					if (friendMessageProcesser_ && type == MiraiEvent::FriendMessage)
					{
						FriendMessage fm;
						fm.Set(ele);
						boost::asio::post(pool_, [=]() { friendMessageProcesser_(fm); });
						continue;
					}
					if (tempMessageProcesser_ && type == MiraiEvent::TempMessage)
					{
						TempMessage gm;
						gm.Set(ele);
						boost::asio::post(pool_, [=]() { tempMessageProcesser_(gm); });
						continue;
					}
					received_count++;
				}
			}
			else
				throw runtime_error(res.ErrorMsg);
			return received_count;
		}

		bool Release()
		{
			try
			{
				return SessionRelease();
			}
			catch (const std::exception&)
			{
				return false;
			}

		}

		QQ_t qq_;
		string sessionKey_;
		string api_url_prefix_ = "http://127.0.0.1:8080";
		GroupMessageProcesser groupMessageProcesser_;
		TempMessageProcesser tempMessageProcesser_;
		FriendMessageProcesser friendMessageProcesser_;
		boost::asio::thread_pool pool_;
	};
} // namespace Cyan



#endif // !_mirai_cpp_mirai_hpp__
