#include "Font.h"

namespace vgl
{
	namespace Text
	{
		Font::Font()
		{

		}
		Font::~Font()
		{

		}

		bool FontLoader::getFontFromPath(const char* p_Path, Font& p_Font)
		{
			FT_Library ftlib;

			if (FT_Init_FreeType(&ftlib))
			{
				VGL_LOG_MSG("Failed to initialize freetype\n", "Font", Utils::Severity::Error);
				return false;
			}

			if (FT_New_Face(ftlib, p_Path, 0, &p_Font.face))
			{
				VGL_LOG_MSG("Failed to laod font file " + std::string(p_Path) + "\n", "Font", Utils::Severity::Error);
				return false;
			}

			FT_Set_Pixel_Sizes(p_Font.face, 0, p_Font.m_FontSize);
			FT_Select_Charmap(p_Font.face, FT_ENCODING_UNICODE);

			for (wchar_t c = 0; c < 128; c++)
			{
				if (FT_Load_Char(p_Font.face, c, FT_LOAD_RENDER))
				{
					VGL_LOG_MSG("Failed to load character: " + (char)c, "Font", Utils::Severity::Error);
					continue;
				}
				
				Character character;
				character.size = Vector2i(p_Font.face->glyph->bitmap.width, p_Font.face->glyph->bitmap.rows);
				character.bearing = Vector2i(p_Font.face->glyph->bitmap_left, p_Font.face->glyph->bitmap_top);
				character.advance = p_Font.face->glyph->advance.x;

				if (character.size != Vector2i(0))
				{
					p_Font.glyphs.push_back(Image());
					p_Font.glyphs[p_Font.glyphs.size() - 1].create(character.size, p_Font.face->glyph->bitmap.buffer, Channels::R, SamplerMode::ClampToBorder);

					p_Font.characters.emplace_back((char)c, character);
				}
			}

			FT_Done_Face(p_Font.face);
			FT_Done_FreeType(ftlib);
		}

		Text::Text()
		{

		}
		Text::Text(Font& p_Font)
		{
			m_Font = &p_Font;
		}
		Text::~Text()
		{

		}

		void Font::setFontSize(const int p_FontSize)
		{
			m_FontSize = p_FontSize;
		}

		void Text::setFont(Font& p_Font)
		{
			m_Font = &p_Font;
		}

		std::string& Text::getString()
		{
			return m_Text;
		}
	}
}
