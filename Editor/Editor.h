#pragma once

#include <iostream>
#include <filesystem>

#include "../include/Application/Application.h"
#include "../include/DefaultECSComponents.h"
//#include "../include/VGL-3D/Mesh/VGL-Assimp.h"
#include "../include/VGL-3D/Mesh/OBJ_Loader.h"
#include "../include/VGL-3D/SkyBox.h"
#include "../include/Utils/FileGUI.h"
#include "../include/Scene.h"
//#include "../include/SceneRenderer.h"
#include "../include/RenderPipelines.h"

//#include "../lib/ImGuiColorTextEdit/TextEditor.h"

namespace vgl
{
	class E3DLayer : public Layer
	{
		public:
			void onAttach();
			void onDetach();
			void onUpdate(Renderer& p_Renderer);
			void onImGuiUpdate();

			bool onMouseMove(Event::MouseMovedEvent& p_Move) { m_CameraController.onMouseMovedEvent(p_Move); return false; };
			bool onMousePressed(Event::MouseButtonPressedEvent& p_Press) { return false; };
			bool onMouseReleased(Event::MouseButtonReleasedEvent& p_Released) { return false; };
			bool onScrollEvent(Event::MouseScrolledEvent& p_Scroll) {
				//if (windowFocused)
					m_CameraController.onMouseScrolledEvent(p_Scroll);
				return false;
			};

			bool onKeyPressed(Event::KeyPressedEvent& p_Press) { return false; };
			bool onKeyReleased(Event::KeyReleasedEvent& p_Released) { return false; };
			bool onKeyRepeated(Event::KeyRepeatedEvent& p_Repeated) { return false; };
			bool onKeyTyped(Event::KeyTypedEvent& p_Typed) { return false; };

		private:
			friend class Editor;

			//TextEditor m_TextEditor;

			CameraController3D m_CameraController;
			Window* m_WindowPtr;
			Renderer* m_RendererPtr;

			bool m_ViewportFocused = false;
			Viewport m_ViewportData;

			std::string m_CubeMapPath;

			Image m_HDRImage;
			ImageCube m_HDRCubeMap;
			ImageCube m_IrradianceImage;
			ImageCube m_PrefilteredImage;
			Image m_BRDFImage;

			ecs::EntityHandle m_DirectionalLightEntity;
			ecs::EntityHandle m_SkyBoxEntity;

			RenderPipeline_Deferred m_RenderPipeline;

			bool m_SkyBoxUI = false;
			bool m_AtmosphereUI = false;
			bool m_DepthOfFieldUI = false;
			bool m_EditorCameraUI = false;
			bool m_PostProcessingUI = false;
			bool m_ContentBrowserUI = false;
			bool m_SceneHierarchyUI = false;;
			bool m_ShaderHierarchyUI = false;

			Image m_DefaultImage;
			Image m_EntityIcon;
			Image m_TransformIcon;
			Image m_MaterialIcon;
			Image m_OBJIcon;
			Image m_PNGIcon;
			Image m_JPGIcon;
			Image m_HDRIcon;
			Image m_FOLDERIcon;
			Image m_SCENEIcon;
			Image m_FILEIcon;

			std::string m_Draggable;
			std::string m_DraggableFileName;
			std::string m_DraggableDirectory;

			std::filesystem::path asset_directory;
			std::filesystem::path current_directory;
			
			Scene m_Scene;
			std::string m_ScenePath;
			ecs::SystemList m_Systems;

			void cameraEdit();
			void sceneDisplay();
			void contentBrowser();
			void post_process_UI();
			void dof_UI();
			void skybox_UI();
			void atmosphere_UI();
			void transformEdit(Transform3DComponent* p_TransformComponent, const int p_ID, bool p_Translation = true, bool p_Rotation = true, bool p_Scale = true);
	};

	class Editor : public Application
	{
		public:
			Editor(AppConfig& p_AppConfig);
			~Editor() {};

			void setup();

		private:

			E3DLayer* e3Dlayer = new E3DLayer;
	};
}