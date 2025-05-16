#include <stdinc.hpp>

#include <utils/assembler.hpp>
#include <modules/openness.hpp>

namespace onigiri
{
	enum class CurrentEncryption : uint32_t
	{
		NG = 0xF0EFFFFF,
		OPEN = 0x4E45504F,
	};

	CurrentEncryption currentEncryption;

	bool AES__isTransformITKeyId(uint32_t encryption)
	{
		currentEncryption = (CurrentEncryption)encryption;

		return (encryption & 0xFF00000) == 0xFE00000;
	}

	void openness::AES__TransformITDecryptHook(uint32_t selector, void* data, uint32_t size)
	{
		if (currentEncryption == CurrentEncryption::OPEN)
		{
			onigiri::services::logger::info("found NEPO packfile, not decrypting");
			return;
		}

		openness::AES__TransformITDecrypt(selector, data, size);
	}

	void openness::AES__DecryptHook(uint32_t encryption, uint32_t salt, char* header, int nameTableLen)
	{
		if (encryption == (uint32_t)CurrentEncryption::OPEN)
		{
			onigiri::services::logger::info("found NEPO packfile, not decrypting");
			return;
		}

		openness::AES__Decrypt(encryption, salt, header, nameTableLen);
	}

	STATICALLY_INITIALIZE(openness)([]()
	{
		onigiri::services::logger::info("adding openiv archive support :3");

		//hook::nopVP(hook::get_pattern("89 D8 25 ? ? ? ? 48 8D 0D"), 29);

		// AES::isTransformITKeyId
		hook::call(hook::get_pattern("89 8E BC 00 00 00 E8 ? ? ? ? 80 7C", 6), AES__isTransformITKeyId);

		// AES::TransformITDecrypt
		{
			auto location = hook::get_pattern("74 0F 48 8B 56 28 44 89 E1", 12);
			AES__TransformITDecrypt = onigiri::utils::call(location, AES__TransformITDecryptHook);
		}

		// rage::AES::Decrypt
		{
			auto location = hook::get_pattern("8B 8E BC 00 00 00 44 89 E2 4D 89 F8", 12);
			AES__Decrypt = onigiri::utils::call(location, AES__DecryptHook);
		}

		// rage::AES::Decrypt
		{
			auto location = hook::get_pattern("48 8B 46 ? 8B 4C 24 ? 48 8D 14 08", -5);
			onigiri::utils::call(location, AES__DecryptHook);
		}

		auto location = hook::get_pattern("8B 44 24 ? 89 04 0B");
		auto retLoc = hook::get_pattern("8B 46 ? EB ? 31 C0 48 83 7E");

		onigiri::utils::assembler(location, [retLoc](auto& a)
		{
			auto post = a.newLabel();
			auto isOpenIV = a.newLabel();

			a.mov(eax, dword_ptr(rsp, 0x3C)); // mov eax, [rsp+88h+a4+0Ch]

			a.cmp(eax, 0x4E45504F);
			a.jz(isOpenIV);

			a.mov(dword_ptr(rbx, rcx), eax);
			a.jmp(post);

			a.bind(isOpenIV);
			a.mov(dword_ptr(rbx, rcx), 0x0FEFFFFF);

			a.bind(post);
			a.jmp(retLoc);
		});
	});
}
