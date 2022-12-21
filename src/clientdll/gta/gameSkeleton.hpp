#pragma once

#include <gta/atArray.hpp>

namespace rage
{
	enum InitFunctionType : int
	{
		INIT_UNKNOWN = 0,
		INIT_CORE = 1,
		INIT_BEFORE_MAP_LOADED = 2,
		INIT_AFTER_MAP_LOADED = 4,
		INIT_SESSION = 8
	};

	const char* InitFunctionTypeToString(InitFunctionType type);

	struct InitFunctionData
	{
		void(*initFunction)(int);
		void(*shutdownFunction)(int);

		int initOrder;
		int shutdownOrder;
		InitFunctionType initMask;
		InitFunctionType shutdownMask;

		uint32_t funcHash;

		const char* GetName() const;

		bool TryInvoke(InitFunctionType type);
	};

	struct InitFunctionEntry
	{
		int order;
		atArray<int> functions;
		InitFunctionEntry* next;
	};

	struct InitFunctionList
	{
		InitFunctionType type;
		InitFunctionEntry* entries;
		InitFunctionList* next;
	};

	struct gameSkeleton_updateBase
	{
		virtual ~gameSkeleton_updateBase() = 0;

		virtual void Run() = 0;

		void RunGroup();

		bool m_flag;

		float m_unkFloat;

		uint32_t m_hash;

		// might be different for updateElement
		gameSkeleton_updateBase* m_nextPtr;

		gameSkeleton_updateBase* m_childPtr;
	};

	struct UpdateFunctionList
	{
		int type;
		gameSkeleton_updateBase* entry;
		UpdateFunctionList* next;
	};

	class gameSkeleton
	{
	public:
		void RunInitFunctions(InitFunctionType type);

		void RunUpdate(int type);

	protected:
		int m_functionOrder; // 8
		InitFunctionType m_functionType; // 12

		int32_t pad; // 16
		int m_updateType; // 20

		atArray<InitFunctionData> m_initFunctions; // 24
		void* pad2; // 40

		char pad3[256]; // 48

		InitFunctionList* m_initFunctionList; // 304

		void* pad4;

		UpdateFunctionList* m_updateFunctionList;

	private:
		virtual ~gameSkeleton() = 0;
	};
}
