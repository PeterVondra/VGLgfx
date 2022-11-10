#include <iostream>
#include <list>
#include "GUI/GUIWindow.h"

namespace vgl
{
	enum console_item_type_t
	{
		CTYPE_UCHAR,        // variable: unsigned char
		CTYPE_CHAR,         // variable: char
		CTYPE_UINT,         // variable: unsigned char
		CTYPE_INT,          // variable: int
		CTYPE_FLOAT,        // variable: float
		CTYPE_VEC3F,        // variable: float
		CTYPE_STRING,       // variable: std::string
		CTYPE_FUNCTION      // function
	};

	typedef void (*console_function)(const std::vector<std::string>&);

	struct console_item_t
	{
		std::string name;

		console_item_type_t ctype;

		union
		{
			void* var_ptr;
			console_function function;
		};
	};

	class Console
	{
		public:
			Console();
			virtual ~Console();

			void addItem(std::string p_cName, void* p_ItemPtr, console_item_type_t p_CType);
			void removeItem(std::string p_cName);
			void setDefaultCommand(console_function p_DefaultCmd);

			virtual void print(std::string p_Text);

		protected:
			bool parseCommandLine();

			void passKey(char c);
			void passBackspace();
			void passIntro();

			console_function m_DefaultCommand;

			std::vector<std::string> m_CommandBuffer;
			std::list<console_item_t> m_Items;

			bool command_echo_enabled = true;
			const uint32_t max_Commands = 100;

		protected:
			std::list<std::string> m_TextBuffer;
			std::string m_CommandLine;

	};
	class ConsoleWindow : public Console, public GUIWindow
	{
		public:
			ConsoleWindow(Window& p_CurrentWindow, const Vector2f p_Size, const Vector2f p_Position, Text::Font& p_Font, const std::string p_Title);
			~ConsoleWindow();

			void update() override;
			void updateText();

			void dockToMainWindow(DockConf p_Conf, Vector2f p_Block) override
			{
				if (p_Conf == DockConf::Top) {
					static float prevXS = m_TitleShape.getSize().x / 2;
					setSize({ (float)m_CurrentWindow->getWindowSize().x + p_Block.x, m_Size.y });
					setPosition({ (float)m_CurrentWindow->getWindowSize().x / 2 + p_Block.x / 2, (float)m_CurrentWindow->getWindowSize().y - m_Size.y / 2 + p_Block.y });
					text.setPosition({ m_TitleShape.getPosition().x - m_TitleShape.getSize().x / 2 - prevXS + 5, m_TitleShape.getPosition().y - 5 });
				}
				else if (p_Conf == DockConf::Bottom) {
					static float prevXS = m_TitleShape.getSize().x / 2;
					setSize({ (float)m_CurrentWindow->getWindowSize().x + p_Block.x, m_Size.y });
					setPosition({ (float)m_CurrentWindow->getWindowSize().x / 2 + p_Block.x / 2, m_Size.y / 2 + p_Block.y });
					text.setPosition({ m_TitleShape.getPosition().x - m_TitleShape.getSize().x / 2 - prevXS + 5, m_TitleShape.getPosition().y - 5 });
				}
				else if (p_Conf == DockConf::Left) {
					setSize({ m_Size.x + p_Block.x, (float)m_CurrentWindow->getWindowSize().y + p_Block.y });
					setPosition({ m_Size.x / 2 + p_Block.x / 2, (float)m_CurrentWindow->getWindowSize().y / 2 + p_Block.y / 2 });
					text.setPosition({ m_TitleShape.getPosition().x - m_TitleShape.getSize().x + 5, m_TitleShape.getPosition().y - 5 });
				}
				else if (p_Conf == DockConf::Right) {
					setSize({ m_Size.x + p_Block.x, (float)m_CurrentWindow->getWindowSize().y + p_Block.y });
					setPosition({ (float)m_CurrentWindow->getWindowSize().x - m_Size.x / 2 + p_Block.x / 2, (float)m_CurrentWindow->getWindowSize().y / 2 + p_Block.y / 2 });
					text.setPosition({ m_TitleShape.getPosition().x - m_TitleShape.getSize().x + 5, m_TitleShape.getPosition().y - 5 });
				}
			}

			bool inFocus() { return inputFocus; };
			void print(std::string p_Output) override;

			bool inputFocus = false;
		private:
			friend class vk::Renderer;

			int m_CursorIndex;

			Rectangle m_gCommandLine; // GUI
			Rectangle m_gCursor; // GUI
			TextBox   m_gTextCommandLine; // GUI
			TextBox   m_gTextOutput; // GUI

			bool tickFlag = false;
			bool prevPressed = true;
	};
}
