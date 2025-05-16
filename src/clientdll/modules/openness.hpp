#pragma once

namespace onigiri
{
	class openness final
	{
	private:
		STATICALLY_INITIALIZED(openness);

		static void AES__TransformITDecryptHook(uint32_t selector, void* data, uint32_t size);
		static inline utils::function<decltype(AES__TransformITDecryptHook)> AES__TransformITDecrypt;

		static void AES__DecryptHook(uint32_t encryption, uint32_t salt, char* header, int nameTableLen);
		static inline utils::function<decltype(AES__DecryptHook)> AES__Decrypt;
	};
}
