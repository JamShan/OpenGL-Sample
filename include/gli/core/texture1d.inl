///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Image (gli.g-truc.net)
///
/// Copyright (c) 2008 - 2013 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// @ref core
/// @file gli/core/texture1d.inl
/// @date 2013-01-12 / 2013-01-12
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include "levels.hpp"

namespace gli
{
	inline texture1D::texture1D() :
		BaseLayer(0),
		MaxLayer(0),
		BaseFace(0),
		MaxFace(0),
		BaseLevel(0),
		MaxLevel(0),
		Format(FORMAT_INVALID)
	{}

	inline texture1D::texture1D
	(
		size_type const & Levels,
		format_type const & Format,
		dim_type const & Dimensions
	) :
		Storage(
			1, 1, Levels,
			Format,
			storage::dim_type(Dimensions.x, 1, 1)),
		BaseLayer(0), MaxLayer(0),
		BaseFace(0), MaxFace(0),
		BaseLevel(0), MaxLevel(Levels - 1),
		Format(Format)
	{}

	inline texture1D::texture1D
	(
		format_type const & Format,
		dim_type const & Dimensions
	) :
		Storage(
			1, 1, gli::levels(Dimensions),
			Format,
			storage::dim_type(Dimensions.x, 1, 1)),
		BaseLayer(0), MaxLayer(0),
		BaseFace(0), MaxFace(0),
		BaseLevel(0), MaxLevel(Storage.levels() - 1),
		Format(Format)
	{}

	inline texture1D::texture1D
	(
		storage const & Storage
	) :
		Storage(Storage),
		BaseLayer(0), MaxLayer(0),
		BaseFace(0), MaxFace(0),
		BaseLevel(0), MaxLevel(Storage.levels() - 1),
		Format(Storage.format())
	{}

	inline texture1D::texture1D
	(
		storage const & Storage,
		format_type const & Format,
		size_type BaseLayer,
		size_type MaxLayer,
		size_type BaseFace,
		size_type MaxFace,
		size_type BaseLevel,
		size_type MaxLevel
	) :
		Storage(Storage),
		BaseLayer(BaseLayer), MaxLayer(MaxLayer),
		BaseFace(BaseFace), MaxFace(MaxFace),
		BaseLevel(BaseLevel), MaxLevel(MaxLevel),
		Format(Format)
	{}
 
	inline texture1D::texture1D
	(
		texture1D const & Texture,
		size_type const & BaseLevel,
		size_type const & MaxLevel
	) :
		Storage(Texture.Storage),
		BaseLayer(Texture.base_layer()),
		MaxLayer(Texture.max_layer()),
		BaseFace(Texture.base_face()),
		MaxFace(Texture.max_face()),
		BaseLevel(Texture.base_level() + BaseLevel),
		MaxLevel(Texture.base_level() + MaxLevel),
		Format(Texture.format())
	{}

	inline texture1D::texture1D
	(
		texture1DArray const & Texture,
		size_type const & BaseLayer,
		size_type const & BaseLevel,
		size_type const & MaxLevel
	) :
		Storage(Texture),
		BaseLayer(Texture.baseLayer() + BaseLayer),
		MaxLayer(Texture.baseLayer() + BaseLayer),
		BaseFace(Texture.baseFace()),
		MaxFace(Texture.maxFace()),
		BaseLevel(Texture.baseLevel() + BaseLevel),
		MaxLevel(Texture.baseLevel() + MaxLevel),
		Format(Texture.format())
	{}

	inline texture1D::operator storage() const
	{
		return this->Storage;
	}

	inline image texture1D::operator[](texture1D::size_type const & Level) const
	{
		assert(Level < this->levels());

		return image(
			this->Storage,
			this->base_layer(),
			this->max_layer(),
			this->base_face(),
			this->max_face(),
			this->base_level() + Level,
			this->base_level() + Level);
	}

	inline bool texture1D::empty() const
	{
		return this->Storage.empty();
	}

	inline texture1D::dim_type texture1D::dimensions() const
	{
		return texture1D::dim_type(this->Storage.dimensions(this->base_level()).x);
	}

	inline texture1D::format_type texture1D::format() const
	{
		return this->Format;
	}

	inline texture1D::size_type texture1D::layers() const
	{
		return 1;
	}

	inline texture1D::size_type texture1D::faces() const
	{
		return 1;
	}

	inline texture1D::size_type texture1D::levels() const
	{
		return this->max_level() - this->base_level() + 1;
	}

	inline texture1D::size_type texture1D::size() const
	{
		return this->Storage.face_size(this->base_level(), this->max_level());
	}

	inline void * texture1D::data()
	{
		assert(!this->empty());

		size_type const offset = detail::imageAddressing(
			this->Storage, this->base_layer(), this->base_face(), this->base_level());

		return this->Storage.data() + offset;
	}

	inline void const * texture1D::data() const
	{
		assert(!this->empty());
		
		size_type const offset = detail::imageAddressing(
			this->Storage, this->base_layer(), this->base_face(), this->base_level());

		return this->Storage.data() + offset;
	}

	template <typename genType>
	inline texture1D::size_type texture1D::size() const
	{
		assert(sizeof(genType) <= this->Storage.block_size());
		return this->size() / sizeof(genType);
	}

	template <typename genType>
	inline genType * texture1D::data()
	{
		assert(!this->empty());
		assert(this->Storage.block_size() >= sizeof(genType));

		return reinterpret_cast<genType *>(this->data());
	}

	template <typename genType>
	inline genType const * texture1D::data() const
	{
		assert(!this->empty());
		assert(this->Storage.block_size() >= sizeof(genType));

		return reinterpret_cast<genType const *>(this->data());
	}

	inline void texture1D::clear()
	{
		memset(this->data<glm::byte>(), 0, this->size<glm::byte>());
	}

	template <typename genType>
	inline void texture1D::clear(genType const & Texel)
	{
		assert(this->Storage.block_size() == sizeof(genType));

		for(size_type TexelIndex = 0; TexelIndex < this->size<genType>(); ++TexelIndex)
			*(this->data<genType>() + TexelIndex) = Texel;
	}

	inline texture1D::size_type texture1D::base_layer() const
	{
		return this->BaseLayer;
	}

	inline texture1D::size_type texture1D::max_layer() const
	{
		return this->MaxLayer;
	}

	inline texture1D::size_type texture1D::base_face() const
	{
		return this->BaseFace;
	}

	inline texture1D::size_type texture1D::max_face() const
	{
		return this->MaxFace;
	}

	inline texture1D::size_type texture1D::base_level() const
	{
		return this->BaseLevel;
	}

	inline texture1D::size_type texture1D::max_level() const
	{
		return this->MaxLevel;
	}
}//namespace gli
