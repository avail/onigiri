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
		if (funcHash == hash_string("fwClothMeshing")
			|| funcHash == hash_string("rageSecEngine")
			|| funcHash == hash_string("CCreditsText")
			|| funcHash == hash_string("rageSecGamePluginManager")
			|| funcHash == hash_string("TamperActions")
			|| funcHash == sus_2802_hash)
		{
			onigiri::services::logger::info("not invoking ac init func");
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
			if (/*entry->m_hash == hash_string("fwClothMeshing")
				||*/ entry->m_hash == hash_string("rageSecEngine")
				//|| entry->m_hash == hash_string("CCreditsText")
				//|| entry->m_hash == hash_string("rageSecGamePluginManager")
				//|| entry->m_hash == hash_string("TamperActions")
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
	onigiri::services::logger::info("disabling rage::RageSecurity~");

	{
		void* location = hook::pattern("BA 04 00 00 00 E8 ? ? ? ? E8 ? ? ? ? E8").count(1).get(0).get<void>(5);
		hook::jump(hook::get_call(location), hook::get_member(&rage::gameSkeleton::RunInitFunctions));
	}

	hook::jump(hook::get_call(hook::get_pattern("48 8D 0D ? ? ? ? BA 02 00 00 00 84 DB 75 05", -17)), hook::get_member(&rage::gameSkeleton::RunUpdate));
	hook::jump(hook::get_pattern("40 53 48 83 EC 20 48 8B 59 20 EB 0D 48 8B 03 48"), hook::get_member(&rage::gameSkeleton_updateBase::RunGroup));
}, INT32_MIN);
