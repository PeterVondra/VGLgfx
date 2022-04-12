#include "Console.h"
#include "Application/InputManager.h"

namespace vgl
{
	Console::Console()
	{
		//setDefaultCommand(default);
	}
	Console::~Console()
	{

	}

	void Console::addItem(std::string p_cName, void* p_ItemPtr, console_item_type_t p_cType)
	{
		console_item_t item;
		item.ctype = p_cType;
		item.name = p_cName;

		if (p_cType == CTYPE_FUNCTION)
			item.function = (console_function)p_ItemPtr;
		else
			item.var_ptr = p_ItemPtr;

		m_Items.push_back(item);
	}
	void Console::removeItem(std::string p_cName)
	{
		std::list<console_item_t>::const_iterator it;
		for (it = m_Items.begin(); it != m_Items.end(); ++it)
		{
			if (it->name == p_cName)
				m_Items.erase(it);
		}
	}
	void Console::setDefaultCommand(console_function p_DefaultCmd)
	{

	}

	void Console::print(std::string p_Text)
	{
		
	}

	bool Console::parseCommandLine()
	{
		std::ostringstream out;
		std::vector<std::string> args;
		std::list<console_item_t>::const_iterator it;

		if (command_echo_enabled)
			print(m_CommandLine);

		m_CommandBuffer.push_back(m_CommandLine);
		if (m_CommandBuffer.size() > max_Commands)
			m_CommandBuffer.erase(m_CommandBuffer.begin());

		int index = 0;
		for (int i = 0; i < m_CommandLine.length(); i++)
		{
			if (m_CommandLine[i] == ' ')
			{
				// i - 1 is index of character before space
				// i + 1 is index of character after space
				args.push_back(m_CommandLine.substr(index, std::abs(index - i)));
				index = i + 1;
			}
			else if (i + 1 == m_CommandLine.length())
				args.push_back(m_CommandLine.substr(index, std::abs(index - i - 1)));
		}

		for (it = m_Items.begin(); it != m_Items.end(); ++it)
		{
			if (it->name == args[0])
			{
				switch(it->ctype)
				{
					case CTYPE_UINT:
						if (args.size() > 2)
							return false;
						else if (args.size() == 1)
						{
							out.str(""); // clear string stream
							out << (*it).name << " = " << *((unsigned int*)(*it).var_ptr);

							print(out.str());
							return true;
						}
						else if (args.size() == 2)
						{
							*((unsigned int*)(*it).var_ptr) = (unsigned int)std::stoi(args[1]);
							return true;
						}
						break;
					case CTYPE_VEC3F:
						if (args.size() == 1)
						{
							out.str(""); // clear string stream
							out << (*it).name << " = " << *((Vector3f*)(*it).var_ptr);

							print(out.str());
							return true;
						}

					case CTYPE_FUNCTION:
						(*it).function(args);
						return true;
						break;
					default:
						print(args[0] + " is not a recognized command");
						break;
				}
			}
		}

		print(args[0] + " is not a recognized command");
	}

	void Console::passKey(char c)
	{
		if ((int)c == 1)
		{
			parseCommandLine();
			m_CommandLine = "";
			return;
		}
		m_CommandLine += c;
	}
	void Console::passBackspace()
	{
		
	}
	void Console::passIntro()
	{

	}

	ConsoleWindow::ConsoleWindow(Window& p_CurrentWindow, const Vector2f p_Size, const Vector2f p_Position, Text::Font& p_Font, const std::string p_Title)
		: GUIWindow::GUIWindow(p_CurrentWindow, p_Size, p_Position, p_Font, p_Title),
		m_gCommandLine
		(
			{ p_Size.x - 10, 20 },
			{ p_Position.x, p_Position.y - (p_Size.y / 2) + 15 },
			m_BaseShape.getColor()
		),
		m_gCursor
		(
			{ 2, 0 },
			{ p_Position.x, p_Position.y - (p_Size.y / 2) + 15 },
			{ 0.5, 0.5, 0.5}
		)
	{

		m_gCommandLine.setOutlineThickness(3);
		m_gCommandLine.setOutlineColor({ 0.2, 0.2, 0.2 });

		m_gCursor.setOpacity(0.6);

		m_gTextCommandLine.init
		(
			{ m_gCommandLine.getSize().x, 15 },
			{ m_gCommandLine.getPosition().x + 5, m_gCommandLine.getPosition().y - 5 },
			Vector3f(1.0f),
			font, "> "
		);
		m_gTextOutput.init
		(
			{ m_gCommandLine.getSize().x, 15 },
			{ m_gCommandLine.getPosition().x + 5, m_gCommandLine.getPosition().y + m_gCommandLine.getSize().y },
			Vector3f(1.0f),
			font, "Console: "
		);
		m_gTextOutput.setText("");

		command_echo_enabled = false;
		m_CursorIndex = -1;
	}
	ConsoleWindow::~ConsoleWindow()
	{

	}

	void ConsoleWindow::updateText()
	{
		static bool prevMousePressed = false;
		static std::string cmd = "> ";
		static bool inputIntersect;

		inputIntersect = Physics2D::AABBCollider::checkIntersection(Input::getPrevCursorPosition(), m_gCommandLine) ||
			Physics2D::AABBCollider::checkIntersection(Input::getPrevCursorPosition(), m_gCommandLine);

		if (!inputIntersect && Input::mouseButtonIsPressed(0))
			inputFocus = false;
		else if (Input::mouseButtonIsPressed(0))
			inputFocus = true;

		if (m_CurrentWindow->onTick5ms() && !prevPressed)
			tickFlag = !tickFlag;

		if ((Input::keyIsPressed(Key::Backspace) || Input::keyIsRepeated(Key::Backspace)) && inputFocus)
		{
			if (m_CommandLine.size() > 0)
			{
				if (m_CursorIndex >= 0)
				{
					m_CommandLine.erase(m_CommandLine.begin() + m_CursorIndex);

					m_CursorIndex--;
					m_gTextCommandLine.setText(cmd + m_CommandLine);
				}
			}

			tickFlag = false;
		}
		else if (Input::keyIsPressed(Key::Enter) && inputFocus && !m_CommandLine.empty())
		{
			parseCommandLine();
			m_CommandLine = "";
			cmd = "> ";
			m_CursorIndex = 0;
			m_gTextCommandLine.setText(cmd + m_CommandLine);
		}
		else if ((Input::keyIsDown(Key::LeftControl) || Input::keyIsDown(Key::RightControl)) && Input::keyIsPressed(Key::V) && inputFocus)
		{
			m_CommandLine += m_CurrentWindow->getClipboard();
			m_CursorIndex = m_CommandLine.size();
			m_gTextCommandLine.setText(cmd + m_CommandLine);
		}
		else if (Input::anyKeyIsPressed() && inputFocus)
		{
			if (Input::getCurrentKey() > 0 && Input::getCurrentKey() < 128)
			{
				if(m_CommandLine.size() == 0) {
					m_CommandLine += Input::getCurrentKey();
					m_CursorIndex += 1;
				}
				else if(m_CursorIndex == m_CommandLine.size()){
					m_CommandLine += Input::getCurrentKey();
					m_CursorIndex++;
				}
				else{
					m_CommandLine.insert(m_CommandLine.begin() + m_CursorIndex + 1, Input::getCurrentKey());
					m_CursorIndex++;
				}
				m_gTextCommandLine.setText(cmd + m_CommandLine);
				tickFlag = false;
			}
		}
		if (inputIntersect)
			m_CurrentWindow->setCursor(Cursor::TextInput);
		else if (!checkBorderIntersection(Input::getCursorPosition(), m_BaseShape) && !checkBorderIntersection(Input::getPrevCursorPosition(), m_BaseShape))
			m_CurrentWindow->setCursor(Cursor::Standard);

		m_Collided = Physics2D::AABBCollider::checkIntersection(Input::getPrevCursorPosition(), m_TitleShape);

		prevMousePressed = Input::mouseButtonIsPressed(0);

		if (Input::keyIsPressed(Key::Left) || Input::keyIsRepeated(Key::Left))
		{
			if (m_CursorIndex >= 0)
				m_CursorIndex--;

			tickFlag = false;
			prevPressed = true;
		}
		else if (Input::keyIsPressed(Key::Right) || Input::keyIsRepeated(Key::Right))
		{
			if (m_CursorIndex + 1 < m_CommandLine.size())
				m_CursorIndex++;
			tickFlag = false;
			prevPressed = true;
		}
		else
			prevPressed = false;

		if (inputFocus){
			float offset = 0;
			float xs = 0;
			float x = 0;
			
			if (m_CursorIndex + 1 == 0 || m_CommandLine.size() == 0){
				xs = 12;
				x = m_gTextCommandLine.characterTransforms[1].getPosition().x - 4;
				offset += xs;
			}
			else if (m_CursorIndex == m_CommandLine.size())
			{
				offset += m_gTextCommandLine.characterTransforms[m_CursorIndex + 1].getScalar().x + 0.5;
				x = m_gTextCommandLine.characterTransforms[m_CursorIndex + 1].getPosition().x;
				xs = 12;
			}
			else{
				xs = m_gTextCommandLine.characterTransforms[m_CursorIndex + 2].getScalar().x + 0.5;
				x = m_gTextCommandLine.characterTransforms[m_CursorIndex + 2].getPosition().x;
				offset = xs;
			}

			m_gCursor.setSize({ xs, m_gCommandLine.getSize().y - 4 });
			m_gCursor.setPosition({ x + offset, m_gCommandLine.getPosition().y });
		}
	}
	void ConsoleWindow::update()
	{
		static bool prevMousePressed = false;
		
		m_Collided = Physics2D::AABBCollider::checkIntersection(Input::getPrevCursorPosition(), m_TitleShape);

		if (checkBorderIntersection(Input::getPrevCursorPosition(), m_BaseShape))
		{
			Vector2f mouseDelta = Input::getCursorPosition() - Input::getPrevCursorPosition();
			Vector2f delta = m_BaseShape.getPosition() - Input::getCursorPosition();

			Vector2f halfSize2 = m_BaseShape.getSize() / 2;
			Vector2f halfSize22 = (m_BaseShape.getSize() - m_BorderLength) / 2;

			Vector2f temp_penetration = Math::abs(delta) - halfSize2;
			Vector2f temp_penetration2 = Math::abs(delta) - halfSize22;

			if (temp_penetration.x > temp_penetration.y&& temp_penetration2.x > temp_penetration2.y)
			{
				m_CurrentWindow->setCursor(Cursor::HResize);

				if (Input::cursorIsMoving() && Input::mouseButtonIsDown(0))
				{
					if (delta.x > 0)
					{
						m_BaseShape.setSize({
								m_BaseShape.getSize().x - mouseDelta.x,
								m_BaseShape.getSize().y
							}
						);
						m_TitleShape.setSize({
								m_TitleShape.getSize().x - mouseDelta.x,
								m_TitleShape.getSize().y
							}
						);
						m_gCommandLine.setSize({
								m_gCommandLine.getSize().x - mouseDelta.x,
								m_gCommandLine.getSize().y
							}
						);

						text.move({mouseDelta.x, 0});
						m_gTextCommandLine.move({mouseDelta.x, 0});
						m_gTextOutput.move({mouseDelta.x, 0});
					}
					else if (delta.x < 0)
					{
						m_BaseShape.setSize({
								m_BaseShape.getSize().x + mouseDelta.x,
								m_BaseShape.getSize().y
							}
						);
						m_TitleShape.setSize({
								m_TitleShape.getSize().x + mouseDelta.x,
								m_TitleShape.getSize().y
							}
						);
						m_gCommandLine.setSize({
								m_gCommandLine.getSize().x + mouseDelta.x,
								m_gCommandLine.getSize().y
							}
						);
					}

					m_BaseShape.move({ mouseDelta.x / 2, 0 });
					m_TitleShape.setPosition({ m_BaseShape.getPosition().x, m_BaseShape.getPosition().y + (m_BaseShape.getSize().y / 2) - 15 });
					m_gCommandLine.setPosition({ m_BaseShape.getPosition().x, m_BaseShape.getPosition().y - (m_BaseShape.getSize().y / 2) + 15 });
				}
			}
			else
			{
				m_CurrentWindow->setCursor(Cursor::VResize);

				if (Input::cursorIsMoving() && Input::mouseButtonIsDown(0))
				{
					if (delta.y > 0)
					{
						m_BaseShape.setSize({ m_BaseShape.getSize().x, m_BaseShape.getSize().y - mouseDelta.y });
						m_gTextCommandLine.move({ 0, mouseDelta.y });
						m_gTextOutput.move({ 0, mouseDelta.y });
					}
					else if (delta.y < 0)
					{
						m_BaseShape.setSize({ m_BaseShape.getSize().x, m_BaseShape.getSize().y + mouseDelta.y });

						text.move({ 0, mouseDelta.y });
					}

					m_BaseShape.move({ 0, mouseDelta.y / 2 });
					m_TitleShape.setPosition({ m_BaseShape.getPosition().x, m_BaseShape.getPosition().y + (m_BaseShape.getSize().y / 2) - 15 });
					m_gCommandLine.setPosition({ m_BaseShape.getPosition().x, m_BaseShape.getPosition().y - (m_BaseShape.getSize().y / 2) + 15 });
				}
			}
		}

		if ((m_Collided || Physics2D::AABBCollider::checkIntersection(Input::getCursorPosition(), m_TitleShape)) && vgl::Input::mouseButtonIsDown(0) && vgl::Input::cursorIsMoving())
		{
			//if (vgl::Input::mouseButtonIsPressed(0) && vgl::Input::cursorIsMoving())
			//{
				Vector2f disp = (Input::getCursorPosition() - Input::getPrevCursorPosition());
				
				m_TitleShape.move(disp);
				m_gCommandLine.move(disp);
				m_BaseShape.move(disp);

				text.move(disp);
				m_gTextCommandLine.move(disp);
				m_gTextOutput.move(disp);
			//}
		}

		m_Collided = Physics2D::AABBCollider::checkIntersection(Input::getPrevCursorPosition(), m_TitleShape) ||
		   	Physics2D::AABBCollider::checkIntersection(Input::getCursorPosition(), m_TitleShape);

		m_Pressed = Input::cursorIsMoving() && !Input::mouseButtonIsReleased(0) && !m_Collided;

		prevMousePressed = Input::mouseButtonIsDown(0);
	}


	void ConsoleWindow::print(std::string p_Output)
	{
		static std::string str;
		str += "Console: " + p_Output + "\n";
		m_gTextOutput.setText(str);
		m_gTextOutput.move({ 0, m_gTextOutput.rec.getSize().y + m_gTextOutput.getNewlineHeight() - 7});
	}
}
