#include <stdinc.hpp>

// don't need tls.. i think
#if 0

#include <gta/sysAllocator.hpp>

static rage::sysMemAllocator* g_gtaTlsEntry;
static uint32_t g_tempAllocatorTlsOffset;

namespace rage
{
	void* sysUseAllocator::operator new(size_t size)
	{
		return GetAllocator()->allocate(size, 16, 0);
	}

	void sysUseAllocator::operator delete(void* memory)
	{
		GetAllocator()->free(memory);
	}

	static uint32_t g_tlsOffset;

	uint32_t sysMemAllocator::GetAllocatorTlsOffset()
	{
		return g_tlsOffset;
	}

	sysMemAllocator* GetAllocator()
	{
		sysMemAllocator* allocator = *(sysMemAllocator**)(hook::get_tls() + sysMemAllocator::GetAllocatorTlsOffset());

		if (!allocator)
		{
			return sysMemAllocator::UpdateAllocatorValue();
		}

		return allocator;
	}

	sysMemAllocator* sysMemAllocator::UpdateAllocatorValue()
	{
		assert(g_gtaTlsEntry);

		*(sysMemAllocator**)(hook::get_tls() + sysMemAllocator::GetAllocatorTlsOffset()) = g_gtaTlsEntry;
		*(sysMemAllocator**)(hook::get_tls() + g_tempAllocatorTlsOffset) = g_gtaTlsEntry;

		return g_gtaTlsEntry;
	}
}

static DWORD RageThreadHook(HANDLE hThread)
{
	// store the allocator
	g_gtaTlsEntry = *(rage::sysMemAllocator**)(hook::get_tls() + rage::sysMemAllocator::GetAllocatorTlsOffset());

	return GetThreadId(hThread);
}

static onigiri::utils::static_initializer _([]()
{
	rage::g_tlsOffset = *hook::pattern("B9 ? ? ? ? 48 8B 0C 01 45 33 C9 49 8B D2 48").get(0).get<uint32_t>(1);

	void* location = hook::pattern("48 8B CF FF 15 ? ? ? ? 4C 8B 4D 50 4C 8B 45").count(1).get(0).get<void>(3);

	hook::nop(location, 6);
	hook::call(location, RageThreadHook);

	g_tempAllocatorTlsOffset = *hook::get_pattern<uint32_t>("4A 3B 1C 09 75 ? 49 8B 0C 08 48", -4);
});

#endif
