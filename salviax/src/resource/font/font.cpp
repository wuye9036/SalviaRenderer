#include <salviax/include/resource/font/font.h>

#include <salviar/include/surface.h>
#include <eflib/include/string/string.h>
#include <eflib/include/math/collision_detection.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <eflib/include/platform/boost_begin.h>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <eflib/include/platform/boost_end.h>

#if defined(EFLIB_WINDOWS)
#	include <salviax/include/utility/inc_windows.h>
#endif

EFLIB_USING_SHARED_PTR(salviar, surface);

BEGIN_NS_SALVIAX_RESOURCE();

class font_library
{
public:
	static font_library& instance()
	{
		static font_library inst;
		FT_Init_FreeType(&inst.ftlib_);
		return inst;
	}

	FT_Library& freetype_library()
	{
		return ftlib_;
	}

private:
	font_library();
	FT_Library ftlib_;
};

template <typename T>
class optional_value
{
public:
	typedef optional_value<T> this_type;

	optional_value<T>(): value_(), enabled_(false)
	{
	}

	this_type& operator = (this_type const& v)
	{
		value_ = v.value_;
		enabled_ = v.enabled_;
		return *this;
	}

	this_type& operator = (T const& v)
	{
		value_ = v;
		enabled_ = true;
		return *this;
	}

	T& get()
	{
		return value_;
	}
	T const& get() const
	{
		return value_;
	}

	T* get_ptr()
	{
		return &value_;
	}

	T const* get_ptr() const
	{
		return &value_;
	}

	bool enabled()
	{
		return enabled_;
	}

	void make_enable()
	{
		enabled_ = true;
	}

	void make_disable()
	{
		enabled_ = false;
	}

private:
	T		value_;
	bool	enabled_;
};

class font_impl: public font
{
public:
	font_impl(std::string const& file_name, size_t face_index)
	{
		size_ = 0;
		unit_ = pixels;

		FT_Error err = 0;
		err = FT_Init_FreeType( library_.get_ptr() );
		if( err ) { return; }
		library_.make_enable();

		err = FT_New_Face( library_.get(), file_name.c_str(), face_index, face_.get_ptr() );
		if(!err) {
			face_.make_enable();
			return;
		}
		if( err == FT_Err_Cannot_Open_Resource )
		{
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
		else if( err == FT_Err_Unknown_File_Format )
		{
			EFLIB_ASSERT_UNIMPLEMENTED();
		}

		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	~font_impl()
	{
		if( face_.enabled() ){
			FT_Done_Face( face_.get() );
		}
		if( library_.enabled() ){
			FT_Done_FreeType( library_.get() );
		}
	}

	virtual size_t size() const
	{
		return size_;
	}

	virtual units unit() const
	{
		return unit_;
	}

	virtual void size_and_unit(size_t sz, units un)
	{
		size_ = sz;
		unit_ = un;

		static int const dpi = 96;
		
		switch(un)
		{
		case pixels:
			FT_Set_Pixel_Sizes(face_.get(), sz, sz);
			return;
		case points:
			FT_Set_Char_Size(face_.get(), sz << 6, sz << 6, dpi, dpi);
			break;
		default:
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}

	//virtual size_t kerning() const
	//{
	//}

	//virtual void kerning( size_t sz )
	//{
	//}

	bool update_glyph(wchar_t wch)
	{
		glyph_slot_.make_disable();
		FT_UInt glyph_index = FT_Get_Char_Index(face_.get(), wch);
		FT_Error err = FT_Load_Glyph(face_.get(), glyph_index, FT_LOAD_DEFAULT);
		if(err){ return false; }
		glyph_slot_ = face_.get()->glyph;
		return true;
	}

	void draw(
		std::string const& text,
		salviar::surface* target, eflib::rect<int32_t> const& rc,
		salviar::color_rgba32f const& foreground_color, salviar::color_rgba32f const& background_color,
		font::render_hints /*hint*/)
	{
		std::wstring wtext;
		::setlocale(LC_ALL, ".936");
		eflib::to_wide_string(wtext, text);
		if( wtext.empty() ) { return; }

		int32_t target_pen_x = rc.x;
		int32_t target_pen_y = rc.y+rc.h;

		for(size_t i_ch = 0; i_ch < wtext.length(); ++i_ch)
		{
			FT_Glyph ch_glyph;
			update_glyph(wtext[i_ch]);

			FT_Get_Glyph(glyph_slot_.get(), &ch_glyph);

			FT_Glyph_To_Bitmap(&ch_glyph, FT_RENDER_MODE_NORMAL, NULL, 1);
			FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)ch_glyph;
			FT_Bitmap& bitmap = bitmap_glyph->bitmap;

			int32_t target_region_left   = target_pen_x + bitmap_glyph->left;
			int32_t target_region_top    = target_pen_y - (face_.get()->size->metrics.y_ppem - bitmap_glyph->top);
			int32_t target_region_bottom = std::max(target_region_top-bitmap.rows, rc.y);
			int32_t target_region_right  = std::min(bitmap.width+target_region_left, rc.x+rc.w);

			int32_t target_region_width  = target_region_right - target_region_left;
			int32_t target_region_height = target_region_top - target_region_bottom;

			for (int i = 0; i < target_region_height; ++i)
			{
				for (int j = 0; j < target_region_width; ++j)
				{
					float gray = bitmap.buffer[bitmap.width*i+j] / 255.0f;
					target->set_texel(target_region_left+j, target_region_top-i-1, 0, lerp(background_color, foreground_color, gray) );
				}
			}

			target_pen_x += (glyph_slot_.get()->advance.x >> 6);

			FT_Done_Glyph(ch_glyph);
		}
	}

private:
	optional_value<FT_Library>		library_;
	optional_value<FT_Face>			face_;
	optional_value<FT_GlyphSlot>	glyph_slot_;

	size_t							size_;
	units							unit_;
};

font_ptr font::create( std::string const& font_file_path, size_t face_index, size_t size, font::units unit )
{
	boost::shared_ptr<font_impl> ret( new font_impl(font_file_path, face_index) );
	ret->size_and_unit(size, unit);
	return ret;
}

font_ptr font::create_in_system_path( std::string const& font_file_name, size_t face_index, size_t size, font::units unit )
{
	char system_directory[1024];
#if defined(EFLIB_WINDOWS)
	GetWindowsDirectoryA(system_directory, 1024);
#else
#	error "Unsupport platform."
#endif

	boost::filesystem::path font_path( system_directory );
	font_path /= "Fonts";
	font_path /= font_file_name;

	return create( font_path.string(), face_index, size, unit );
}

END_NS_SALVIAX_RESOURCE();