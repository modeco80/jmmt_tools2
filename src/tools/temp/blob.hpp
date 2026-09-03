#pragma once
#include <mco/base_types.hpp>

/// A simple container for a heap array and size,
/// which is uniquely owned. Contains helper utilities.
struct Blob {
	Unique<u8[]> pBlobData;
	usize blobSize;

	/// Typed array view.
	template <class T>
	struct TypedArrayView {
		T* pArray;
		usize arraySize;

		// TODO begin() end()

		const usize size() const {
			return arraySize;
		}

		const T& operator[](usize index) const {
			// TODO
			return pArray[index];
		}
	};

	template <class T>
	inline T* cast() {
		return reinterpret_cast<T*>(pBlobData.get());
	}

	template <class T>
	inline T* castAt(usize offset) {
		return reinterpret_cast<T*>(pBlobData.get() + offset);
	}

	/// Casts an array. Assumes the whole blob is the array.
	template <class T>
	inline TypedArrayView<T> castArray() {
		return TypedArrayView<T> {
			.pArray = cast<T>(),
			.arraySize = blobSize / sizeof(T)
		};
	}

	template <class T>
	inline TypedArrayView<T> castArrayAt(usize offset, usize length) {
		return TypedArrayView<T> {
			.pArray = castAt<T>(offset),
			.arraySize = length
		};
	}
};
