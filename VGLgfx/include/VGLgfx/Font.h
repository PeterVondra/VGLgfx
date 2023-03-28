#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H  

#include "VGL-2D/Rectangle.h"

namespace vgl
{
	namespace Text
	{
		struct Character
		{
			Character() : advance(0)
			{

			}
			Character(const Character& p_Character)
			{
				*this = p_Character; 
			}
			
			Vector2i size;
			Vector2i bearing;
			int advance;
		};
		
		class Font
		{
			public:
				Font();
				~Font();

				void setFontSize(const int p_FontSize);
				std::vector<std::pair<wchar_t, Character>> characters;
				std::vector<Image> glyphs;
			protected:
			private:
				friend class FontLoader;
				friend class Text;

				FT_Face face;
				

				int m_FontSize = 48;
		};
		class FontLoader
		{
			public:

				static bool getFontFromPath(const char* p_Path, Font& p_Font);

			protected:
			private:

		};
		class Text
		{
			public:
				Text();
				Text(Font& p_Font);
				~Text();

				void setFont(Font& p_Font);
				Font& getFont() { return *m_Font; };

				int getFontSize() { return m_Font->m_FontSize; }

				std::string& getString();
			private:
				friend Font;

				Font* m_Font;
				std::string m_Text;
		};
	}
}