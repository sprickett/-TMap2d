/*
	Copyright (c) 2018, Shaun Prickett
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this
	  list of conditions and the following disclaimer.

	* Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	* Neither the name of the copyright holder nor the names of its
	  contributors may be used to endorse or promote products derived from
	  this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once
#include <memory>

template<typename T>
class TMap
{
public:
	TMap<T>(void)
		:cols_(0)
		,rows_(0)
		,stride_(0)
		,capacity_(0)
		,buffer_(nullptr)
		,row0_(0)
	{}

	TMap<T>(int width, int height, int stride = 0)
		:cols_(width < 0 ? 0 : width)
		,rows_(height < 0 ? 0 : height)
		,stride_(stride? stride: cols_)
		,capacity_(stride_*rows_)
		,buffer_(new T[capacity_])
		,row0_(buffer_.get())
	{}

	TMap<T>(int width, int height, T* data, int stride = 0, bool copy = false)
		:cols_(width)
		,rows_(height)
		,stride_(stride ? stride : width)
		,capacity_(0)
		,buffer_(nullptr)
		,row0_(data)
	{
		if (copy)
			*this = clone();
	}

	int cols(void)const { return cols_; }
	int rows(void)const { return rows_; }
	int stride(void)const { return stride_; }
	
	bool const isContinuous(void) const { return stride_ == cols_; }

	TMap<T> operator()(int x, int y, int width, int height) const
	{
		TMap<T> b(*this);
		b.cols_ = width;
		b.rows_ = height;
		b.row0_ = b.ptr(y, x);
		return b;
	}

	void copyTo(TMap<T>& other) const
	{
		other.create(cols_, rows_);

		int h = rows_;
		int w = cols_;
		if (w == stride_ && w == other.stride_)
		{
			w *= h;
			h = 1;
		}

		for (int y = 0; y < h; ++y)
		{
			const T* p = ptr(y);
			std::copy(p, p + w, other.ptr(y));
		}
	}

	void setTo(const T& value)
	{
		int h = rows_;
		int w = cols_;
		if (w == stride_)
		{
			w *= h;
			h = 1;
		}

		for (int y = 0; y < h; ++y)
		{
			T* p = ptr(y);
			for (int x = 0; x < w; ++x)
				p[x] = value;
		}
	}

	void create(int width, int height)
	{
		if (cols_ == width && rows_ == height)
			return;

		if (buffer_.use_count() == 1 && capacity_ >= width * height)
		{
			cols_ = width;
			rows_ = height;
			stride_ = width;
			row0_ = buffer_.get();
			return;
		}

		*this = TMap<T>(width, height);
	}

	TMap<T> clone(void) const
	{
		TMap<T> b;
		copyTo(b);
		return b;
	}
	
	T* ptr(int y = 0, int x = 0)
	{
		return row0_ + (stride_ * y + x);
	}

	const T* ptr(int y = 0, int x = 0) const
	{
		return row0_ + (stride_ * y + x);
	}

	bool isOverlapping(const TMap<T>& other) const
	{
		if (row0_ < other.row0_)
			return isOverlapping(*this, other);
		else if (row0_ > other.row0_)
			return isOverlapping(other, *this);
		else
			return row0_ != nullptr;
	}
private:
	static bool isOverlapping(const TMap<T>& lo, const TMap<T>& hi)
	{
		if (hi.row0_ < lo.row0_)
			return isOverlapping(hi, lo);
		if (lo.ptr(lo.rows_ - 1, lo.cols_ - 1) < hi.row0_)
			return false;
		int x = (hi.row0_ - lo.row0_) % lo.stride_;
		return (x < lo.cols_ || x + hi.cols_ > lo.stride_);
	}
	
	int cols_;
	int rows_;
	int stride_;
	size_t capacity_;
	std::shared_ptr<T[]> buffer_;
	T* row0_;
};