#include <stdinc.hpp>
#include <utils/static_initializer.hpp>

namespace onigiri::utils
{
	void static_initializer::init()
	{
		for (auto initializer = static_initializer::initializers_; initializer; initializer = initializer->next_)
		{
			initializer->run();
		}
	}

	static_initializer::static_initializer(static_initializer::function_type function, std::uint32_t priority)
		: function_(function), priority_(priority)
	{
		if (static_initializer::initializers_)
		{
			auto current = static_initializer::initializers_;
			auto last = static_cast<utils::static_initializer*>(nullptr);

			while (current && this->priority_ >= current->priority_)
			{
				last = current;
				current = current->next_;
			}

			this->next_ = current;

			((!last) ? static_initializer::initializers_ : last->next_) = this;
		}

		else
		{
			this->next_ = nullptr;
			static_initializer::initializers_ = this;
		}
	}
}
