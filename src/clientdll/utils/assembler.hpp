#pragma once

#define ASMJIT_STATIC // ???
#include <asmjit/asmjit.h>

namespace onigiri::utils
{
	class assembler final
	{
	public:
		template <typename T> static inline auto memory(const T& value) -> asmjit::x86::Mem
		{
			return asmjit::x86::Mem(reinterpret_cast<std::size_t>(&value), sizeof(value));
		}

		template <typename F> inline assembler(void* address, F&& callback)
		{
			auto code = asmjit::CodeHolder();
			auto result = code.init(assembler::runtime_.environment());

			if (result != asmjit::ErrorCode::kErrorOk)
			{
				assert(false);
			}
			else
			{
				auto assembler = asmjit::x86::Assembler(&code);

				callback(assembler);

				auto function = reinterpret_cast<void(__cdecl*)()>(nullptr);

				result = assembler::runtime_.add(&function, &code);

				if (result != asmjit::ErrorCode::kErrorOk)
				{
					assert(false);
				}
				else
				{
					hook::jump(address, function);
				}
			}
		}

	private:
		static inline asmjit::JitRuntime runtime_;
	};
}

using namespace asmjit::x86;
