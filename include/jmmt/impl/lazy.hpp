#pragma once
#include <mco/base_types.hpp>
#include <optional>

namespace jmmt::impl {

	/// A lazy computation.
	template <class T>
	class Lazy {
		class ImplData {
		public:
			virtual ~ImplData() = default;
			virtual const T& get() = 0;
		};

		Unique<ImplData> data;
	   public:

		/// Sets this lazy instance to use a lambda function.
		template<class F>
		void setLambda(F&& f) {
			class Impl : public ImplData {
				F fun;
				std::optional<T> t;
			public:
				explicit Impl(F&& f)
					: fun(static_cast<F&&>(f)) {
				}

				virtual ~Impl() = default;

				const T& get() override {
					if(!t.has_value())
						t = fun();
					return *t;
				}
			};
			data = std::make_unique<Impl>(static_cast<F&&>(f));
		}

		const T& get() {
			return data->get();
		}
	};
} // namespace jmmt::impl
