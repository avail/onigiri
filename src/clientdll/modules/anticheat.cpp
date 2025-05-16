#include <stdinc.hpp>

#include <utils/static_initializer.hpp>

#include <gta/gameSkeleton.hpp>

static std::uint32_t sus_2802_hash = 0xA0F39FB6;

namespace rage
{
	const char* InitFunctionData::GetName() const
	{
		return &std::format("0x{:08x}", this->funcHash)[0];
	}

	bool InitFunctionData::TryInvoke(InitFunctionType type)
	{
		static std::map<uint32_t, const char*> hashmap =
		{
			{ hash_string("TamperActions"), "TamperActions" },
			{ sus_2802_hash, "sus_2802_hash" }
		};

		if (funcHash == hash_string("TamperActions")
			|| funcHash == sus_2802_hash)
		{
			onigiri::services::logger::info("not invoking ac init func: {}", hashmap[funcHash]);
			return false;
		}

		initFunction(type);
		return true;
	}

	void gameSkeleton::RunInitFunctions(InitFunctionType type)
	{
		for (auto list = m_initFunctionList; list; list = list->next)
		{
			if (list->type == type)
			{
				for (auto entry = list->entries; entry; entry = entry->next)
				{
					int i = 0;

					for (int index : entry->functions)
					{
						auto func = m_initFunctions[index];

						func.TryInvoke(type);

						++i;
					}
				}
			}
		}
	}

	static void run_entries(gameSkeleton_updateBase* update)
	{
		for (auto entry = update; entry; entry = entry->m_nextPtr)
		{
			if (entry->m_hash == hash_string("TamperActions")
				|| entry->m_hash == sus_2802_hash)
			{
				return;
			}

			entry->Run();
		}
	}

	void gameSkeleton::RunUpdate(int type)
	{
		for (auto list = m_updateFunctionList; list; list = list->next)
		{
			if (list->type == type)
			{
				run_entries(list->entry);
			}
		}
	}

	void gameSkeleton_updateBase::RunGroup()
	{
		run_entries(this->m_childPtr);
	}
}

static onigiri::utils::static_initializer _([]()
{
	// only disable the anticheat in DEBUG, RunInitFunctions has a pso init section in it now which breaks.. something
	// we disable the anticheat to be able to debug GTA.
#if !DEBUG
	return;
#endif

	onigiri::services::logger::info("disabling rage::RageSecurity~");

	{
		void* location = hook::pattern("BA ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? 48 83 C4").count(1).get(0).get<void>(5);
		hook::jump(hook::get_call(location), hook::get_member(&rage::gameSkeleton::RunInitFunctions));
	}

	hook::jump(hook::get_call(hook::get_pattern("83 C2 ? 48 8D 0D ? ? ? ? E8 ? ? ? ? C6 05", 10)), hook::get_member(&rage::gameSkeleton::RunUpdate));
	hook::jump(hook::get_pattern("56 48 83 EC ? 48 8B 71 ? 48 85 F6 74 ? 66 90 48 8B 06 48 89 F1"), hook::get_member(&rage::gameSkeleton_updateBase::RunGroup));
}, INT32_MIN);

